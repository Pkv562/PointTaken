#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>     
#include <unistd.h>  
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <time.h>

#define PORT 6969
#define BUFFER_SIZE 1024

#define DECK 20
#define MAX_CARDS 5
#define MAX_ROUNDS 10
#define STARTING_OPINION 50
#define MAX_OPINION 100
#define MIN_OPINION 0
#define CARD_COUNT 10
#define PARTY_COUNT 5

#define START_INIT 1
#define CARD_SELECTION 2
#define CARD_REVEAL 3
#define ROUND_RESULT 4
#define GAME_OVER 5
#define ERROR 6

typedef enum {
    PARTIDO_BAYANIHAN,
    LAKAS_NG_MASA,
    PARTIDO_KALIKASAN,
    MAKABAGONG_TEKNOLOHIYA,
    TAHIMIK_NA_SIGAW,
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
} CardType;

const char* CardNames[CARD_COUNT] = {
    "Appeal",
    "Stats",
    "OppAtt",
    "Topic",
    "Story",
    "Distrst",
    "FactChk",
    "Attack",
    "Defend",
    "Silent"
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
    int opponent_party;  
    int round;               
    int cards[MAX_CARDS]; 
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

int received_message(int socket, GameMessage* msg) {
    return recv(socket, (char*)msg, sizeof(GameMessage), 0);
}

void display_opinion_bar(int opinion) {
    printf("\n         P1 [%d] [", opinion);
    
    int p1_bars = opinion / 10;
    int p2_bars = 10 - p1_bars;
    
    for (int i = 0; i < p1_bars; i++) {
        printf("█");
    }
    
    for (int i = 0; i < p2_bars; i++) {
        printf("-");
    }
    
    printf("] P2 [%d]\n\n", 100 - opinion);
}

void display_terminal_header(int round, int (*round_history)[2], int history_size) {
    printf("---------------\n");
    printf("|   ROUND %-3d |\n", round);
    printf("---------------\n");
    printf("| HISTORY     |\n");
    
    if (history_size > 0) {
        int display_count = history_size > 3 ? 3 : history_size;
        for (int i = history_size - display_count; i < history_size; i++) {
            printf("| > P1 %-7s |\n", CardNames[round_history[i][0]]);
            printf("| > P2 %-7s |\n", CardNames[round_history[i][1]]);
        }
    } else {
        printf("| > No moves   |\n");
        printf("| > yet        |\n");
    }
    
    printf("|             |\n");
    printf("---------------\n");
}

void display_party_boxes(int player_party, int opponent_party) {
    printf("\n     ----------------   |   ----------------\n");
    printf("     | %-14s |   |   | %-14s |\n", PartyNames[player_party], PartyNames[opponent_party]);
    printf("     ----------------   |   ----------------\n");
}

void display_played_cards(int player_card, int opponent_card) {
    if (player_card >= 0 && opponent_card >= 0) {
        printf("                        |\n");
        printf("        ----------      |      ----------\n");
        printf("        |        |      |      |        |\n");
        printf("        | %-6s |      |      | %-6s |\n", 
               CardNames[player_card], CardNames[opponent_card]);
        printf("        |  CARD  |      |      |  CARD  |\n");
        printf("        |        |      |      |        |\n");
        printf("        ----------      |      ----------\n");
        printf("                        |\n");
    } else {
        printf("                        |\n");
        printf("                        |\n");
        printf("                        |\n");
        printf("                        |\n");
        printf("                        |\n");
    }
}

void display_hand(GameMessage *msg) {
    printf("--------- --------- --------- --------- ---------\n");
    printf("|       | |       | |       | |       | |       |\n");
    
    for (int i = 0; i < MAX_CARDS; i++) {
        if (i < msg->hand_size) {
            const char* card_name = CardNames[msg->cards[i]];
            int len = strlen(card_name);
            if (len > 7) {
                char shortened[8];
                strncpy(shortened, card_name, 7);
                shortened[7] = '\0';
                printf("| %-7s ", shortened);
            } else {
                printf("| %-7s ", card_name);
            }
        } else {
            printf("|        ");
        }
    }
    printf("|\n");
    
    for (int i = 0; i < MAX_CARDS; i++) {
        if (i < msg->hand_size) {
            printf("| CARD %-2d ", i + 1);
        } else {
            printf("|        ");
        }
    }
    printf("|\n");
    
    printf("|       | |       | |       | |       | |       |\n");
    printf("--------- --------- --------- --------- ---------\n");
}

void display_full_terminal(GameMessage *msg, int player_party, int (*round_history)[2], int history_size, int player_card, int opponent_card) {
    printf("\033[2J\033[H"); 
    display_terminal_header(msg->round, round_history, history_size);
    display_opinion_bar(msg->public_opinion);
    display_party_boxes(player_party, msg->opponent_party);
    display_played_cards(player_card, opponent_card);
    display_hand(msg);
}

#endif