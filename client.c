#include "common.h"
#include "game_logic.h"

int select_card(GameMessage *msg) {
    int choice;

    while(1) {
        printf("\nSelect a card to play (1-%d): ", msg->hand_size);
        if(scanf("%d",  &choice) != 1) {
            while(getchar() != '\n');
            printf("Invalid input. Please enter a number\n");
            continue;
        }

        choice--;

        if(choice >= 0 && choice < msg->hand_size) {
            return choice;
        }

        printf("Invalid choice. Please select a card between 1 and %d\n", msg->hand_size);
    }
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    GameMessage msg;
    int game_over = 0;
    int round_history[MAX_ROUNDS][2];
    int history_size = 0;
    int player_card = -1;
    int opponent_card = -1;

    if(argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address\n");
        return -1;
    }

    printf("Connecting to server at %s:%d...\n", argv[1], PORT);
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed!\n");
        return -1;
    }

    printf("Connected to the server. Waiting for the game to start!\n");
    int player_party = -1;

    while(!game_over) {
        if(received_message(sock, &msg) <= 0) {
            printf("Error receiving message from server.\n");
            break;
        }
        
        switch (msg.type) {
            case START_INIT:
                player_party = msg.party;
                printf("\n%s\n", msg.message);
                display_full_terminal(&msg, player_party, round_history, history_size, player_card, opponent_card);
                break;
                
            case CARD_SELECTION:
                player_card = -1;
                opponent_card = -1;
                display_full_terminal(&msg, player_party, round_history, history_size, player_card, opponent_card);
                
                int choice = select_card(&msg);
                msg.selected_card = choice;
                send_message(sock, &msg);
                break;
                
            case CARD_REVEAL:
                player_card = msg.selected_card;
                opponent_card = msg.opponent_card;
                
                display_full_terminal(&msg, player_party, round_history, history_size, player_card, opponent_card);
                printf("\nResolving round...\n");
                break;
                
            case ROUND_RESULT:
                if (history_size < MAX_ROUNDS) {
                    round_history[history_size][0] = player_card;
                    round_history[history_size][1] = opponent_card;
                    history_size++;
                }
                
                display_full_terminal(&msg, player_party, round_history, history_size, player_card, opponent_card);
                
                if (msg.opinion_change > 0) {
                    printf("\nPublic opinion shifted in your favor by %d%%!\n", msg.opinion_change);
                } else if (msg.opinion_change < 0) {
                    printf("\nPublic opinion shifted against you by %d%%!\n", -msg.opinion_change);
                } else {
                    printf("\nPublic opinion remained unchanged.\n");
                }
                
                printf("\n%s\n", msg.message);
                
                printf("\nPress Enter to continue to the next round...");
                while (getchar() != '\n');
                getchar();
                break;
                
            case GAME_OVER:
                if (player_party >= 0 && player_party < PARTY_COUNT) {
                    display_full_terminal(&msg, player_party, round_history, history_size, player_card, opponent_card);
                } else {
             
                    printf("\033[2J\033[H"); 
                }
                
                printf("\n=== GAME OVER ===\n");
                if (msg.public_opinion > 50) {
                    printf("Congratulations! You've won the debate!\n");
                } else if (msg.public_opinion < 50) {
                    printf("You've lost the debate.\n");
                } else {
                    printf("The debate ended in a tie.\n");
                }
                
                printf("\n%s\n", msg.message);
                game_over = 1;
                break;
                
            case ERROR:
                printf("\nERROR: %s\n", msg.message);
                break;
                
            default:
                printf("\nUnknown message type received: %d\n", msg.type);
                break;
        }
    }
    
    usleep(100000);
    
    close(sock);
    return 0;
}