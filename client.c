#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

int client_socket = -1;

void handle_sigint(int sig) {
    printf("\nDisconnecting from server...\n");
    if (client_socket != -1) {
        close(client_socket);
    }
    exit(0);
}

void display_opinion_bar(int opinion) {
    printf("Public Opinion: [");
    
    int bar_length = 50;
    int position = (opinion - MIN_OPINION) * bar_length / (MAX_OPINION - MIN_OPINION);
    
    for (int i = 0; i < bar_length; i++) {
        if (i < position) {
            printf("=");
        } else if (i == position) {
            printf("|");
        } else {
            printf(" ");
        }
    }
    
    printf("] %d%%\n", opinion);
}

void display_hand(GameMessage *msg) {
    printf("\nYour Hand:\n");
    for (int i = 0; i < msg->hand_size; i++) {
        printf("%d. %s - %s\n", i + 1, CardNames[msg->cards[i]], CardDescriptions[msg->cards[i]]);
    }
    printf("\n");
}

int connect_to_server(const char *server_ip, int port) {
    struct sockaddr_in server_addr;
    
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    return client_socket;
}

int send_game_message(GameMessage *msg) {
    return send(client_socket, msg, sizeof(GameMessage), 0);
}

int receive_game_message(GameMessage *msg) {
    return recv(client_socket, msg, sizeof(GameMessage), 0);
}

void play_game() {
    GameMessage msg;
    int game_active = 1;
    
    while (game_active) {
        if (receive_game_message(&msg) <= 0) {
            printf("Disconnected from server.\n");
            break;
        }

        switch (msg.type) {
            case MSG_ACK:
                printf("Server: %s\n", msg.message);
                break;
                
            case MSG_READY:
                printf("Server: %s\n", msg.message);
                break;
                
            case MSG_INIT:
                printf("------------------------------------\n");
                printf("| Welcome to the Point Taken! |\n");
                printf("------------------------------------\n");
                printf("\nYou are %s.\n", PartyNames[msg.party]);
                printf("Your Opponent is %s.\n\n", PartyNames[msg.opponent_party]);
                display_opinion_bar(msg.public_opinion);
                display_hand(&msg);

                int player_card_choice;
                int valid_input = 0;
                
                while (!valid_input) {
                    printf("< Round %d >\nSelect a card to play (1-%d): ", msg.round, msg.hand_size);
                    if (scanf("%d", &player_card_choice) == 1) {
                        if (player_card_choice >= 1 && player_card_choice <= msg.hand_size) {
                            valid_input = 1;
                        } else {
                            printf("Invalid card choice. Please enter a number between 1 and %d.\n", msg.hand_size);
                        }
                    } else {
                        printf("Invalid input. Please enter a number.\n");
                        scanf("%*s");
                    }
                }

                msg.selected_card = msg.cards[player_card_choice - 1];
                send_game_message(&msg);
                break;
                
            case MSG_DEAL:
                if (msg.round > 1) {
                    display_opinion_bar(msg.public_opinion);
                }

                display_hand(&msg);

                valid_input = 0;
                while (!valid_input) {
                    printf("< Round %d >\nSelect a card to play (1-%d): ", msg.round, msg.hand_size);
                    if (scanf("%d", &player_card_choice) == 1) {
                        if (player_card_choice >= 1 && player_card_choice <= msg.hand_size) {
                            valid_input = 1;
                        } else {
                            printf("Invalid card choice. Please enter a number between 1 and %d.\n", msg.hand_size);
                        }
                    } else {
                        printf("Invalid input. Please enter a number.\n");
                        scanf("%*s");
                    }
                }

                msg.selected_card = msg.cards[player_card_choice - 1];
                send_game_message(&msg);
                break;
                
            case MSG_ROUND_RESULT:
                printf("\nResults:\n");
                printf("  You played: %s\n", CardNames[msg.selected_card]);
                printf("  Opponent played: %s\n", CardNames[msg.opponent_card]);
                printf("\n%s\n", msg.message);
                printf("\n");
                display_opinion_bar(msg.public_opinion);
                
                if (msg.game_over) {
                    printf("\nGame Over! %s\n", msg.message);
                    game_active = 0;
                }
                break;
                
            case MSG_GAME_OVER:
                printf("\nGame Over! %s\n", msg.message);
                game_active = 0;
                break;
                
            default:
                printf("Unknown message type received.\n");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, handle_sigint);

    if (connect_to_server(argv[1], 8080) < 0) {
        printf("Failed to connect to server.\n");
        return 1;
    }
    
    printf("Connected to server at %s\n", argv[1]);
    
    play_game();

    close(client_socket);
    return 0;
}