#ifndef COMMON_H
#define COMMON_H

#include <string.h>

#define MSG_INIT 1
#define MSG_ACK 2
#define MSG_READY 3
#define MSG_DEAL 4
#define MSG_ROUND_RESULT 5
#define MSG_GAME_OVER 6

#define MAX_ROUNDS 5
#define MAX_HAND_SIZE 2
#define STARTING_OPINION 50
#define MIN_OPINION 0
#define MAX_OPINION 100

#define ATTACK 0
#define DEFEND 1
#define CARD_COUNT 2

static const char *CardNames[] = {
    "Attack",
    "Defend"
};

static const char *CardDescriptions[] = {
    "Launch a strong attack against your opponent.",
    "Defend against your opponent's attacks."
};

static const char *PartyNames[] = {
    "Player 1",
    "Player 2"
};

typedef struct {
    int type;
    int party;
    int opponent_party;
    int public_opinion;
    int round;
    int hand_size;
    int cards[MAX_HAND_SIZE];
    int selected_card;
    int opponent_card;
    int opinion_change;
    int game_over;
    char message[256];
} GameMessage;

void init_game_message(GameMessage *msg, int type, int party, int round);
int generate_random_card();
void deal_hand(GameMessage *msg);
void display_opinion_bar(int opinion);
void display_hand(GameMessage *msg);
void simulate_round(GameMessage *msg, int player_card, int opponent_card);

#endif