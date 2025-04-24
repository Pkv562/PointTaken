#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#endif
#ifdef __unix__
#include <arpa/inet.h>
#endif
#ifdef __unix__
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CARDS 20
#define MAX_HAND_SIZE 5
#define MAX_ROUNDS 10

#define STARTING_OPINION 50
#define MIN_OPINION 0
#define MAX_OPINION 100

#define MSG_INIT 1
#define MSG_CARD_SELECTION 2
#define MSG_CARD_REVEAL 3
#define MSG_ROUND_RESULT 4
#define MSG_GAME_END 5
#define MSG_ERROR 6

typedef enum {
    PARTIDO_BAYANIHAN,
    LAKAS_NG_MASA,
    PARTIDO_KALIKASAN,
    MAKABAGONG_TEKNOLOHIYA,
    TAHIMIK_NA_SIGAW,
    PARTY_COUNT
} PartyType;

const char* PartyNames[PARTY_COUNT] = {
    "Partido Bayanihan",
    "Lakas ng Masa",
    "Partido Kalikasan",
    "Makabagong Teknolohiya",
    "Tahimik na Sigaw"
};

typedef enum {
    APPEAL_TO_EMOTION,
    USE_STATISTICS,
    ATTACK_OPPONENT,
    CHANGE_TOPIC,
    PERSONAL_STORY,
    DISTRUST_DATA,
    FACT_CHECK,
    ATTACK,
    DEFEND,
    STAY_SILENT,
    CARD_COUNT
} CardType;

const char* CardNames[CARD_COUNT] = {
    "Appeal to Emotion",
    "Use Statistics",
    "Attack Opponent",
    "Change Topic",
    "Personal Story",
    "Distrust Data",
    "Fact Check",
    "Attack",
    "Defend",
    "Stay Silent"
};

const char* CardDescriptions[CARD_COUNT] = {
    "Tug at the public's feelings",
    "Drop facts and data",
    "Slam opponent's past actions",
    "Avoid tough questions",
    "Share a touching anecdote",
    "Cast doubt on the opponent's facts",
    "Expose inaccuracies in opponent's claim",
    "Aggressively challenge the opponent",
    "Shield against attacks",
    "Remain composed and say nothing"
};

typedef struct {
    int type;                 
    int party;                 
    int public_opinion;        
    int round;                 
    int cards[MAX_HAND_SIZE]; 
    int hand_size;            
    int selected_card;        
    int opponent_card;         
    int opinion_change;        
    int game_over;             
    char message[256];         
} GameMessage;

int send_message(int socket, GameMessage *msg) {
    return send(socket, (const char*)msg, sizeof(GameMessage), 0);
}

int receive_message(int socket, GameMessage *msg) {
    return recv(socket, (char*)msg, sizeof(GameMessage), 0);
}

void display_opinion_bar(int opinion) {
    printf("P1 [%d] [", opinion);
    
    int p1_bars = opinion / 10;
    int p2_bars = 10 - p1_bars;
    
    for (int i = 0; i < p1_bars; i++) {
        printf("█");
    }
    
    for (int i = 0; i < p2_bars; i++) {
        printf("-");
    }
    
    printf("] P2 [%d]\n", 100 - opinion);
}

void display_hand(GameMessage *msg) {
    printf("\nYour cards:\n");
    for (int i = 0; i < msg->hand_size; i++) {
        printf("%d. %s - %s\n", i + 1, CardNames[msg->cards[i]], CardDescriptions[msg->cards[i]]);
    }
}

#endif