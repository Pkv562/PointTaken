#include "common.h"
#include "game_logic.h"

void display_party_info(int party) {
    printf("\nYou are playing as: %s\n", PartyNames[party]);
    
    printf("Party Effects:\n");
    switch (party) {
        case PARTIDO_BAYANIHAN:
            printf("- Public opinion penalties from any counter is reduced by 50%%\n");
            printf("- Gains only 75%% of the score when using aggressive cards\n");
            break;
            
        case LAKAS_NG_MASA:
            printf("- \"Attack\" and \"Attack Opponent\" gain an extra +5%% boost\n");
            printf("- Takes an extra -5%% when a move is countered\n");
            break;
            
        case PARTIDO_KALIKASAN:
            printf("- \"Appeal to Emotion\" and \"Personal Story\" gain an extra +5%% score\n");
            printf("- The \"Use statistic\" is 50%% less effective\n");
            break;
            
        case MAKABAGONG_TEKNOLOHIYA:
            printf("- \"Use Statistic\" is only half countered (still gives around +7%%)\n");
            printf("- When used increase the score by 50%%\n");
            printf("- \"Appeal to emotion\" and \"Personal Story\" has no effect\n");
            break;
            
        case TAHIMIK_NA_SIGAW:
            printf("- \"Stay silent\" and \"Change topic\" grant a bonus +5%% when used\n");
            printf("- Can only use \"Attack\" or \"Attack Opponent\" once per game\n");
            break;
    }
}

void display_card_effects() {
    printf("\nCard Effects:\n");
    printf("- Appeal to Emotion: +10%% (countered by Fact Check)\n");
    printf("- Use Statistics: +15%% (countered by Distrust Data)\n");
    printf("- Attack Opponent: +5%% (countered by Defend, Fact Check)\n");
    printf("- Change Topic: Nullifies opponent's move this round\n");
    printf("- Personal Story: +8%% (immune to Attack or Distrust Data)\n");
    printf("- Distrust Data: Cancels \"Use Statistics\"; -3%% if used alone\n");
    printf("- Fact Check: Nullifies \"Appeal to Emotion\" or \"Attack\"; +5%% if opponent used those\n");
    printf("- Attack: +6%% (countered by Defend, Fact Check)\n");
    printf("- Defend: Cancels \"Attack\" or \"Attack Opponent\"\n");
    printf("- Stay Silent: +3%% if opponent uses \"Attack\" or \"Attack Opponent\"\n");
}


void select_card(GameMessage *msg) {
    int choice;

    while(true) {
        printf("\nSelect a card to play (1-%d): ", msg->hand_size);
        if(scanf("%d",  &choice != 1)) {
            while(getchar() != '\n');
            print("Invalid input. Please enter a number \n");
            continue;
        }

        choice--;

        if(choice > 0 && choice < msg->hand_size) {
            return choice;
        }

        printf("Invalid choice. please selected a card between 1 and %d \n", msg->hand_size);


    }
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    GameMessage msg;
    int game_over = 0;
    int round_history[MAX_ROUNDS][2];
    int history_size = 0;

    if(argc != 2) {
        printf("Usage: %s <server_ip>", argv[0]);
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

    while(!game_over) {
        
        if(receivedMessage(sock, &msg) <= 0) {
            printf("Error receiving message from server. \n");
            break;
        }
        switch (msg.type) {
            case START_INIT:
                printf("\n%s\n", msg.message);

                display_party_info(msg.party);

                display_card_effects();

                printf("\nInitial Public Opinion:\n");
                display_opinion_bar(msg.public_opinion);

                display_hand(&msg);
                break;
                
            case CARD_SELECTION:
                printf("\033[2J\033[H");

                printf("\n=== Round %d ===\n", msg.round);

                printf("\nCurrent Public Opinion:\n");
                display_opinion_bar(msg.public_opinion);

                if (history_size > 0) {
                    printf("\nMove History:\n");
                    for (int i = 0; i < history_size; i++) {
                        printf("Round %d: You played %s, Opponent played %s\n", 
                               i + 1, 
                               CardNames[round_history[i][0]], 
                               CardNames[round_history[i][1]]);
                    }
                }

                display_hand(&msg);

                int choice = select_card(&msg);
                msg.selected_card = choice;

                send_message(sock, &msg);
                break;
                
            case CARD_REVEAL:
                round_history[history_size][0] = msg.selected_card;
                round_history[history_size][1] = msg.opponent_card;

                printf("\nCards Revealed:\n");
                printf("You played: %s\n", CardNames[msg.selected_card]);
                printf("Opponent played: %s\n", CardNames[msg.opponent_card]);

                printf("\nResolving round...\n");
                break;
                
            case ROUND_RESULT:
                history_size++;

                printf("\nRound %d Result:\n", msg.round);

                if (msg.opinion_change > 0) {
                    printf("Public opinion shifted in your favor by %d%%!\n", msg.opinion_change);
                } else if (msg.opinion_change < 0) {
                    printf("Public opinion shifted against you by %d%%!\n", -msg.opinion_change);
                } else {
                    printf("Public opinion remained unchanged.\n");
                }

                printf("\nUpdated Public Opinion:\n");
                display_opinion_bar(msg.public_opinion);

                printf("\n%s\n", msg.message);

                printf("\nYour updated hand:\n");
                display_hand(&msg);

                printf("\nPress Enter to continue to the next round...");
                getchar();
                getchar();
                break;
                
            case GAME_OVER:
                printf("\n=== GAME OVER ===\n");
                printf("\nFinal Public Opinion:\n");
                display_opinion_bar(msg.public_opinion);
                
                printf("\n%s\n", msg.message);

                game_over = 1;
                break;
                
            case MSG_ERROR:
                printf("\nERROR: %s\n", msg.message);
                break;
                
            default:
                printf("\nUnknown message type received: %d\n", msg.type);
                break;
        }
    }
    
    close(sock);
    
    return 0;

}