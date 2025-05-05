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
 } GameState;
 
 void initialize_game(GameState *game) {
 // This function would be part of your client code
 void handle_card_selection_message(GameMessage *msg) {
     printf("\n========================================\n");
     printf("ROUND %d - SELECT YOUR CARD\n", msg->round);
     printf("========================================\n");
     printf("Public Opinion: %d%%\n", msg->public_opinion);
     printf("Your cards:\n");
 
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
 
     for(int i = 0; i < msg->hand_size; i++) {
         printf("%d. %s\n", i+1, get_card_name(msg->cards[i]));
     }
     
     printf("\nSelect a card (1-%d): ", msg->hand_size);
     // Get user input for card selection
     
     // After sending selection to server, display:
     printf("\nWaiting for opponent...\n");
 }
 
 void send_starting_game(GameState * game, int p1_socket, int p2_socket) {
     GameMessage p1_msg, p2_msg;
 // This would be added to your client receive code
 void handle_opponent_selection() {
     printf("\n*** Opponent has locked in their card ***\n");
     printf("Press Enter to reveal cards...\n");
     // Wait for user input
 }
 
     p1_msg.type = START_INIT;
     p1_msg.party = game->p1_party;
     p1_msg.public_opinion = game->public_opinion;
     p1_msg.round = game->round;
     p1_msg.hand_size = game->p1_hand_size;
     p1_msg.game_over = 0;
 // This function would be part of your client code
 void handle_card_reveal_message(GameMessage *msg) {
     printf("\n========================================\n");
     printf("CARD REVEAL\n");
     printf("========================================\n");
     printf("Your card: %s\n", get_card_name(msg->selected_card));
     printf("Opponent's card: %s\n", get_card_name(msg->opponent_card));
 }
 
     for(int i = 0; i < game->p1_hand_size; i++) {
         p1_msg.cards[i] = game->p1_hand[i];
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
 
     sprintf(p1_msg.message, "Welcome to Point Taken! You are playing as %s.", PartyNames[game->p1_party]);
 
     p2_msg.type = START_INIT;
     p2_msg.party = game->p2_party;
     p2_msg.public_opinion = game->public_opinion;
     p2_msg.round = game->round;
     p2_msg.hand_size = game->p2_hand_size;
     p2_msg.game_over = 0;
 
     for(int i = 0; i < game->p2_hand_size; i++) {
         p2_msg.cards[i] = game->p2_hand[i];
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
 
     sprintf(p2_msg.message, "Welcome to Point Taken! You are playing as %s.", PartyNames[game->p2_party]);
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
 
     send_message(p1_socket, &p1_msg);
     send_message(p2_socket, &p2_msg);
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
 @@ -103,218 +134,31 @@ void process_round(GameState * game, int p1_socket, int p2_socket) {
     send_message(p1_socket, &p1_msg);
     send_message(p2_socket, &p2_msg);
 
     // Get player 1's selection
     received_message(p1_socket, &p1_msg);
     received_message(p2_socket, &p2_msg);
 
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
         // ... (existing Tahimik na Sigaw logic)
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
 
     p2_msg.type = CARD_REVEAL;
     p2_msg.selected_card = game->p2_hand[game->p2_selected_card];
     p2_msg.opponent_card = game->p1_hand[game->p1_selected_card];
 
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
 
     sprintf(p1_msg.message, "Round %d complete. Public Opinion changed by %d%%.",
     game->round, opinion_change);
 
     p2_msg.type = ROUND_RESULT;
     p2_msg.public_opinion = game->public_opinion;
     p2_msg.round = game->round;
     p2_msg.opinion_change = -opinion_change;
     p2_msg.hand_size = game->p2_hand_size;
 
     for(int i = 0; i < game->p2_hand_size; i++) {
         p2_msg.cards[i] = game->p2_hand[i];
         // ... (existing Tahimik na Sigaw logic)
     }
 
     sprintf(p2_msg.message, "Round %d complete. Public Opinion changed by %d%%.",
     game->round, opinion_change);
 
     send_message(p1_socket, &p1_msg);
     send_message(p2_socket, &p2_msg);
 
     game->round++;
 
 
     // The rest of the function continues as before...
     // ...
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
     struct sockaddr_in address;
     int opt = 1;
     int addrlen = sizeof(address);
     GameState game;
 
     if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
         perror("Socked Failed");
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
 
     printf("Political Debate Duel Server started on port %d\n", PORT);
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
 
     printf("Game over!\n");
 
     return 0;
 
 
 }
