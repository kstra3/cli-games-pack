#include "games.h"

typedef enum {
    HEADS = 1,
    TAILS = 2
} CoinSide;

typedef struct {
    int player_score;
    int computer_score;
    int total_flips;
    int streak;
    int best_streak;
} CoinFlipGame;

void display_coin_flip_rules(void) {
    printf("\n===========================================\n");
    printf("              COIN FLIP\n");
    printf("===========================================\n");
    printf("How to play:\n");
    printf("* Call heads or tails before the flip\n");
    printf("* Correct guess = 1 point\n");
    printf("* Try to build the longest winning streak!\n");
    printf("* Beat the computer's lucky guesses\n");
    printf("-------------------------------------------\n");
}

void animate_coin_flip(void) {
    printf("\nFlipping coin");
    fflush(stdout);
    
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        // Simple delay simulation
        for (int j = 0; j < 100000000; j++);
    }
    printf("\n");
}

void display_coin_result(int result, int animated) {
    if (animated) {
        animate_coin_flip();
    }
    
    printf("\n");
    printf("        ***********\n");
    printf("      *             *\n");
    printf("    *                 *\n");
    printf("   *                   *\n");
    printf("  *         %s         *\n", (result == HEADS) ? "HEADS" : "TAILS");
    printf("   *                   *\n");
    printf("    *                 *\n");
    printf("      *             *\n");
    printf("        ***********\n");
    printf("\n");
}

int get_player_call(void) {
    int choice;
    
    printf("\nMake your call:\n");
    printf("1. HEADS\n");
    printf("2. TAILS\n");
    printf("0. Return to main menu\n");
    printf("\nEnter your choice (0-2): ");
    
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return -1; // Invalid input
    }
    
    clear_input_buffer();
    return choice;
}

int flip_coin(void) {
    return (rand() % 2) + 1; // Returns 1 (HEADS) or 2 (TAILS)
}

int get_computer_call(void) {
    return (rand() % 2) + 1; // Computer makes random guess
}

void display_game_stats(const CoinFlipGame* game) {
    printf("\n===========================================\n");
    printf("             GAME STATISTICS\n");
    printf("===========================================\n");
    printf("Your Score:        %d\n", game->player_score);
    printf("Computer Score:    %d\n", game->computer_score);
    printf("Total Flips:       %d\n", game->total_flips);
    printf("Current Streak:    %d\n", game->streak);
    printf("Best Streak:       %d\n", game->best_streak);
    
    if (game->total_flips > 0) {
        float accuracy = ((float)game->player_score / game->total_flips) * 100;
        printf("Your Accuracy:     %.1f%%\n", accuracy);
    }
    printf("===========================================\n");
}

void play_single_coin_flip(CoinFlipGame* game) {
    int player_call = get_player_call();
    
    if (player_call == 0) {
        return; // Return to main menu
    }
    
    if (player_call < 1 || player_call > 2) {
        printf("Invalid choice! Please select 1 or 2.\n");
        return;
    }
    
    int computer_call = get_computer_call();
    int coin_result = flip_coin();
    
    printf("\nYou called: %s\n", (player_call == HEADS) ? "HEADS" : "TAILS");
    printf("Computer called: %s\n", (computer_call == HEADS) ? "HEADS" : "TAILS");
    
    display_coin_result(coin_result, 1);
    
    game->total_flips++;
    
    // Check player result
    int player_won = (player_call == coin_result);
    int computer_won = (computer_call == coin_result);
    
    if (player_won && computer_won) {
        printf("*** BOTH CORRECT! You and computer both guessed right! ***\n");
        game->player_score++;
        game->computer_score++;
        game->streak++;
    } else if (player_won) {
        printf("*** YOU WIN! Great call! ***\n");
        game->player_score++;
        game->streak++;
    } else if (computer_won) {
        printf("*** COMPUTER WINS! Computer got lucky! ***\n");
        game->computer_score++;
        game->streak = 0; // Reset player streak
    } else {
        printf("*** NOBODY WINS! Both guessed wrong! ***\n");
        game->streak = 0; // Reset player streak
    }
    
    // Update best streak
    if (game->streak > game->best_streak) {
        game->best_streak = game->streak;
        if (game->best_streak >= 3) {
            printf("*** NEW PERSONAL BEST STREAK: %d! ***\n", game->best_streak);
        }
    }
    
    // Special messages for streaks
    if (game->streak >= 5) {
        printf("*** INCREDIBLE! %d in a row! You're on fire! ***\n", game->streak);
    } else if (game->streak >= 3) {
        printf("*** NICE STREAK! %d correct in a row! ***\n", game->streak);
    }
    
    printf("\n*** Score: You %d - %d Computer ***\n", 
           game->player_score, game->computer_score);
}

void play_coin_flip_tournament(CoinFlipGame* game) {
    printf("\n>>> COIN FLIP TOURNAMENT MODE! <<<\n");
    printf("First to 5 points wins!\n");
    
    while (game->player_score < 5 && game->computer_score < 5) {
        play_single_coin_flip(game);
        
        if (game->player_score < 5 && game->computer_score < 5) {
            printf("\nContinue tournament? (y/n): ");
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
        }
    }
    
    // Tournament results
    if (game->player_score >= 5) {
        printf("\n*** TOURNAMENT CHAMPION! ***\n");
        printf("You won the tournament %d-%d!\n", game->player_score, game->computer_score);
    } else if (game->computer_score >= 5) {
        printf("\n*** TOURNAMENT OVER! ***\n");
        printf("Computer won the tournament %d-%d!\n", game->computer_score, game->player_score);
    }
}

void play_coin_flip(void) {
    CoinFlipGame game = {0, 0, 0, 0, 0}; // Initialize all fields to 0
    int game_mode;
    
    display_coin_flip_rules();
    
    while (1) {
        printf("\nChoose game mode:\n");
        printf("1. Single Flips (casual play)\n");
        printf("2. Tournament (first to 5 wins)\n");
        printf("3. View Statistics\n");
        printf("0. Return to main menu\n");
        printf("\nSelect mode (0-3): ");
        
        if (scanf("%d", &game_mode) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        
        clear_input_buffer();
        
        switch (game_mode) {
            case 0:
                if (game.total_flips > 0) {
                    display_game_stats(&game);
                }
                printf("\nThanks for playing Coin Flip!\n");
                return;
                
            case 1:
                play_single_coin_flip(&game);
                if (game.total_flips > 0) {
                    printf("\nPlay another flip? (y/n): ");
                    char play_again;
                    if (scanf(" %c", &play_again) == 1) {
                        clear_input_buffer();
                        if (play_again != 'y' && play_again != 'Y') {
                            continue; // Go back to mode selection
                        }
                    } else {
                        clear_input_buffer();
                        continue;
                    }
                }
                break;
                
            case 2:
                // Reset scores for tournament
                game.player_score = 0;
                game.computer_score = 0;
                play_coin_flip_tournament(&game);
                break;
                
            case 3:
                if (game.total_flips > 0) {
                    display_game_stats(&game);
                } else {
                    printf("\nNo games played yet! Start flipping some coins!\n");
                }
                break;
                
            default:
                printf("Invalid selection! Please choose 0-3.\n");
                break;
        }
    }
}
