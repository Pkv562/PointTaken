#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "common.h"

void create_deck(int *deck, int *deck_size) {
    *deck_size = 0;

    for(int i = 0; i < CARD_COUNT; i++) {
        for(int j = 0; j < 2; j++) {
            deck[(*deck_size)++] = i;
        }
    }
}

void shuffle_deck(int *deck, int deck_size) {
    for(int i = deck_size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

int draw_cards(int *deck, int *deck_size) {
    if(*deck_size <= 0) {
        return -1;
    }

    return deck[(*deck_size)--];
}

void draw_starting_cards(int *deck, int *deck_size, int *hand, int *hand_size) {
    *hand_size = 0;

    for(int i = 0; i < 5; i++) {
        if(*deck_size > 0) {
            hand[(*hand_size)++] = draw_cards(deck, deck_size);
        }
    }
}

int party_effects(int party, int card, int opponent_card, int change) {
    int score = change;

    switch(party) {
        case PARTIDO_BAYANIHAN:
            
            if(change < 0) {
                score = change / 2;
            }

            if(card == ATTACK || card == ATTACK_OPPONENT || card == DISTRUST_DATA || FACT_CHECK) {
                score = (change * 75) / 100;
            }
            break;

        case LAKAS_NG_MASA:
            
            if(card == ATTACK || card == ATTACK_OPPONENT) {
                score += 5;
            }

            if(change < 0) {
                score -= 5;
            }
            break;

        case PARTIDO_KALIKASAN:
            
            if(card == APPEAL_TO_EMOTION || card == PERSONAL_STORY) {
                score += 5;
            }

            if(card == USE_STATISTICS) {
                score = (change * 50) / 100;
            }
            break;

        case MAKABAGONG_TEKNOLOHIYA:
            
            if(card == USE_STATISTICS && opponent_card == DISTRUST_DATA) {
                score = 7;
            }

            if(card == USE_STATISTICS) {
                score = (change * 150) / 100;
            }

            if(card == APPEAL_TO_EMOTION || card == PERSONAL_STORY) {
                score = 0;
            }
            break;

        case TAHIMIK_NA_SIGAW:
            
            if(card == STAY_SILENT || card == CHANGE_TOPIC) {
                score += 10;
            }
            break;
        
    }

    return score;
}

int resolve_cards(int p1_party, int p1_card, int p2_party, int p2_card) {
    int opinion_score = 0;

    switch(p1_card) {
        case APPEAL_TO_EMOTION:
            if(p2_card == FACT_CHECK) {
                opinion_score -= 5;
            }
            else {
                opinion_score += 10;
            }
            break;

        case USE_STATISTICS:
            if(p2_card == DISTRUST_DATA) {
                opinion_score -= 5;
            }
            else {
                opinion_score += 15;
            }
            break;

        case ATTACK_OPPONENT:
            if(p2_card == DEFEND || p2_card == FACT_CHECK) {
                opinion_score -= 5;
            }
            else {
                opinion_score += 5;
            }
            break;

        case CHANGE_TOPIC:
            break;

        case PERSONAL_STORY:
            opinion_score += 8;

        case DISTRUST_DATA:
            if(p2_card == USE_STATISTICS) {
                opinion_score += 15;
            }
            else {
                opinion_score -= 3;
            }
            break;

        case FACT_CHECK:
            if(p2_card == APPEAL_TO_EMOTION || p2_card == ATTACK) {
                opinion_score -= 5;
            }
            else {
                opinion_score += 5;
            }
            break;

        case ATTACK:
            if(p2_card == DEFEND || p2_card == FACT_CHECK) {
                opinion_score -= 5;
            }
            else {
                opinion_score += 6;
            }
            break;

        case DEFEND:
            break;

        case STAY_SILENT:
            if(p2_card == ATTACK || p2_card == ATTACK_OPPONENT) {
                opinion_score += 3;
            }
            else {
            }   
            break;
    }

    int p2_effect = 0;
    
    switch (p2_card) {
        case APPEAL_TO_EMOTION:
            if (p1_card == FACT_CHECK) {
                p2_effect += 5;  
            } else {
                p2_effect -= 10; 
            }
            break;
            
        case USE_STATISTICS:
            if (p1_card == DISTRUST_DATA) {
                p2_effect += 5;  
            } else {
                p2_effect -= 15; 
            }
            break;
            
        case ATTACK_OPPONENT:
            if (p1_card == DEFEND || p1_card == FACT_CHECK) {
                p2_effect += 5;  
            } else {
                p2_effect -= 5;
            }
            break;
            
        case CHANGE_TOPIC:
            opinion_score = 0;
            break;
            
        case PERSONAL_STORY:
            p2_effect -= 8;  
            break;
            
        case DISTRUST_DATA:
            if (p1_card == USE_STATISTICS) {
            } else {
                p2_effect += 3;  
            }
            break;
            
        case FACT_CHECK:
            if (p1_card == APPEAL_TO_EMOTION || p1_card == ATTACK) {
            } else {
                p2_effect -= 5;  
            }
            break;
            
        case ATTACK:
            if (p1_card == DEFEND || p1_card == FACT_CHECK) {
                p2_effect += 5; 
            } else {
                p2_effect -= 6;
            }
            break;
            
        case DEFEND:
            break;
            
        case STAY_SILENT:
            if (p1_card == ATTACK || p1_card == ATTACK_OPPONENT) {
                p2_effect -= 3;
            } else {
            }
            break;
    }

    opinion_score = party_effects(p1_party, p1_card, p2_party, p2_card, opinion_score);
    p2_effect = party_effects(p2_party, p2_card, p1_party, p1_card, p2_effect);

    return opinion_score + p2_effect;
}

#endif