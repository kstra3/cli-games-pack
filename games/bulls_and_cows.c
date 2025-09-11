#include "games.h"

// Bulls and Cows (Mastermind) Game Implementation
// Bulls = correct digit in correct position
// Cows = correct digit in wrong position

void display_bulls_cows_rules(void) {
    printf("\n+========================================+\n");
    printf("|         BULLS AND COWS RULES             |\n");
    printf("+==========================================+\n");
    printf("| I'm thinking of a 4-digit secret number  |\n");
    printf("| with all different digits (no repeats)   |\n");
    printf("|                                          |\n");
    printf("| Your job: Guess the number!              |\n");
    printf("|                                          |\n");
    printf("| BULLS = Correct digit in correct spot    |\n");
    printf("| COWS  = Correct digit in wrong spot      |\n");
    printf("|                                          |\n");
    printf("| Example:                                 |\n");
    printf("| Secret: 1234                             |\n");
    printf("| Guess:  1324 --> 2 Bulls, 2 Cows         |\n");
    printf("|                                          |\n");
    printf("| Enter 0 to return to main menu           |\n");
    printf("+==========================================+\n");
}

int generate_secret_number(int secret[]) {
    int used[10] = {0}; // Track used digits (0-9)
    
    for (int i = 0; i < 4; i++) {
        int digit;
        do {
            digit = rand() % 10;
        } while (used[digit]); // Keep trying until we get an unused digit
        
        secret[i] = digit;
        used[digit] = 1;
    }
    
    return 1; // Success
}

int parse_guess(char* input, int guess[]) {
    // Check if input is exactly 4 characters
    if (strlen(input) != 4) {
        return 0;
    }
    
    // Check if all characters are digits
    for (int i = 0; i < 4; i++) {
        if (!isdigit(input[i])) {
            return 0;
        }
        guess[i] = input[i] - '0';
    }
    
    // Check for duplicate digits
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (guess[i] == guess[j]) {
                return 0; // Duplicate found
            }
        }
    }
    
    return 1; // Valid guess
}

void calculate_bulls_and_cows(int secret[], int guess[], int* bulls, int* cows) {
    *bulls = 0;
    *cows = 0;
    
    // First pass: count bulls (exact matches)
    for (int i = 0; i < 4; i++) {
        if (secret[i] == guess[i]) {
            (*bulls)++;
        }
    }
    
    // Second pass: count cows (right digit, wrong position)
    for (int i = 0; i < 4; i++) {
        if (secret[i] != guess[i]) { // Not a bull
            for (int j = 0; j < 4; j++) {
                if (i != j && secret[i] == guess[j] && secret[j] != guess[j]) {
                    (*cows)++;
                    break; // Found match, don't count again
                }
            }
        }
    }
}

void display_bulls_cows_stats(int attempts, int best_score) {
    printf("\n  +==========================================+\n");
    printf("    |            GAME STATISTICS               |\n");
    printf("    +==========================================+\n");
    printf("    | Attempts this game: %-2d                 |\n", attempts);
    if (best_score > 0) {
        printf("| Your best score: %-2d attempts           |\n", best_score);
    } else {
        printf("| Your best score: Not set yet             |\n");
    }
    printf("    +==========================================+\n");
}

void display_guess_history(int history[][6], int count) {
    if (count == 0) return;
    
    printf("\n+===========================================+\n");
    printf("  |               GUESS HISTORY               |\n");
    printf("  +===========================================+\n");
    printf("  |  Attempt | Guess | Bulls | Cows | Result  |\n");
    printf("  | ---------|-------|-------|------|-------- |\n");
    
    for (int i = 0; i < count; i++) {
        printf("|   %-2d    | %d%d%d%d  |   %d   |  %d   |", 
               i + 1,
               history[i][0], history[i][1], history[i][2], history[i][3],
               history[i][4], history[i][5]);
        
        if (history[i][4] == 4) {
            printf(" WIN!   |\n");
        } else {
            printf("        |\n");
        }
    }
    printf(" +==========================================+\n");
}

void play_bulls_and_cows(void) {
    int secret[4];
    int guess[4];
    int bulls, cows;
    int attempts = 0;
    int max_attempts = 10;
    char input[100];
    static int best_score = 0; // Keep track of best score across games
    int won = 0;
    
    // History tracking (guess + bulls + cows)
    int history[10][6]; // [attempt][4 digits + bulls + cows]
    
    printf("\n+=========================================+\n");
    printf("  |      [BULLS] BULLS AND COWS [COWS]      |\n");
    printf("  +=========================================+\n");
    
    display_bulls_cows_rules();
    
    // Generate secret number
    srand(time(NULL));
    generate_secret_number(secret);
    
    // Debug mode (uncomment for testing)
    // printf("DEBUG - Secret: %d%d%d%d\n", secret[0], secret[1], secret[2], secret[3]);
    
    printf("\nSecret number generated! Start guessing!\n");
    printf("You have %d attempts maximum.\n\n", max_attempts);
    
    while (attempts < max_attempts && !won) {
        printf("Attempt %d/%d - Enter your 4-digit guess (or 0 to quit): ", 
               attempts + 1, max_attempts);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input. Exiting game.\n");
            break;
        }
        
        // Remove newline character
        input[strcspn(input, "\n")] = 0;
        
        // Check for exit
        if (strcmp(input, "0") == 0) {
            printf("\n>>> Returning to main menu. The secret was: %d%d%d%d\n", 
                   secret[0], secret[1], secret[2], secret[3]);
            break;
        }
        
        // Validate and parse guess
        if (!parse_guess(input, guess)) {
            printf("❌ Invalid guess! Please enter exactly 4 different digits.\n");
            continue;
        }
        
        attempts++;
        
        // Calculate bulls and cows
        calculate_bulls_and_cows(secret, guess, &bulls, &cows);
        
        // Store in history
        for (int i = 0; i < 4; i++) {
            history[attempts-1][i] = guess[i];
        }
        history[attempts-1][4] = bulls;
        history[attempts-1][5] = cows;
        
        // Display result
        printf("\nGuess: %d%d%d%d --> ", guess[0], guess[1], guess[2], guess[3]);
        
        if (bulls == 4) {
            printf("*** CONGRATULATIONS! YOU WON! ***\n");
            printf("You cracked the code in %d attempts!\n", attempts);
            
            // Update best score
            if (best_score == 0 || attempts < best_score) {
                best_score = attempts;
                printf("*** NEW PERSONAL BEST! ***\n");
            }
            
            won = 1;
        } else {
            if (bulls > 0 || cows > 0) {
                printf("%d Bull%s, %d Cow%s\n", 
                       bulls, (bulls == 1) ? "" : "s",
                       cows, (cows == 1) ? "" : "s");
            } else {
                printf("No Bulls, No Cows (All wrong!)\n");
            }
            
            // Show encouragement based on progress
            if (bulls > 0) {
                printf("[HINT] You have %d digit%s in the right position!\n", 
                       bulls, (bulls == 1) ? "" : "s");
            }
            if (cows > 0) {
                printf("[CLUE] You have %d correct digit%s in wrong position%s!\n", 
                       cows, (cows == 1) ? "" : "s", (cows == 1) ? "" : "s");
            }
        }
        
        // Show remaining attempts
        if (!won && attempts < max_attempts) {
            printf("Attempts remaining: %d\n", max_attempts - attempts);
        }
        
        printf("\n");
        
        // Show history every 3 attempts or on win/loss
        if ((attempts % 3 == 0) || won || (attempts == max_attempts)) {
            display_guess_history(history, attempts);
        }
    }
    
    // Game over - show final results
    if (!won && attempts >= max_attempts) {
        printf("\n*** Game Over! You've used all %d attempts. ***\n", max_attempts);
        printf("The secret number was: %d%d%d%d\n", 
               secret[0], secret[1], secret[2], secret[3]);
        printf("Better luck next time!\n");
    }
    
    // Show game statistics
    display_bulls_cows_stats(attempts, best_score);
    
    // Ask if they want to play again
    printf("\nWould you like to see the solution strategy? (y/n): ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        if (tolower(input[0]) == 'y') {
            printf("\n+==========================================+\n");
            printf("  |           STRATEGY TIPS                  |\n");
            printf("  +==========================================+\n");
            printf("  | 1. Start with 1234 or 5678               |\n");
            printf("  | 2. Use bulls/cows info to narrow down    |\n");
            printf("  | 3. If 0 bulls/cows, avoid those digits   |\n");
            printf("  | 4. If you get cows, try those digits     |\n");
            printf("  |    in different positions                |\n");
            printf("  | 5. Expert players average 5-6 attempts   |\n");
            printf("  +==========================================+\n");
        }
    }
}
