#include "common.h"
#include "game_logic.h"

typedef struct {
    int deck[DECK];
    int deck_size;
    int p1_hand[MAX_CARDS];
    int p1_hand_size;
    int p2_hand[MAX_CARDS];
    int p2_hand_size;
    int p1_party;
    int p2_party;
    int public_opinion;
    int round;
    int p1_selected_card;
    int p2_selected_card;
    int p1_attacked;
    int p2_attacked;
    int p1_has_attacked;
    int p2_has_attacked;
    int p1_ready;
    int p2_ready;
} GameState;

void initialize_game(GameState *game) {
    
    srand(time(NULL));

    create_deck(game->deck, &game->deck_size);
    shuffle_deck(game->deck, game->deck_size);

    game->p1_party = rand() % PARTY_COUNT;
    game->p2_party = rand() % PARTY_COUNT;

    draw_starting_cards(game->deck, &game->deck_size, game->p1_hand, &game->p1_hand_size);
    draw_starting_cards(game->deck, &game->deck_size, game->p2_hand, &game->p2_hand_size);

    game->public_opinion = STARTING_OPINION;
    game->round = 1;

    game->p1_selected_card = -1;
    game->p2_selected_card = -1;

    game->p1_has_attacked = 0;
    game->p2_has_attacked = 0;
    game->p1_attacked = 0;
    game->p2_attacked = 0;
    
    game->p1_ready = 0;
    game->p2_ready = 0;
}

void send_starting_game(GameState * game, int p1_socket, int p2_socket) {
    GameMessage p1_msg, p2_msg;

    p1_msg.type = START_INIT;
    p1_msg.party = game->p1_party;
    p1_msg.public_opinion = game->public_opinion;
    p1_msg.round = game->round;
    p1_msg.hand_size = game->p1_hand_size;
    p1_msg.game_over = 0;

    for(int i = 0; i < game->p1_hand_size; i++) {
        p1_msg.cards[i] = game->p1_hand[i];
    }

    sprintf(p1_msg.message, "===== POINT TAKEN: POLITICAL DEBATE DUEL =====\nWelcome to Point Taken! You are playing as %s.", PartyNames[game->p1_party]);

    p2_msg.type = START_INIT;
    p2_msg.party = game->p2_party;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.round = game->round;
    p2_msg.hand_size = game->p2_hand_size;
    p2_msg.game_over = 0;

    for(int i = 0; i < game->p2_hand_size; i++) {
        p2_msg.cards[i] = game->p2_hand[i];
    }

    sprintf(p2_msg.message, "===== POINT TAKEN: POLITICAL DEBATE DUEL =====\nWelcome to Point Taken! You are playing as %s.", PartyNames[game->p2_party]);

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);
}

void process_round(GameState * game, int p1_socket, int p2_socket) {
    GameMessage p1_msg, p2_msg;

    // Reset ready flags
    game->p1_ready = 0;
    game->p2_ready = 0;

    p1_msg.type = CARD_SELECTION;
    p1_msg.public_opinion = game->public_opinion;
    p1_msg.round = game->round;
    p1_msg.hand_size = game->p1_hand_size;

    for(int i = 0; i < game->p1_hand_size; i++) {
        p1_msg.cards[i] = game->p1_hand[i];
    }

    sprintf(p1_msg.message, "===== ROUND %d =====\nSelect a card to play:", game->round);

    p2_msg.type = CARD_SELECTION;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.round = game->round;
    p2_msg.hand_size = game->p2_hand_size;

    for(int i = 0; i < game->p2_hand_size; i++) {
        p2_msg.cards[i] = game->p2_hand[i];
    }

    sprintf(p2_msg.message, "===== ROUND %d =====\nSelect a card to play:", game->round);

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

    // Wait for both players to select cards
    while (!game->p1_ready || !game->p2_ready) {
        // Check if player 1 has submitted their choice
        if (!game->p1_ready) {
            if (received_message(p1_socket, &p1_msg) > 0) {
                if (p1_msg.selected_card >= 0 && p1_msg.selected_card < game->p1_hand_size) {
                    game->p1_selected_card = p1_msg.selected_card;
                    game->p1_ready = 1;
                    
                    // Inform player 1 waiting for opponent
                    strcpy(p1_msg.message, "Card selected. Waiting for opponent...");
                    p1_msg.type = INFO_MESSAGE;
                    send_message(p1_socket, &p1_msg);
                    
                    // If player 2 is already ready, inform them
                    if (game->p2_ready) {
                        strcpy(p2_msg.message, "Opponent has locked in their card");
                        p2_msg.type = INFO_MESSAGE;
                        send_message(p2_socket, &p2_msg);
                    }
                } else {
                    // Invalid card selection
                    strcpy(p1_msg.message, "Invalid card selection. Please try again.");
                    p1_msg.type = INFO_MESSAGE;
                    send_message(p1_socket, &p1_msg);
                    
                    // Resend card selection prompt
                    p1_msg.type = CARD_SELECTION;
                    for(int i = 0; i < game->p1_hand_size; i++) {
                        p1_msg.cards[i] = game->p1_hand[i];
                    }
                    sprintf(p1_msg.message, "===== ROUND %d =====\nSelect a card to play:", game->round);
                    send_message(p1_socket, &p1_msg);
                }
            }
        }
        
        // Check if player 2 has submitted their choice
        if (!game->p2_ready) {
            if (received_message(p2_socket, &p2_msg) > 0) {
                if (p2_msg.selected_card >= 0 && p2_msg.selected_card < game->p2_hand_size) {
                    game->p2_selected_card = p2_msg.selected_card;
                    game->p2_ready = 1;
                    
                    // Inform player 2 waiting for opponent
                    strcpy(p2_msg.message, "Card selected. Waiting for opponent...");
                    p2_msg.type = INFO_MESSAGE;
                    send_message(p2_socket, &p2_msg);
                    
                    // If player 1 is already ready, inform them
                    if (game->p1_ready) {
                        strcpy(p1_msg.message, "Opponent has locked in their card");
                        p1_msg.type = INFO_MESSAGE;
                        send_message(p1_socket, &p1_msg);
                    }
                } else {
                    // Invalid card selection
                    strcpy(p2_msg.message, "Invalid card selection. Please try again.");
                    p2_msg.type = INFO_MESSAGE;
                    send_message(p2_socket, &p2_msg);
                    
                    // Resend card selection prompt
                    p2_msg.type = CARD_SELECTION;
                    for(int i = 0; i < game->p2_hand_size; i++) {
                        p2_msg.cards[i] = game->p2_hand[i];
                    }
                    sprintf(p2_msg.message, "===== ROUND %d =====\nSelect a card to play:", game->round);
                    send_message(p2_socket, &p2_msg);
                }
            }
        }
        
        // Small sleep to prevent CPU hogging
        usleep(10000);
    }

    if(game->p1_party == TAHIMIK_NA_SIGAW) {
        if((game->p1_hand[game->p1_selected_card] == ATTACK || game->p1_hand[game->p1_selected_card] == ATTACK_OPPONENT)
    && game->p1_has_attacked) {
            do {
                game->p1_selected_card = rand() % game->p1_hand_size;
            }
            while (game->p1_hand[game->p1_selected_card] == ATTACK || game->p1_hand[game->p1_selected_card] == ATTACK_OPPONENT);

        }
        else if(game->p1_hand[game->p1_selected_card] == ATTACK || game->p1_hand[game->p1_selected_card] == ATTACK_OPPONENT) {
            game->p1_has_attacked = 1;
        }
    }

    if(game->p2_party == TAHIMIK_NA_SIGAW) {
        if((game->p2_hand[game->p2_selected_card] == ATTACK || game->p2_hand[game->p2_selected_card] == ATTACK_OPPONENT)
    && game->p2_has_attacked) {
            do {
                game->p2_selected_card = rand() % game->p2_hand_size;
            }
            while (game->p2_hand[game->p2_selected_card] == ATTACK || game->p2_hand[game->p2_selected_card] == ATTACK_OPPONENT);

        }
        else if(game->p2_hand[game->p2_selected_card] == ATTACK || game->p2_hand[game->p2_selected_card] == ATTACK_OPPONENT) {
            game->p2_has_attacked = 1;
        }
    }

    p1_msg.type = CARD_REVEAL;
    p1_msg.selected_card = game->p1_hand[game->p1_selected_card];
    p1_msg.opponent_card = game->p2_hand[game->p2_selected_card];
    sprintf(p1_msg.message, "===== CARDS REVEALED =====");

    p2_msg.type = CARD_REVEAL;
    p2_msg.selected_card = game->p2_hand[game->p2_selected_card];
    p2_msg.opponent_card = game->p1_hand[game->p1_selected_card];
    sprintf(p2_msg.message, "===== CARDS REVEALED =====");

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

    int opinion_change = resolve_cards(
        game->p1_party, game->p1_hand[game->p1_selected_card],
        game->p2_party, game->p2_hand[game->p2_selected_card]
    );

    game->public_opinion += opinion_change;

    if(game->public_opinion < MIN_OPINION) {
        game->public_opinion = MIN_OPINION;
    }
    else if(game->public_opinion > MAX_OPINION) {
        game->public_opinion = MAX_OPINION;
    }

    for(int i = game->p1_selected_card; i < game->p1_hand_size - 1; i++) {
        game->p1_hand[i] = game->p1_hand[i+1];
    }
    game->p1_hand_size--;

    for(int i = game->p2_selected_card; i < game->p2_hand_size - 1; i++) {
        game->p2_hand[i] = game->p2_hand[i+1];
    }
    game->p2_hand_size--;

    if(game->deck_size > 0) {
        game->p1_hand[game->p1_hand_size++] = draw_cards(game->deck, &game->deck_size);
    }

    if(game->deck_size > 0) {
        game->p2_hand[game->p2_hand_size++] = draw_cards(game->deck, &game->deck_size);
    }

    p1_msg.type = ROUND_RESULT;
    p1_msg.public_opinion = game->public_opinion;
    p1_msg.round = game->round;
    p1_msg.opinion_change = opinion_change;
    p1_msg.hand_size = game->p1_hand_size;

    for(int i = 0; i < game->p1_hand_size; i++) {
        p1_msg.cards[i] = game->p1_hand[i];
    }

    sprintf(p1_msg.message, "===== ROUND %d RESULTS =====\nPublic Opinion changed by %d%%.",
    game->round, opinion_change);

    p2_msg.type = ROUND_RESULT;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.round = game->round;
    p2_msg.opinion_change = -opinion_change;
    p2_msg.hand_size = game->p2_hand_size;

    for(int i = 0; i < game->p2_hand_size; i++) {
        p2_msg.cards[i] = game->p2_hand[i];
    }

    sprintf(p2_msg.message, "===== ROUND %d RESULTS =====\nPublic Opinion changed by %d%%.",
    game->round, -opinion_change);

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

    game->round++;
}

int check_game_over(GameState* game) {

    if(game->public_opinion <= MIN_OPINION || game->public_opinion >= MAX_OPINION) {
        return 1;
    }

    if(game->round > MAX_ROUNDS) {
        return 1;
    }

    if(game->deck_size == 0 && game->p1_hand_size == 0 && game->p2_hand_size == 0) {
        return 1;
    }

    return 0;
}

void send_game_over(GameState* game, int p1_socket, int p2_socket) {
    GameMessage p1_msg, p2_msg;

    p1_msg.type = GAME_OVER;
    p1_msg.public_opinion = game->public_opinion;
    p1_msg.game_over = 1;
    
    sprintf(p1_msg.message, "===== GAME OVER =====\nFinal Public Opinion: %d%%", game->public_opinion);
    
    if (game->public_opinion > 50) {
        strcat(p1_msg.message, "\nYou won the debate!");
    } else if (game->public_opinion < 50) {
        strcat(p1_msg.message, "\nYou lost the debate.");
    } else {
        strcat(p1_msg.message, "\nThe debate ended in a tie.");
    }

    p2_msg.type = GAME_OVER;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.game_over = 1;
    
    sprintf(p2_msg.message, "===== GAME OVER =====\nFinal Public Opinion: %d%%", game->public_opinion);
    
    if (game->public_opinion < 50) {
        strcat(p2_msg.message, "\nYou won the debate!");
    } else if (game->public_opinion > 50) {
        strcat(p2_msg.message, "\nYou lost the debate.");
    } else {
        strcat(p2_msg.message, "\nThe debate ended in a tie.");
    }

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);
}

int main() {

    int server_fd, p1_socket, p2_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    GameState game;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 2) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("===== POINT TAKEN: POLITICAL DEBATE DUEL =====\n");
    printf("Server started on port %d\n", PORT);
    printf("Waiting for players to connect...\n");

    if((p1_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Player 1 connected!\n");
    printf("Waiting for player 2...\n");

    if((p2_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Player 2 connected!\n");
    printf("Starting the game...\n");

    initialize_game(&game);

    send_starting_game(&game, p1_socket, p2_socket);

    while(!check_game_over(&game)) {
        process_round(&game, p1_socket, p2_socket);
    }

    send_game_over(&game, p1_socket, p2_socket);

    close(p1_socket);
    close(p2_socket);
    close(server_fd);

    printf("===== GAME OVER =====\n");

    return 0;
}
