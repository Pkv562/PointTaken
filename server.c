#include "common.h"
#include "game_logic.h"

// This function would be part of your client code
void handle_card_selection_message(GameMessage *msg) {
    printf("\n========================================\n");
    printf("ROUND %d - SELECT YOUR CARD\n", msg->round);
    printf("========================================\n");
    printf("Public Opinion: %d%%\n", msg->public_opinion);
    printf("Your cards:\n");
    
    for(int i = 0; i < msg->hand_size; i++) {
        printf("%d. %s\n", i+1, get_card_name(msg->cards[i]));
    }
    
    printf("\nSelect a card (1-%d): ", msg->hand_size);
    // Get user input for card selection
    
    // After sending selection to server, display:
    printf("\nWaiting for opponent...\n");
}

// This would be added to your client receive code
void handle_opponent_selection() {
    printf("\n*** Opponent has locked in their card ***\n");
    printf("Press Enter to reveal cards...\n");
    // Wait for user input
}

// This function would be part of your client code
void handle_card_reveal_message(GameMessage *msg) {
    printf("\n========================================\n");
    printf("CARD REVEAL\n");
    printf("========================================\n");
    printf("Your card: %s\n", get_card_name(msg->selected_card));
    printf("Opponent's card: %s\n", get_card_name(msg->opponent_card));
}

// This function would be part of your client code
void handle_round_result_message(GameMessage *msg) {
    printf("\n========================================\n");
    printf("ROUND EFFECTS\n");
    printf("========================================\n");
    printf("%s\n", msg->message);
    printf("Public Opinion is now: %d%%\n", msg->public_opinion);
    
    if(msg->opinion_change > 0) {
        printf("You gained ground! +%d%%\n", msg->opinion_change);
    } else if(msg->opinion_change < 0) {
        printf("You lost ground! %d%%\n", msg->opinion_change);
    } else {
        printf("The public opinion didn't change.\n");
    }
    
    printf("\nYour new hand:\n");
    for(int i = 0; i < msg->hand_size; i++) {
        printf("%d. %s\n", i+1, get_card_name(msg->cards[i]));
    }
    
    printf("\nPress Enter to start the next round...\n");
    // Wait for user input
}

// This function would be part of your client code at the start of the game
void handle_start_init_message(GameMessage *msg) {
    printf("\n========================================\n");
    printf("WELCOME TO POINT TAKEN\n");
    printf("========================================\n");
    printf("%s\n", msg->message);
    printf("\n----------------------------------------\n");
    printf("GAME INFORMATION\n");
    printf("----------------------------------------\n");
    printf("Your party: %s\n", get_party_name(msg->party));
    printf("Starting Public Opinion: %d%%\n", msg->public_opinion);
    printf("Round: %d\n", msg->round);
    
    printf("\n----------------------------------------\n");
    printf("YOUR STARTING CARDS\n");
    printf("----------------------------------------\n");
    for(int i = 0; i < msg->hand_size; i++) {
        printf("%d. %s\n", i+1, get_card_name(msg->cards[i]));
    }
    
    printf("\nReady to begin? Press Enter to start the game...\n");
    // Wait for user input
}

// Helper function to get card name
char* get_card_name(int card_type) {
    switch(card_type) {
        case 0: return "DEFEND";
        case 1: return "ATTACK";
        case 2: return "ATTACK_OPPONENT";
        case 3: return "FACT_CHECK";
        case 4: return "PANDER";
        default: return "UNKNOWN";
    }
}

// Helper function to get party name
char* get_party_name(int party) {
    switch(party) {
        case 0: return "Makabayan";
        case 1: return "Republikano";
        case 2: return "Tahimik na Sigaw";
        case 3: return "Independiente";
        default: return "UNKNOWN";
    }
}

// This would be modifications to your server-side process_round function
void process_round(GameState * game, int p1_socket, int p2_socket) {
    GameMessage p1_msg, p2_msg;

    // Send card selection message to both players
    p1_msg.type = CARD_SELECTION;
    p1_msg.public_opinion = game->public_opinion;
    p1_msg.round = game->round;
    p1_msg.hand_size = game->p1_hand_size;

    for(int i = 0; i < game->p1_hand_size; i++) {
        p1_msg.cards[i] = game->p1_hand[i];
    }

    p2_msg.type = CARD_SELECTION;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.round = game->round;
    p2_msg.hand_size = game->p2_hand_size;

    for(int i = 0; i < game->p2_hand_size; i++) {
        p2_msg.cards[i] = game->p2_hand[i];
    }

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

    // Get player 1's selection
    received_message(p1_socket, &p1_msg);
    game->p1_selected_card = p1_msg.selected_card;
    
    // Notify player 2 that player 1 has locked in their card
    p2_msg.type = OPPONENT_LOCKED_IN;
    send_message(p2_socket, &p2_msg);
    
    // Get player 2's selection
    received_message(p2_socket, &p2_msg);
    game->p2_selected_card = p2_msg.selected_card;
    
    // Notify player 1 that player 2 has locked in their card
    p1_msg.type = OPPONENT_LOCKED_IN;
    send_message(p1_socket, &p1_msg);

    // Apply Tahimik na Sigaw party rules
    if(game->p1_party == TAHIMIK_NA_SIGAW) {
        // ... (existing Tahimik na Sigaw logic)
    }

    if(game->p2_party == TAHIMIK_NA_SIGAW) {
        // ... (existing Tahimik na Sigaw logic)
    }

    // The rest of the function continues as before...
    // ...
}
