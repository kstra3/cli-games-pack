#include "games.h"

#define DECK_SIZE 52
#define MAX_HAND_SIZE 10
#define STARTING_CHIPS 100

typedef enum {
    HEARTS, DIAMONDS, CLUBS, SPADES
} Suit;

typedef enum {
    ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING
} Rank;

typedef struct {
    Suit suit;
    Rank rank;
} Card;

typedef struct {
    Card cards[MAX_HAND_SIZE];
    int card_count;
    int value;
    int aces;
    int is_bust;
    int is_blackjack;
} Hand;

typedef struct {
    Card deck[DECK_SIZE];
    int cards_left;
    int current_card;
} Deck;

typedef struct {
    Hand player_hand;
    Hand dealer_hand;
    Deck game_deck;
    int player_chips;
    int current_bet;
    int games_played;
    int games_won;
    int blackjacks;
} BlackjackGame;

const char* suit_names[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
const char* suit_symbols[] = {"H", "D", "C", "S"};
const char* rank_names[] = {"", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

void display_blackjack_rules(void) {
    printf("\n===========================================\n");
    printf("             BLACKJACK (21)\n");
    printf("===========================================\n");
    printf("Rules:\n");
    printf("* Get as close to 21 as possible without going over\n");
    printf("* Face cards (J, Q, K) are worth 10 points\n");
    printf("* Aces are worth 1 or 11 (automatically optimized)\n");
    printf("* Dealer must hit on 16, stand on 17\n");
    printf("* Blackjack (21 with 2 cards) beats regular 21\n");
    printf("* You start with 100 chips\n");
    printf("-------------------------------------------\n");
}

void initialize_deck(Deck* deck) {
    int index = 0;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 1; rank <= 13; rank++) {
            deck->deck[index].suit = (Suit)suit;
            deck->deck[index].rank = (Rank)rank;
            index++;
        }
    }
    deck->cards_left = DECK_SIZE;
    deck->current_card = 0;
}

void shuffle_deck(Deck* deck) {
    for (int i = 0; i < DECK_SIZE; i++) {
        int j = rand() % DECK_SIZE;
        Card temp = deck->deck[i];
        deck->deck[i] = deck->deck[j];
        deck->deck[j] = temp;
    }
    deck->current_card = 0;
    deck->cards_left = DECK_SIZE;
}

Card deal_card(Deck* deck) {
    if (deck->cards_left <= 0) {
        printf("*** Reshuffling deck... ***\n");
        shuffle_deck(deck);
    }
    
    Card card = deck->deck[deck->current_card];
    deck->current_card++;
    deck->cards_left--;
    return card;
}

int get_card_value(Rank rank) {
    if (rank >= JACK) return 10;
    if (rank == ACE) return 11; // Will be adjusted in calculate_hand_value
    return rank;
}

void calculate_hand_value(Hand* hand) {
    hand->value = 0;
    hand->aces = 0;
    
    // First pass: count all cards and aces
    for (int i = 0; i < hand->card_count; i++) {
        if (hand->cards[i].rank == ACE) {
            hand->aces++;
            hand->value += 11;
        } else {
            hand->value += get_card_value(hand->cards[i].rank);
        }
    }
    
    // Adjust aces from 11 to 1 if needed
    while (hand->value > 21 && hand->aces > 0) {
        hand->value -= 10; // Change ace from 11 to 1
        hand->aces--;
    }
    
    hand->is_bust = (hand->value > 21);
    hand->is_blackjack = (hand->card_count == 2 && hand->value == 21);
}

void add_card_to_hand(Hand* hand, Card card) {
    if (hand->card_count < MAX_HAND_SIZE) {
        hand->cards[hand->card_count] = card;
        hand->card_count++;
        calculate_hand_value(hand);
    }
}

void display_card(Card card) {
    printf("[%s%s]", rank_names[card.rank], suit_symbols[card.suit]);
}

void display_hand(const Hand* hand, const char* owner, int hide_first) {
    printf("%s's hand: ", owner);
    
    for (int i = 0; i < hand->card_count; i++) {
        if (i == 0 && hide_first) {
            printf("[??] ");
        } else {
            display_card(hand->cards[i]);
            printf(" ");
        }
    }
    
    if (hide_first) {
        printf("(Hidden total)\n");
    } else {
        printf("(Total: %d)", hand->value);
        if (hand->is_blackjack) {
            printf(" *** BLACKJACK! ***");
        } else if (hand->is_bust) {
            printf(" *** BUST! ***");
        }
        printf("\n");
    }
}

void initialize_hand(Hand* hand) {
    hand->card_count = 0;
    hand->value = 0;
    hand->aces = 0;
    hand->is_bust = 0;
    hand->is_blackjack = 0;
}

int get_bet_amount(int max_chips) {
    int bet;
    
    printf("\nYou have %d chips.\n", max_chips);
    printf("Enter your bet (1-%d, or 0 to quit): ", max_chips);
    
    if (scanf("%d", &bet) != 1) {
        clear_input_buffer();
        return -1;
    }
    
    clear_input_buffer();
    
    if (bet == 0) return 0; // Quit signal
    if (bet < 1 || bet > max_chips) return -1; // Invalid bet
    
    return bet;
}

int get_player_action(const Hand* hand) {
    int action;
    
    printf("\nYour options:\n");
    printf("1. Hit (take another card)\n");
    printf("2. Stand (keep current hand)\n");
    
    if (hand->card_count == 2) {
        printf("3. Double Down (double bet, take one card, then stand)\n");
    }
    
    printf("Enter your choice: ");
    
    if (scanf("%d", &action) != 1) {
        clear_input_buffer();
        return -1;
    }
    
    clear_input_buffer();
    return action;
}

void play_dealer_turn(BlackjackGame* game) {
    printf("\n>>> Dealer's turn <<<\n");
    
    // Reveal dealer's hidden card
    display_hand(&game->dealer_hand, "Dealer", 0);
    
    // Dealer hits on 16, stands on 17
    while (game->dealer_hand.value < 17) {
        printf("\nDealer hits...\n");
        Card card = deal_card(&game->game_deck);
        add_card_to_hand(&game->dealer_hand, card);
        printf("Dealer draws: ");
        display_card(card);
        printf("\n");
        display_hand(&game->dealer_hand, "Dealer", 0);
        
        if (game->dealer_hand.is_bust) {
            printf("\n*** Dealer busts! ***\n");
            break;
        }
    }
    
    if (!game->dealer_hand.is_bust && game->dealer_hand.value >= 17) {
        printf("\nDealer stands.\n");
    }
}

void determine_blackjack_winner(BlackjackGame* game) {
    printf("\n===========================================\n");
    printf("             FINAL RESULTS\n");
    printf("===========================================\n");
    
    display_hand(&game->player_hand, "Player", 0);
    display_hand(&game->dealer_hand, "Dealer", 0);
    
    int payout = 0;
    
    if (game->player_hand.is_bust) {
        printf("\n*** You busted! Dealer wins! ***\n");
        payout = -game->current_bet;
    } else if (game->dealer_hand.is_bust) {
        printf("\n*** Dealer busted! You win! ***\n");
        payout = game->current_bet;
        game->games_won++;
    } else if (game->player_hand.is_blackjack && !game->dealer_hand.is_blackjack) {
        printf("\n*** BLACKJACK! You win 3:2! ***\n");
        payout = (game->current_bet * 3) / 2;
        game->games_won++;
        game->blackjacks++;
    } else if (game->dealer_hand.is_blackjack && !game->player_hand.is_blackjack) {
        printf("\n*** Dealer has blackjack! Dealer wins! ***\n");
        payout = -game->current_bet;
    } else if (game->player_hand.value > game->dealer_hand.value) {
        printf("\n*** You win with %d! ***\n", game->player_hand.value);
        payout = game->current_bet;
        game->games_won++;
    } else if (game->dealer_hand.value > game->player_hand.value) {
        printf("\n*** Dealer wins with %d! ***\n", game->dealer_hand.value);
        payout = -game->current_bet;
    } else {
        printf("\n*** Push! It's a tie! ***\n");
        payout = 0; // No money changes hands
    }
    
    game->player_chips += payout;
    
    if (payout > 0) {
        printf("You won %d chips!\n", payout);
    } else if (payout < 0) {
        printf("You lost %d chips.\n", -payout);
    } else {
        printf("No chips won or lost.\n");
    }
    
    printf("Chips remaining: %d\n", game->player_chips);
    printf("===========================================\n");
}

void display_blackjack_stats(const BlackjackGame* game) {
    printf("\n===========================================\n");
    printf("            GAME STATISTICS\n");
    printf("===========================================\n");
    printf("Games Played:      %d\n", game->games_played);
    printf("Games Won:         %d\n", game->games_won);
    printf("Blackjacks:        %d\n", game->blackjacks);
    printf("Current Chips:     %d\n", game->player_chips);
    
    if (game->games_played > 0) {
        float win_rate = ((float)game->games_won / game->games_played) * 100;
        printf("Win Rate:          %.1f%%\n", win_rate);
    }
    
    int profit = game->player_chips - STARTING_CHIPS;
    printf("Total Profit/Loss: %+d chips\n", profit);
    printf("===========================================\n");
}

void play_blackjack(void) {
    BlackjackGame game = {0};
    game.player_chips = STARTING_CHIPS;
    
    display_blackjack_rules();
    initialize_deck(&game.game_deck);
    shuffle_deck(&game.game_deck);
    
    printf("\nWelcome to Blackjack! You start with %d chips.\n", STARTING_CHIPS);
    
    while (game.player_chips > 0) {
        printf("\n>>> New Hand <<<\n");
        
        // Get bet
        game.current_bet = get_bet_amount(game.player_chips);
        if (game.current_bet == 0) {
            break; // Player wants to quit
        }
        if (game.current_bet == -1) {
            printf("Invalid bet! Please try again.\n");
            continue;
        }
        
        // Initialize hands
        initialize_hand(&game.player_hand);
        initialize_hand(&game.dealer_hand);
        game.games_played++;
        
        // Deal initial cards
        add_card_to_hand(&game.player_hand, deal_card(&game.game_deck));
        add_card_to_hand(&game.dealer_hand, deal_card(&game.game_deck));
        add_card_to_hand(&game.player_hand, deal_card(&game.game_deck));
        add_card_to_hand(&game.dealer_hand, deal_card(&game.game_deck));
        
        // Show initial hands
        printf("\nInitial deal:\n");
        display_hand(&game.player_hand, "Player", 0);
        display_hand(&game.dealer_hand, "Dealer", 1); // Hide dealer's first card
        
        // Check for blackjacks
        if (game.player_hand.is_blackjack || game.dealer_hand.is_blackjack) {
            play_dealer_turn(&game); // Reveal dealer's hand
            determine_blackjack_winner(&game);
        } else {
            // Player's turn
            int doubled_down = 0;
            
            while (!game.player_hand.is_bust) {
                int action = get_player_action(&game.player_hand);
                
                if (action == 1) { // Hit
                    Card card = deal_card(&game.game_deck);
                    add_card_to_hand(&game.player_hand, card);
                    printf("\nYou drew: ");
                    display_card(card);
                    printf("\n");
                    display_hand(&game.player_hand, "Player", 0);
                    
                } else if (action == 2) { // Stand
                    break;
                    
                } else if (action == 3 && game.player_hand.card_count == 2) { // Double Down
                    if (game.current_bet * 2 <= game.player_chips) {
                        game.current_bet *= 2;
                        printf("\n*** Doubled down! Bet is now %d chips ***\n", game.current_bet);
                        
                        Card card = deal_card(&game.game_deck);
                        add_card_to_hand(&game.player_hand, card);
                        printf("You drew: ");
                        display_card(card);
                        printf("\n");
                        display_hand(&game.player_hand, "Player", 0);
                        doubled_down = 1;
                        break;
                    } else {
                        printf("Not enough chips to double down!\n");
                    }
                } else {
                    printf("Invalid action! Please try again.\n");
                }
            }
            
            // Dealer's turn (if player didn't bust)
            if (!game.player_hand.is_bust) {
                play_dealer_turn(&game);
            }
            
            determine_blackjack_winner(&game);
        }
        
        // Check if player is out of chips
        if (game.player_chips <= 0) {
            printf("\n*** GAME OVER! You're out of chips! ***\n");
            display_blackjack_stats(&game);
            break;
        }
        
        // Ask if player wants to continue
        printf("\nPlay another hand? (y/n): ");
        char continue_game;
        if (scanf(" %c", &continue_game) == 1) {
            clear_input_buffer();
            if (continue_game != 'y' && continue_game != 'Y') {
                break;
            }
        } else {
            clear_input_buffer();
            break;
        }
        
        // Reshuffle if deck is getting low
        if (game.game_deck.cards_left < 15) {
            printf("\n*** Reshuffling deck for next hand... ***\n");
            shuffle_deck(&game.game_deck);
        }
    }
    
    if (game.games_played > 0) {
        display_blackjack_stats(&game);
        
        if (game.player_chips > STARTING_CHIPS) {
            printf("\nCongratulations! You left the table with a profit!\n");
        } else if (game.player_chips == STARTING_CHIPS) {
            printf("\nYou broke even! Not bad!\n");
        } else {
            printf("\nBetter luck next time!\n");
        }
    }
    
    printf("\nThanks for playing Blackjack!\n");
}