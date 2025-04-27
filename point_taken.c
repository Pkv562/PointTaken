#include "common.h" 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    msg->attack_used = 0;
}

int generate_random_card(int hand[], int hand_size) {
    int card;
    int duplicate;
    do {
        duplicate = 0;
        card = rand() % CARD_COUNT;
        for (int i = 0; i < hand_size; i++) {
            if (hand[i] == card) {
                duplicate = 1;
                break;
            }
        }
    } while (duplicate);
    return card;
}

void deal_hand(GameMessage *msg) {
    msg->hand_size = MAX_HAND_SIZE;
    int hand[MAX_HAND_SIZE];
    for (int i = 0; i < MAX_HAND_SIZE; i++) {
        msg->cards[i] = generate_random_card(hand, i);
        hand[i] = msg->cards[i];
    }
}

void display_card_details(int card) {
    if (card >= 0 && card < CARD_COUNT) {
        printf("Card: %s\n", CardNames[card]);
        printf("Description: %s\n", CardDescriptions[card]);
    } else {
        printf("Invalid card index.\n");
    }
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

void simulate_round(GameMessage *msg, int player_card, int opponent_card, int opponent_party) {
    msg->selected_card = player_card;
    msg->opponent_card = opponent_card;
    msg->type = MSG_ROUND_RESULT;

    int base_opinion_change = 0;

    if (player_card == opponent_card) {
        msg->opinion_change = 0;
        snprintf(msg->message, sizeof(msg->message), "Both players played the same card. No change in opinion.");
    }
    else {
        switch (player_card) {
            case APPEAL_TO_EMOTION:
                base_opinion_change = 5;
                snprintf(msg->message, sizeof(msg->message), "Appeal to Emotion was effective.");
                if (msg->party == PARTIDO_KALIKASAN)
                    base_opinion_change += 5;
                else if (msg->party == MAKABAGONG_TEKNOLOHIYA)
                    base_opinion_change = 0;
                break;
            case USE_STATISTICS:
                base_opinion_change = 10;
                snprintf(msg->message, sizeof(msg->message), "Use Statistics was effective.");
                if (msg->party == PARTIDO_KALIKASAN)
                    base_opinion_change /= 2;
                else if (msg->party == MAKABAGONG_TEKNOLOHIYA)
                    base_opinion_change = 7;
                break;
            case ATTACK_OPPONENT:
                base_opinion_change = 15;
                snprintf(msg->message, sizeof(msg->message), "Attack Opponent was effective.");
                if (msg->party == LAKAS_NG_MASA)
                    base_opinion_change += 5;
                if (msg->party == PARTIDO_BAYANIHAN)
                    base_opinion_change *= 0.75;
                break;
            case CHANGE_TOPIC:
                base_opinion_change = -5;
                snprintf(msg->message, sizeof(msg->message), "Change Topic was not effective.");
                if(msg->party == TAHIMIK_NA_SIGAW)
                    base_opinion_change += 5;
                break;
            case PERSONAL_STORY:
                base_opinion_change = 5;
                snprintf(msg->message, sizeof(msg->message), "Personal Story was effective.");
                if (msg->party == PARTIDO_KALIKASAN)
                    base_opinion_change += 5;
                else if (msg->party == MAKABAGONG_TEKNOLOHIYA)
                    base_opinion_change = 0;
                break;
            case DISTRUST_DATA:
                base_opinion_change = -10;
                snprintf(msg->message, sizeof(msg->message), "Distrust Data was not effective.");
                break;
            case FACT_CHECK:
                base_opinion_change = 10;
                snprintf(msg->message, sizeof(msg->message), "Fact Check was effective.");
                break;
            case ATTACK:
                base_opinion_change = 15;
                snprintf(msg->message, sizeof(msg->message), "Attack was very effective.");
                if (msg->party == LAKAS_NG_MASA)
                    base_opinion_change += 5;
                if (msg->party == PARTIDO_BAYANIHAN)
                    base_opinion_change *= 0.75;
                if (msg->party == TAHIMIK_NA_SIGAW){
                    if(msg->attack_used == 0){
                         msg->attack_used = 1;
                    }
                    else{
                         base_opinion_change = -100;
                         snprintf(msg->message, sizeof(msg->message), "Attack can only be used once.");
                    }
                }
                break;
            case DEFEND:
                base_opinion_change = -5;
                snprintf(msg->message, sizeof(msg->message), "Defend was not effective.");
                break;
            case STAY_SILENT:
                base_opinion_change = -1;
                snprintf(msg->message, sizeof(msg->message), "Stay Silent was not effective.");
                if(msg->party == TAHIMIK_NA_SIGAW)
                    base_opinion_change += 5;
                break;
            default:
                msg->opinion_change = 0;
                snprintf(msg->message, sizeof(msg->message), "No effect.");
        }
    }
    if (opponent_card == APPEAL_TO_EMOTION || opponent_card == USE_STATISTICS ||
        opponent_card == ATTACK_OPPONENT || opponent_card == CHANGE_TOPIC    ||
        opponent_card == PERSONAL_STORY  || opponent_card == DISTRUST_DATA ||
        opponent_card == FACT_CHECK      || opponent_card == ATTACK        ||
        opponent_card == DEFEND        || opponent_card == STAY_SILENT)
    {
        if (msg->party == PARTIDO_BAYANIHAN) {
            msg->opinion_change = base_opinion_change / 2;
        } else if (msg->party == LAKAS_NG_MASA) {
            msg->opinion_change = base_opinion_change - 5;
        }
    }
    if (player_card == APPEAL_TO_EMOTION || player_card == USE_STATISTICS ||
        player_card == ATTACK_OPPONENT || player_card == CHANGE_TOPIC    ||
        player_card == PERSONAL_STORY  || player_card == DISTRUST_DATA ||
        player_card == FACT_CHECK      || player_card == ATTACK        ||
        player_card == DEFEND        || player_card == STAY_SILENT)
    {
        if (opponent_party == PARTIDO_BAYANIHAN) {
            msg->opinion_change -= base_opinion_change / 2;
        } else if (opponent_party == LAKAS_NG_MASA) {
            msg->opinion_change -= (base_opinion_change - 5);
        }
    }
    msg->opinion_change = base_opinion_change;

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

int main() {
    srand(time(NULL));

    GameMessage game_msg;
    int player_party = rand() % PARTY_COUNT; 
    int opponent_party = rand() % PARTY_COUNT;
    int current_round = 1;

    if(player_party == opponent_party){
        opponent_party = (opponent_party + 1) % PARTY_COUNT;
    }

    init_game_message(&game_msg, MSG_INIT, player_party, current_round);
    game_msg.opponent_party = opponent_party; 

    deal_hand(&game_msg);

    printf("------------------------------------\n");
    printf("| Welcome to the Point Taken Game! |\n");
    printf("------------------------------------\n");
    printf("\nYou are from %s.\n", PartyNames[player_party]);
    printf("Your Opponent is from %s.\n\n", PartyNames[opponent_party]);
    display_opinion_bar(game_msg.public_opinion);
    display_hand(&game_msg);

    while (game_msg.round < MAX_ROUNDS && !game_msg.game_over) {
        int player_card_choice;
        int valid_input = 0;

        while (!valid_input) {
            printf("< Round %d >\nSelect a card to play (1-%d): ", game_msg.round, game_msg.hand_size);
            if (scanf("%d", &player_card_choice) == 1) {
                if (player_card_choice >= 1 && player_card_choice <= game_msg.hand_size) {
                    valid_input = 1;
                } else {
                    printf("Invalid card choice. Please enter a number between 1 and %d.\n", game_msg.hand_size);
                }
            } else {
                printf("Invalid input. Please enter a number.\n");
                scanf("%*s");
            }
        }
        int player_card = game_msg.cards[player_card_choice - 1];
        int opponent_card = generate_random_card(game_msg.cards, game_msg.hand_size); 

        simulate_round(&game_msg, player_card, opponent_card, opponent_party); 

        printf("\nResults:\n");
        printf("  You played: %s\n", CardNames[player_card]);
        printf("  Opponent played: %s\n", CardNames[opponent_card]);
        printf("\n%s\n", game_msg.message);
        printf("\n");
        display_opinion_bar(game_msg.public_opinion);

        if (game_msg.game_over) {
            printf("\nGame Over! %s\n", game_msg.message);
            break;
        }
        deal_hand(&game_msg); 
        display_hand(&game_msg);
    }
    if (!game_msg.game_over)
    {
        printf("\nGame Over: Max rounds reached.\n");
        if (game_msg.public_opinion > 50)
             printf("%s wins!\n", PartyNames[player_party]);
        else if (game_msg.public_opinion < 50)
            printf("Opponent wins!\n");
        else
            printf("It's a tie!\n");
    }

    return 0;
}