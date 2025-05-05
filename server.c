#include "common.h"
#include "game_logic.h"

typedef struct {
    int deck[DECK_SIZE];
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
    int p1_has_attacked;
    int p2_has_attacked;
} GameState;

void initialize_game(GameState *game) {
    srand(time(NULL));
    create_deck(game->deck, &game->deck_size);
    shuffle_deck(game->deck, game->deck_size);
    game->p1_party = rand() % PARTY_COUNT;
    game->p2_party = rand() % PARTY_COUNT;
    draw_starting_cards(game->deck, &game->deck_size, game->p1_hand, &game->p1_hand_size);
    for(int i = 0; i < game->p1_hand_size; i++) {
        p1_msg.cards[i] = game->p1_hand[i];
    }

    sprintf(p1_msg.message, "Round %d complete. Public Opinion changed by %d%%.",
    game->round, opinion_change);

    p2_msg.type = ROUND_RESULT;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.round = game->round;
    p2_msg.opinion_change = -opinion_change;
    p2_msg.hand_size = game->p2_hand_size;

    for(int i = 0; i < game->p2_hand_size; i++) {
        p2_msg.cards[i] = game->p2_hand[i];
    }

    sprintf(p2_msg.message, "Round %d complete. Public Opinion changed by %d%%.",
    game->round, opinion_change);

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

    game->round++;


}

int check_game_over(GameState* game) {

    if(game->public_opinion <= MIN_OPINION || game->public_opinion >= MAX_OPINION) {
        return 1;
    }

    if(game->round == MAX_ROUNDS) {
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

    p2_msg.type = GAME_OVER;
    p2_msg.public_opinion = game->public_opinion;
    p2_msg.game_over = 1;

    send_message(p1_socket, &p1_msg);
    send_message(p2_socket, &p2_msg);

}

int main() {

    int server_fd, p1_socket, p2_socket;
}
    draw_starting_cards(game->deck, &game->deck_size, game->p2_hand, &game->p2_hand_size);
    game->public_opinion = STARTING_OPINION;
    game->round = 1;
    game->p1_selected_card = -1;
    game->p2_selected_card = -1;
    game->p1_has_attacked = 0;
    game->p2_has_attacked = 0;
}

void send_starting_game(GameState *game, int p1_socket, int p2_socket) {
    GameMessage p1_msg = {START_INIT, game->p1_party, game->public_opinion, game->round, game->p1_hand_size, 0};
    memcpy(p1_msg.cards, game->p1_hand, game->p1_hand_size * sizeof(int));
    sprintf(p1_msg.message, "Welcome! You are %s.", PartyNames[game->p1_party]);
    send_message(p1_socket, &p1_msg);

    GameMessage p2_msg = {START_INIT, game->p2_party, game->public_opinion, game->round, game->p2_hand_size, 0};
    memcpy(p2_msg.cards, game->p2_hand, game->p2_hand_size * sizeof(int));
    sprintf(p2_msg.message, "Welcome! You are %s.", PartyNames[game->p2_party]);
    send_message(p2_socket, &p2_msg);
}

void process_round(GameState *game, int p1_socket, int p2_socket) {
    GameMessage p1_msg = {CARD_SELECTION, 0, game->public_opinion, game->round, game->p1_hand_size, 0};
    memcpy(p1_msg.cards, game->p1_hand, game->p1_hand_size * sizeof(int));
    send_message(p1_socket, &p1_msg);

    GameMessage p2_msg = {CARD_SELECTION, 0, game->public_opinion, game->round, game->p2_hand_size, 0};
    memcpy(p2_msg.cards, game->p2_hand, game->p2_hand_size * sizeof(int));
    send_message(p2_socket, &p2_msg);

    received_message(p1_socket, &p1_msg);
    received_message(p2_socket, &p2_msg);

    game->p1_selected_card = (p1_msg.selected_card >= 0 && p1_msg.selected_card < game->p1_hand_size) ? p1_msg.selected_card : 0;
    game->p2_selected_card = (p2_msg.selected_card >= 0 && p2_msg.selected_card < game->p2_hand_size) ? p2_msg.selected_card : 0;

    int opinion_change = resolve_cards(game->p1_party, game->p1_hand[game->p1_selected_card], game->p2_party, game->p2_hand[game->p2_selected_card]);
    game->public_opinion = clamp(game->public_opinion + opinion_change, MIN_OPINION, MAX_OPINION);

    remove_card(game->p1_hand, &game->p1_hand_size, game->p1_selected_card);
    remove_card(game->p2_hand, &game->p2_hand_size, game->p2_selected_card);

    if (game->deck_size > 0) {
        game->p1_hand[game->p1_hand_size++] = draw_card(game->deck, &game->deck_size);
        game->p2_hand[game->p2_hand_size++] = draw_card(game->deck, &game->deck_size);
    }

    p1_msg = (GameMessage){ROUND_RESULT, 0, game->public_opinion, game->round, game->p1_hand_size, 0, opinion_change};
    memcpy(p1_msg.cards, game->p1_hand, game->p1_hand_size * sizeof(int));
    sprintf(p1_msg.message, "Round %d: Opinion changed by %d%%.", game->round, opinion_change);
    send_message(p1_socket, &p1_msg);

    p2_msg = (GameMessage){ROUND_RESULT, 0, game->public_opinion, game->round, game->p2_hand_size, 0, -opinion_change};
    memcpy(p2_msg.cards, game->p2_hand, game->p2_hand_size * sizeof(int));
    sprintf(p2_msg.message, "Round %d: Opinion changed by %d%%.", game->round, -opinion_change);
    send_message(p2_socket, &p2_msg);

    game->round++;
}

int check_game_over(GameState *game) {
    return (game->public_opinion <= MIN_OPINION || game->public_opinion >= MAX_OPINION || 
            game->round > MAX_ROUNDS || (game->deck_size == 0 && game->p1_hand_size == 0 && game->p2_hand_size == 0));
}

void send_game_over(GameState *game, int p1_socket, int p2_socket) {
    GameMessage msg = {GAME_OVER, 0, game->public_opinion, 0, 0, 1};
    sprintf(msg.message, "Game Over! Final Opinion: %d%%", game->public_opinion);
    send_message(p1_socket, &msg);
    send_message(p2_socket, &msg);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {AF_INET, htons(PORT), INADDR_ANY};
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 2);

    printf("Server started. Waiting for players...\n");
    int p1_socket = accept(server_fd, NULL, NULL);
    printf("Player 1 connected.\n");
    int p2_socket = accept(server_fd, NULL, NULL);
    printf("Player 2 connected. Starting game.\n");

    GameState game;
    initialize_game(&game);
    send_starting_game(&game, p1_socket, p2_socket);

    while (!check_game_over(&game)) {
        process_round(&game, p1_socket, p2_socket);
    }

    send_game_over(&game, p1_socket, p2_socket);
    close(p1_socket);
    close(p2_socket);
    close(server_fd);
    return 0;
}
