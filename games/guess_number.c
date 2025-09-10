#include "games.h"

void display_guess_rules(void) {
    printf("\n===========================================\n");
    printf("            GUESS THE NUMBER\n");
    printf("===========================================\n");
    printf("How to play:\n");
    printf("* I'll think of a number between 1 and 100\n");
    printf("* You have to guess it in as few tries as possible\n");
    printf("* I'll give you hints: 'Higher' or 'Lower'\n");
    printf("* Try to beat your best score!\n");
    printf("-------------------------------------------\n");
}

int get_difficulty_level(void) {
    int difficulty;
    
    printf("\nChoose difficulty level:\n");
    printf("1. Easy   (1-50,  unlimited attempts)\n");
    printf("2. Medium (1-100, 15 attempts)\n");
    printf("3. Hard   (1-200, 10 attempts)\n");
    printf("4. Expert (1-500, 8 attempts)\n");
    printf("0. Return to main menu\n");
    printf("\nSelect difficulty (0-4): ");
    
    if (scanf("%d", &difficulty) != 1) {
        clear_input_buffer();
        return -1;
    }
    
    clear_input_buffer();
    return difficulty;
}

void play_single_round(int max_number, int max_attempts) {
    int secret_number = (rand() % max_number) + 1;
    int guess, attempts = 0;
    int won = 0;
    
    printf("\n>>> I've picked a number between 1 and %d!\n", max_number);
    if (max_attempts > 0) {
        printf("You have %d attempts to guess it.\n", max_attempts);
    } else {
        printf("You have unlimited attempts to guess it.\n");
    }
    printf("Good luck!\n");
    
    while (!won && (max_attempts == 0 || attempts < max_attempts)) {
        printf("\nAttempt #%d", attempts + 1);
        if (max_attempts > 0) {
            printf(" (Remaining: %d)", max_attempts - attempts);
        }
        printf("\nEnter your guess: ");
        
        if (scanf("%d", &guess) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        
        clear_input_buffer();
        attempts++;
        
        if (guess < 1 || guess > max_number) {
            printf("Please enter a number between 1 and %d!\n", max_number);
            attempts--; // Don't count invalid guesses
            continue;
        }
        
        if (guess == secret_number) {
            won = 1;
            printf("\n*** CONGRATULATIONS! ***\n");
            printf("You guessed the number %d correctly!\n", secret_number);
            printf("It took you %d attempt%s.\n", attempts, (attempts == 1) ? "" : "s");
            
            // Calculate and display score based on efficiency
            int efficiency_score = max_number / attempts;
            printf("\n*** Your efficiency score: %d points ***\n", efficiency_score);
            
            if (attempts == 1) {
                printf("*** INCREDIBLE! First try! You must be psychic! ***\n");
            } else if (attempts <= 3) {
                printf("*** AMAZING! Outstanding guessing skills! ***\n");
            } else if (attempts <= 6) {
                printf("*** Great job! Well done! ***\n");
            } else if (attempts <= 10) {
                printf("*** Good work! ***\n");
            } else {
                printf("*** You got it! Practice makes perfect! ***\n");
            }
            
        } else if (guess < secret_number) {
            printf(">>> Too LOW! Try a HIGHER number.\n");
        } else {
            printf(">>> Too HIGH! Try a LOWER number.\n");
        }
        
        // Give additional hints for harder difficulties
        if (max_attempts > 0 && attempts >= max_attempts / 2 && !won) {
            int diff = abs(guess - secret_number);
            if (diff <= 5) {
                printf("*** You're very close! (Within 5) ***\n");
            } else if (diff <= 15) {
                printf("*** You're getting warmer! (Within 15) ***\n");
            } else if (diff <= 30) {
                printf("*** You're still quite far... (Within 30) ***\n");
            }
        }
    }
    
    if (!won) {
        printf("\n*** Game Over! You've used all %d attempts. ***\n", max_attempts);
        printf("The number was: %d\n", secret_number);
        printf("Better luck next time!\n");
    }
}

void play_guess_number(void) {
    int difficulty;
    int max_number, max_attempts;
    
    display_guess_rules();
    
    while (1) {
        difficulty = get_difficulty_level();
        
        switch (difficulty) {
            case 0:
                return; // Return to main menu
                
            case 1: // Easy
                max_number = 50;
                max_attempts = 0; // Unlimited
                printf("\n>>> Easy mode selected!");
                break;
                
            case 2: // Medium
                max_number = 100;
                max_attempts = 15;
                printf("\n>>> Medium mode selected!");
                break;
                
            case 3: // Hard
                max_number = 200;
                max_attempts = 10;
                printf("\n>>> Hard mode selected!");
                break;
                
            case 4: // Expert
                max_number = 500;
                max_attempts = 8;
                printf("\n>>> Expert mode selected! Good luck!");
                break;
                
            default:
                printf("Invalid selection! Please choose 0-4.\n");
                continue;
        }
        
        play_single_round(max_number, max_attempts);
        
        printf("\nWould you like to play again? (y/n): ");
        char play_again;
        if (scanf(" %c", &play_again) == 1) {
            clear_input_buffer();
            if (play_again != 'y' && play_again != 'Y') {
                break;
            }
        } else {
            clear_input_buffer();
            break;
        }
    }
    
    printf("\nThanks for playing Guess the Number!\n");
}
