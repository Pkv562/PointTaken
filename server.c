#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define MAX_PLAYERS 2

typedef struct {
    int socket;
    int party;
    int ready;
} Player;

Player players[MAX_PLAYERS];
int player_count = 0;
int server_socket;

void handle_sigint(int sig) {
    printf("\nShutting down server...\n");

    for (int i = 0; i < player_count; i++) {
        if (players[i].socket != -1) {
            close(players[i].socket);
        }
    }

    if (server_socket != -1) {
        close(server_socket);
    }
    
    exit(0);
}

void init_player(Player *player, int socket) {
    player->socket = socket;
    player->party = -1;
    player->ready = 0;
}

int send_game_message(int socket, GameMessage *msg) {
    return send(socket, msg, sizeof(GameMessage), 0);
}

int receive_game_message(int socket, GameMessage *msg) {
    return recv(socket, msg, sizeof(GameMessage), 0);
}

void init_game_message(GameMessage *msg, int type, int party, int round) {
    msg->type = type;
    msg->party = party;
    msg->public_opinion = STARTING_OPINION;
    msg->round = round;
    msg->hand_size = 0;
    msg->selected_card = -1;
    msg->opponent_card = -1;
    msg->opponent_party = -1;
    msg->opinion_change = 0;
    msg->game_over = 0;
    memset(msg->message, 0, sizeof(msg->message));
}

int generate_random_card() {
    return rand() % CARD_COUNT;
}

void deal_hand(GameMessage *msg) {
    msg->hand_size = MAX_HAND_SIZE;
    for (int i = 0; i < MAX_HAND_SIZE; i++) {
        msg->cards[i] = generate_random_card();
    }
}

void simulate_round(GameMessage *msg, int player_card, int opponent_card) {
    msg->selected_card = player_card;
    msg->opponent_card = opponent_card;
    msg->type = MSG_ROUND_RESULT;

    if (player_card == opponent_card) {
        msg->opinion_change = 0;
        snprintf(msg->message, sizeof(msg->message), "Both players played the same card. No change in opinion.");
    }
    else if (player_card == ATTACK && opponent_card == DEFEND) {
        msg->opinion_change = -5;
        snprintf(msg->message, sizeof(msg->message), "Attack was blocked by Defend!");
    }
    else if (player_card == ATTACK && opponent_card != DEFEND) {
        msg->opinion_change = 10;
        snprintf(msg->message, sizeof(msg->message), "Attack was successful!");
    }
    else if (player_card == DEFEND && opponent_card == ATTACK) {
        msg->opinion_change = 5;
        snprintf(msg->message, sizeof(msg->message), "Defend successfully blocked Attack!");
    }
    else if (player_card == DEFEND && opponent_card != ATTACK) {
        msg->opinion_change = -5;
        snprintf(msg->message, sizeof(msg->message), "Defend was not needed.");
    }

    msg->public_opinion += msg->opinion_change;
    if (msg->public_opinion > MAX_OPINION)
        msg->public_opinion = MAX_OPINION;
    if (msg->public_opinion < MIN_OPINION)
        msg->public_opinion = MIN_OPINION;

    if (msg->round >= MAX_ROUNDS || msg->public_opinion >= MAX_OPINION || msg->public_opinion <= MIN_OPINION) {
        msg->game_over = 1;
        msg->type = MSG_GAME_OVER;
        if (msg->public_opinion >= MAX_OPINION) {
            snprintf(msg->message, sizeof(msg->message), "%s wins! (Max opinion reached)", PartyNames[msg->party]);
        } else if (msg->public_opinion <= MIN_OPINION) {
            snprintf(msg->message, sizeof(msg->message), "Opponent wins! (Min opinion reached)");
        } else if (msg->round >= MAX_ROUNDS) {
            if (msg->public_opinion > 50) {
                snprintf(msg->message, sizeof(msg->message), "%s wins!", PartyNames[msg->party]);
            } else if (msg->public_opinion < 50) {
                snprintf(msg->message, sizeof(msg->message), "Opponent wins!");
            } else {
                snprintf(msg->message, sizeof(msg->message), "It's a tie!");
            }
        }
    }
}

void handle_game_setup() {
    players[0].party = 0;
    players[1].party = 1;
    
    printf("Player 1 assigned to party: %s\n", PartyNames[players[0].party]);
    printf("Player 2 assigned to party: %s\n", PartyNames[players[1].party]);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        GameMessage setup_msg;
        init_game_message(&setup_msg, MSG_INIT, players[i].party, 1);

        setup_msg.opponent_party = players[1-i].party;

        deal_hand(&setup_msg);

        if (send_game_message(players[i].socket, &setup_msg) < 0) {
            perror("Failed to send setup message");
            return;
        }

        printf("Sent setup message to player %d\n", i+1);
    }
}

void handle_game_round() {
    GameMessage player1_msg, player2_msg;
    int round = 1;
    int game_over = 0;
    int p1_opinion = STARTING_OPINION;
    int p2_opinion = STARTING_OPINION;

    while (!game_over && round <= MAX_ROUNDS) {
        printf("Round %d starting...\n", round);

        if (receive_game_message(players[0].socket, &player1_msg) <= 0) {
            printf("Player 1 disconnected\n");
            break;
        }
        
        if (receive_game_message(players[1].socket, &player2_msg) <= 0) {
            printf("Player 2 disconnected\n");
            break;
        }
        
        printf("Player 1 played: %s\n", CardNames[player1_msg.selected_card]);
        printf("Player 2 played: %s\n", CardNames[player2_msg.selected_card]);
        
        player1_msg.opponent_card = player2_msg.selected_card;
        player1_msg.round = round;
        simulate_round(&player1_msg, player1_msg.selected_card, player2_msg.selected_card);

        player2_msg.opponent_card = player1_msg.selected_card;
        player2_msg.round = round;
        simulate_round(&player2_msg, player2_msg.selected_card, player1_msg.selected_card);

        send_game_message(players[0].socket, &player1_msg);
        send_game_message(players[1].socket, &player2_msg);
        
        p1_opinion = player1_msg.public_opinion;
        p2_opinion = player2_msg.public_opinion;

        if (player1_msg.game_over || player2_msg.game_over || round >= MAX_ROUNDS) {
            game_over = 1;
        } else {
            GameMessage new_hand1, new_hand2;
            init_game_message(&new_hand1, MSG_DEAL, players[0].party, round + 1);
            init_game_message(&new_hand2, MSG_DEAL, players[1].party, round + 1);
            
            new_hand1.public_opinion = p1_opinion;
            new_hand2.public_opinion = p2_opinion;
            
            deal_hand(&new_hand1);
            deal_hand(&new_hand2);
            
            send_game_message(players[0].socket, &new_hand1);
            send_game_message(players[1].socket, &new_hand2);
        }
        
        round++;
    }
    
    printf("Game completed after %d rounds\n", round - 1);
    printf("Final opinions - Player 1: %d, Player 2: %d\n", p1_opinion, p2_opinion);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;

    srand(time(NULL));
    signal(SIGINT, handle_sigint);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].socket = -1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
 
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_PLAYERS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Simple Game Server started on port %d\n", PORT);
    printf("Waiting for players to connect...\n");

    while (player_count < MAX_PLAYERS) {
        int client_socket;
        
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Player %d connected from %s\n", player_count + 1, client_ip);
        
        init_player(&players[player_count], client_socket);
        player_count++;

        GameMessage ack_msg;
        init_game_message(&ack_msg, MSG_ACK, 0, 0);
        snprintf(ack_msg.message, sizeof(ack_msg.message), "Connected as Player %d. Waiting for opponent...", player_count);
        send_game_message(client_socket, &ack_msg);
        
        if (player_count == MAX_PLAYERS) {
            printf("Both players connected. Starting game...\n");

            for (int i = 0; i < MAX_PLAYERS; i++) {
                GameMessage ready_msg;
                init_game_message(&ready_msg, MSG_READY, 0, 0);
                snprintf(ready_msg.message, sizeof(ready_msg.message), "Both players connected. Game starting!");
                send_game_message(players[i].socket, &ready_msg);
            }

            handle_game_setup();
            handle_game_round();

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (players[i].socket != -1) {
                    close(players[i].socket);
                    players[i].socket = -1;
                }
            }
            
            player_count = 0;
            printf("Game ended. Waiting for new players...\n");
        }
    }

    close(server_socket);
    return 0;
}