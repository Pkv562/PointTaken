#include "common.h"
#include "game_logic.h"

int select_card(GameMessage *msg) {
    int choice;
    while (1) {
        printf("\nSelect card (1-%d): ", msg->hand_size);
        if (scanf("%d", &choice) == 1 && choice >= 1 && choice <= msg->hand_size) {
            return choice - 1;
        }
        while (getchar() != '\n');
        printf("Invalid input. Try again.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {AF_INET, htons(PORT)};
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    GameMessage msg;
    int round_history[MAX_ROUNDS][2] = {0};
    int history_size = 0;

    while (1) {
        if (received_message(sock, &msg) <= 0) break;

        switch (msg.type) {
            case START_INIT:
                printf("\n%s\nYour party: %s\nOpinion:\n", msg.message, PartyNames[msg.party]);
                display_opinion_bar(msg.public_opinion);
                display_hand(&msg);
                break;

            case CARD_SELECTION:
                printf("\033[2J\033[H=== Round %d ===\nOpinion:\n", msg.round);
                display_opinion_bar(msg.public_opinion);
                if (history_size > 0) {
                    printf("\nHistory:\n");
                    for (int i = 0; i < history_size; i++) {
                        printf("Round %d: You: %s, Opponent: %s\n", i+1, 
                               CardNames[round_history[i][0]], CardNames[round_history[i][1]]);
                    }
                }
                printf("\nOpponent has locked in their card.\n");
                display_hand(&msg);
                msg.selected_card = select_card(&msg);
                send_message(sock, &msg);
                break;

            case CARD_REVEAL:
                round_history[history_size][0] = msg.selected_card;
                round_history[history_size][1] = msg.opponent_card;
                printf("\nYou played: %s\nOpponent: %s\n", 
                       CardNames[msg.selected_card], CardNames[msg.opponent_card]);
                break;

            case ROUND_RESULT:
                history_size++;
                printf("\nRound %d: %s\nOpinion change: %+d%%\nNew Opinion:\n", 
                       msg.round, msg.message, msg.opinion_change);
                display_opinion_bar(msg.public_opinion);
                display_hand(&msg);
                printf("\nWaiting for next round...\n");
                sleep(2);
                break;

            case GAME_OVER:
                printf("\nGAME OVER\nFinal Opinion:\n");
                display_opinion_bar(msg.public_opinion);
                printf("%s\n", msg.message);
                close(sock);
                return 0;

            default:
                printf("Unknown message type: %d\n", msg.type);
                break;
        }
    }
    close(sock);
    return 1;
}
