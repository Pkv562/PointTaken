/* game_logic.h - Game logic functions */
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "common.h"

/* Initialize a new deck of cards */
void init_deck(int *deck, int *deck_size) {
    *deck_size = 0;
    
    /* Add multiple copies of each card to the deck */
    for (int i = 0; i < CARD_COUNT; i++) {
        for (int j = 0; j < 2; j++) {  /* 2 copies of each card */
            deck[(*deck_size)++] = i;
        }
    }
}

/* Shuffle the deck using Fisher-Yates algorithm */
void shuffle_deck(int *deck, int deck_size) {
    for (int i = deck_size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

/* Draw a card from the deck */
int draw_card(int *deck, int *deck_size) {
    if (*deck_size <= 0) {
        return -1;  /* Deck is empty */
    }
    
    return deck[--(*deck_size)];
}

/* Draw initial hand of cards */
void draw_initial_hand(int *deck, int *deck_size, int *hand, int *hand_size) {
    *hand_size = 0;
    
    for (int i = 0; i < 5; i++) {
        if (*deck_size > 0) {
            hand[(*hand_size)++] = draw_card(deck, deck_size);
        }
    }
}

/* Apply party effects to the opinion change */
int apply_party_effects(int party, int card, int opponent_card, int base_change) {
    int actual_change = base_change;
    
    switch (party) {
        case PARTIDO_BAYANIHAN:
            /* Public opinion penalties from any counter is reduced by 50% */
            if (base_change < 0) {
                actual_change = base_change / 2;
            }
            /* Gains only 75% of the score when using aggressive cards */
            if (card == ATTACK || card == ATTACK_OPPONENT) {
                actual_change = (base_change * 75) / 100;
            }
            break;
            
        case LAKAS_NG_MASA:
            /* "Attack" and "Attack Opponent" gain an extra +5% boost */
            if (card == ATTACK || card == ATTACK_OPPONENT) {
                actual_change += 5;
            }
            /* Takes an extra -5% when a move is countered */
            if (base_change < 0) {
                actual_change -= 5;
            }
            break;
            
        case PARTIDO_KALIKASAN:
            /* "Appeal to Emotion" and "Personal Story" gain an extra +5% score */
            if (card == APPEAL_TO_EMOTION || card == PERSONAL_STORY) {
                actual_change += 5;
            }
            /* The "Use statistic" is 50% less effective */
            if (card == USE_STATISTICS) {
                actual_change = (base_change * 50) / 100;
            }
            break;
            
        case MAKABAGONG_TEKNOLOHIYA:
            /* "Use Statistic" is only half countered (still gives around +7%) */
            if (card == USE_STATISTICS && opponent_card == DISTRUST_DATA) {
                actual_change = 7;
            }
            /* When used increase the score by 50% */
            if (card == USE_STATISTICS) {
                actual_change = (base_change * 150) / 100;
            }
            /* "Appeal to emotion" and "Personal Story" has no effect */
            if (card == APPEAL_TO_EMOTION || card == PERSONAL_STORY) {
                actual_change = 0;
            }
            break;
            
        case TAHIMIK_NA_SIGAW:
            /* "Stay silent" and "Change topic" grant a bonus +5% when used */
            if (card == STAY_SILENT || card == CHANGE_TOPIC) {
                actual_change += 5;
            }
            break;
    }
    
    return actual_change;
}

/* Resolve the effects of cards and return the opinion change */
int resolve_cards(int p1_party, int p1_card, int p2_party, int p2_card) {
    int opinion_change = 0;
    
    /* Process P1's card */
    switch (p1_card) {
        case APPEAL_TO_EMOTION:
            if (p2_card == FACT_CHECK) {
                opinion_change -= 5;  /* Countered */
            } else {
                opinion_change += 10;  /* Base effect */
            }
            break;
            
        case USE_STATISTICS:
            if (p2_card == DISTRUST_DATA) {
                opinion_change -= 5;  /* Countered */
            } else {
                opinion_change += 15;  /* Base effect */
            }
            break;
            
        case ATTACK_OPPONENT:
            if (p2_card == DEFEND || p2_card == FACT_CHECK) {
                opinion_change -= 5;  /* Countered */
            } else {
                opinion_change += 5;  /* Base effect */
            }
            break;
            
        case CHANGE_TOPIC:
            /* Nullifies opponent's move this round */
            /* No direct opinion change */
            break;
            
        case PERSONAL_STORY:
            opinion_change += 8;  /* Immune to Attack or Distrust Data */
            break;
            
        case DISTRUST_DATA:
            if (p2_card == USE_STATISTICS) {
                /* Already handled in USE_STATISTICS case */
            } else {
                opinion_change -= 3;  /* Minor loss if used alone */
            }
            break;
            
        case FACT_CHECK:
            if (p2_card == APPEAL_TO_EMOTION || p2_card == ATTACK) {
                /* Already handled in their respective cases */
            } else {
                opinion_change += 5;  /* Base effect */
            }
            break;
            
        case ATTACK:
            if (p2_card == DEFEND || p2_card == FACT_CHECK) {
                opinion_change -= 5;  /* Countered */
            } else {
                opinion_change += 6;  /* Base effect */
            }
            break;
            
        case DEFEND:
            /* No direct opinion change, just counters attacks */
            break;
            
        case STAY_SILENT:
            if (p2_card == ATTACK || p2_card == ATTACK_OPPONENT) {
                opinion_change += 3;  /* Extra points when opponent attacks */
            } else {
                /* No change otherwise */
            }
            break;
    }
    
    /* Process P2's card (in the opposite direction) */
    int p2_effect = 0;
    
    switch (p2_card) {
        case APPEAL_TO_EMOTION:
            if (p1_card == FACT_CHECK) {
                p2_effect += 5;  /* Countered */
            } else {
                p2_effect -= 10;  /* Base effect */
            }
            break;
            
        case USE_STATISTICS:
            if (p1_card == DISTRUST_DATA) {
                p2_effect += 5;  /* Countered */
            } else {
                p2_effect -= 15;  /* Base effect */
            }
            break;
            
        case ATTACK_OPPONENT:
            if (p1_card == DEFEND || p1_card == FACT_CHECK) {
                p2_effect += 5;  /* Countered */
            } else {
                p2_effect -= 5;  /* Base effect */
            }
            break;
            
        case CHANGE_TOPIC:
            /* Nullifies opponent's move this round */
            opinion_change = 0;
            break;
            
        case PERSONAL_STORY:
            p2_effect -= 8;  /* Immune to Attack or Distrust Data */
            break;
            
        case DISTRUST_DATA:
            if (p1_card == USE_STATISTICS) {
                /* Already handled in USE_STATISTICS case */
            } else {
                p2_effect += 3;  /* Minor loss if used alone */
            }
            break;
            
        case FACT_CHECK:
            if (p1_card == APPEAL_TO_EMOTION || p1_card == ATTACK) {
                /* Already handled in their respective cases */
            } else {
                p2_effect -= 5;  /* Base effect */
            }
            break;
            
        case ATTACK:
            if (p1_card == DEFEND || p1_card == FACT_CHECK) {
                p2_effect += 5;  /* Countered */
            } else {
                p2_effect -= 6;  /* Base effect */
            }
            break;
            
        case DEFEND:
            /* No direct opinion change, just counters attacks */
            break;
            
        case STAY_SILENT:
            if (p1_card == ATTACK || p1_card == ATTACK_OPPONENT) {
                p2_effect -= 3;  /* Extra points when opponent attacks */
            } else {
                /* No change otherwise */
            }
            break;
    }
    
    /* Apply party-specific effects */
    opinion_change = apply_party_effects(p1_party, p1_card, p2_card, opinion_change);
    p2_effect = apply_party_effects(p2_party, p2_card, p1_card, p2_effect);
    
    /* Special case for Tahimik na Sigaw party - can only use Attack or Attack Opponent once per game */
    /* (This needs to be tracked separately in the game state) */
    
    return opinion_change + p2_effect;
}

/* Generate a news headline based on the game outcome */
void generate_headline(int final_opinion, char *headline, int buffer_size) {
    const char *p1_winning_headlines[] = {
        "LANDSLIDE VICTORY: Player 1 dominates the debate with compelling arguments!",
        "MASTERFUL PERFORMANCE: Player 1 leaves opponent speechless in historic debate!",
        "DEBATE CHAMPION: Player 1's rhetoric proves unbeatable in political showdown!"
    };
    
    const char *p2_winning_headlines[] = {
        "STUNNING UPSET: Player 2 overwhelms opponent in remarkable debate performance!",
        "DECISIVE VICTORY: Player 2's arguments demolish rival in political face-off!",
        "DEBATE DOMINANCE: Player 2 emerges triumphant in battle of wits and words!"
    };
    
    const char *draw_headlines[] = {
        "DEADLOCK: Historic debate ends in unprecedented tie as neither side gives ground!",
        "STANDOFF: Evenly matched debaters battle to a draw in political showdown!",
        "PHOTO FINISH: Debate ends with voters split exactly down the middle!"
    };
    
    int index = rand() % 3;
    
    if (final_opinion >= 75) {
        snprintf(headline, buffer_size, "%s", p1_winning_headlines[index]);
    } else if (final_opinion <= 25) {
        snprintf(headline, buffer_size, "%s", p2_winning_headlines[index]);
    } else {
        snprintf(headline, buffer_size, "%s", draw_headlines[index]);
    }
}

#endif /* GAME_LOGIC_H */