#include "games.h"

#define MAX_WORD_LENGTH 15
#define MAX_SCRAMBLED_LENGTH 20

typedef struct {
    char original_word[MAX_WORD_LENGTH];
    char scrambled_word[MAX_WORD_LENGTH];
    char player_guess[MAX_WORD_LENGTH];
    int attempts;
    int max_attempts;
} WordScrambleGame;

// Word list for scrambling
const char* scramble_words[] = {
    "COMPUTER", "KEYBOARD", "MONITOR", "PROGRAM", "FUNCTION",
    "VARIABLE", "COMPILER", "DEBUGGER", "SOFTWARE", "HARDWARE",
    "INTERNET", "BROWSER", "WEBSITE", "DATABASE", "NETWORK",
    "SECURITY", "PASSWORD", "DOWNLOAD", "UPLOAD", "EXECUTE",
    "ALGORITHM", "STRUCTURE", "POINTER", "MEMORY", "PROCESSOR",
    "GRAPHICS", "MULTIMEDIA", "SPEAKER", "PRINTER", "SCANNER",
    "LAPTOP", "DESKTOP", "MOBILE", "TABLET", "GAMING",
    "CODING", "PYTHON", "JAVASCRIPT", "LANGUAGE", "FRAMEWORK"
};

const int SCRAMBLE_WORD_COUNT = sizeof(scramble_words) / sizeof(scramble_words[0]);

void scramble_word(const char* original, char* scrambled) {
    int len = strlen(original);
    strcpy(scrambled, original);
    
    // Simple scrambling algorithm - swap letters randomly
    for (int i = 0; i < len * 2; i++) {
        int pos1 = rand() % len;
        int pos2 = rand() % len;
        
        // Swap characters
        char temp = scrambled[pos1];
        scrambled[pos1] = scrambled[pos2];
        scrambled[pos2] = temp;
    }
    
    // Make sure the scrambled word is different from original
    int attempts = 0;
    while (strcmp(scrambled, original) == 0 && attempts < 10) {
        int pos1 = rand() % len;
        int pos2 = rand() % len;
        if (pos1 != pos2) {
            char temp = scrambled[pos1];
            scrambled[pos1] = scrambled[pos2];
            scrambled[pos2] = temp;
        }
        attempts++;
    }
}

void display_scramble_rules(void) {
    printf("\n===========================================\n");
    printf("             WORD SCRAMBLE\n");
    printf("===========================================\n");
    printf("How to play:\n");
    printf("* I'll show you a scrambled word\n");
    printf("* Unscramble it to find the original word\n");
    printf("* All words are computer/technology related\n");
    printf("* You have 3 attempts per word\n");
    printf("* Type 'hint' for a clue!\n");
    printf("-------------------------------------------\n");
}

void give_hint(const char* word) {
    int len = strlen(word);
    printf("\nHint: The word has %d letters and starts with '%c'\n", len, word[0]);
    
    // Give category hint based on the word
    if (strstr(word, "COMPUTER") || strstr(word, "LAPTOP") || strstr(word, "DESKTOP")) {
        printf("Category: Hardware device\n");
    } else if (strstr(word, "PROGRAM") || strstr(word, "SOFTWARE") || strstr(word, "FUNCTION")) {
        printf("Category: Software/Programming\n");
    } else if (strstr(word, "INTERNET") || strstr(word, "BROWSER") || strstr(word, "WEBSITE")) {
        printf("Category: Internet/Web\n");
    } else if (strstr(word, "KEYBOARD") || strstr(word, "MONITOR") || strstr(word, "PRINTER")) {
        printf("Category: Computer peripheral\n");
    } else {
        printf("Category: Technology\n");
    }
}

int check_guess(const char* guess, const char* original) {
    // Convert guess to uppercase for comparison
    char upper_guess[MAX_WORD_LENGTH];
    strcpy(upper_guess, guess);
    
    for (int i = 0; upper_guess[i]; i++) {
        upper_guess[i] = toupper(upper_guess[i]);
    }
    
    return strcmp(upper_guess, original) == 0;
}

void play_single_scramble(WordScrambleGame* game) {
    // Select random word
    int word_index = rand() % SCRAMBLE_WORD_COUNT;
    strcpy(game->original_word, scramble_words[word_index]);
    
    // Scramble the word
    scramble_word(game->original_word, game->scrambled_word);
    
    game->attempts = 0;
    game->max_attempts = 3;
    
    printf("\n>>> New Word Scramble! <<<\n");
    printf("Scrambled word: %s\n", game->scrambled_word);
    printf("You have %d attempts to unscramble it.\n", game->max_attempts);
    printf("(Type 'hint' for a clue, 'quit' to return to menu)\n");
    
    while (game->attempts < game->max_attempts) {
        printf("\nAttempt %d/%d - Enter your guess: ", game->attempts + 1, game->max_attempts);
        
        if (scanf("%s", game->player_guess) != 1) {
            printf("Invalid input! Please try again.\n");
            clear_input_buffer();
            continue;
        }
        
        clear_input_buffer();
        
        // Check for special commands
        if (strcmp(game->player_guess, "quit") == 0 || strcmp(game->player_guess, "QUIT") == 0) {
            printf("Returning to main menu...\n");
            return;
        }
        
        if (strcmp(game->player_guess, "hint") == 0 || strcmp(game->player_guess, "HINT") == 0) {
            give_hint(game->original_word);
            continue; // Don't count hint as an attempt
        }
        
        game->attempts++;
        
        if (check_guess(game->player_guess, game->original_word)) {
            printf("\n*** CORRECT! Well done! ***\n");
            printf("The word was: %s\n", game->original_word);
            
            if (game->attempts == 1) {
                printf("*** AMAZING! Got it on the first try! ***\n");
            } else if (game->attempts == 2) {
                printf("*** Great job! Second attempt! ***\n");
            } else {
                printf("*** Good work! You got it! ***\n");
            }
            return;
        } else {
            printf("*** Wrong! Try again. ***\n");
            if (game->attempts < game->max_attempts) {
                printf("The scrambled word is: %s\n", game->scrambled_word);
            }
        }
    }
    
    // If all attempts used
    printf("\n*** Game Over! ***\n");
    printf("The correct word was: %s\n", game->original_word);
    printf("Better luck next time!\n");
}

void play_word_scramble(void) {
    WordScrambleGame game;
    int total_score = 0;
    int games_played = 0;
    
    display_scramble_rules();
    
    while (1) {
        play_single_scramble(&game);
        
        if (game.attempts <= game.max_attempts && 
            check_guess(game.player_guess, game.original_word)) {
            total_score += (game.max_attempts - game.attempts + 1) * 10;
            games_played++;
        } else if (game.attempts < game.max_attempts) {
            // Player quit
            break;
        } else {
            games_played++;
        }
        
        if (games_played > 0) {
            printf("\n*** Current Score: %d points (%d games played) ***\n", 
                   total_score, games_played);
        }
        
        printf("\nPlay another word? (y/n): ");
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
    
    if (games_played > 0) {
        printf("\n===========================================\n");
        printf("            FINAL RESULTS\n");
        printf("===========================================\n");
        printf("Total Score: %d points\n", total_score);
        printf("Games Played: %d\n", games_played);
        printf("Average Score: %.1f points per game\n", 
               (float)total_score / games_played);
        printf("===========================================\n");
    }
    
    printf("\nThanks for playing Word Scramble!\n");
}
