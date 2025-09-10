#include "games.h"

#define MAX_WORD_LENGTH 20
#define MAX_WRONG_GUESSES 6

typedef struct {
    char word[MAX_WORD_LENGTH];
    char guessed_word[MAX_WORD_LENGTH];
    char guessed_letters[26];
    int wrong_guesses;
    int word_length;
    int letters_guessed_count;
} HangmanGame;

// Word list for the game
const char* word_list[] = {
    "PROGRAMMING", "COMPUTER", "ALGORITHM", "FUNCTION", "VARIABLE",
    "STRUCTURE", "POINTER", "ARRAY", "LIBRARY", "COMPILER",
    "DEBUGGING", "SOFTWARE", "HARDWARE", "NETWORK", "DATABASE",
    "SECURITY", "ENCRYPTION", "PROTOCOL", "INTERFACE", "FRAMEWORK",
    "DEVELOPMENT", "APPLICATION", "SYSTEM", "MEMORY", "PROCESSOR",
    "GRAPHICS", "MULTIMEDIA", "INTERNET", "BROWSER", "SERVER",
    "CLIENT", "MOBILE", "DESKTOP", "LAPTOP", "KEYBOARD",
    "MONITOR", "PRINTER", "SCANNER", "STORAGE", "BACKUP"
};

const int WORD_COUNT = sizeof(word_list) / sizeof(word_list[0]);

void draw_hangman(int wrong_guesses) {
    printf("\n");
    printf("  +-----+\n");
    printf("  |     |\n");
    
    switch (wrong_guesses) {
        case 0:
            printf("  |      \n");
            printf("  |      \n");
            printf("  |      \n");
            printf("  |      \n");
            break;
        case 1:
            printf("  |     O\n");
            printf("  |      \n");
            printf("  |      \n");
            printf("  |      \n");
            break;
        case 2:
            printf("  |     O\n");
            printf("  |     |\n");
            printf("  |      \n");
            printf("  |      \n");
            break;
        case 3:
            printf("  |     O\n");
            printf("  |    -|\n");
            printf("  |      \n");
            printf("  |      \n");
            break;
        case 4:
            printf("  |     O\n");
            printf("  |    -|-\n");
            printf("  |      \n");
            printf("  |      \n");
            break;
        case 5:
            printf("  |     O\n");
            printf("  |    -|-\n");
            printf("  |     |\n");
            printf("  |      \n");
            break;
        case 6:
            printf("  |     X\n");
            printf("  |    -|-\n");
            printf("  |     |\n");
            printf("  |    / \\\n");
            break;
    }
    printf("  |      \n");
    printf("+---+    \n");
    printf("|   |    \n");
    printf("+---+    \n");
}

void init_game(HangmanGame* game) {
    // Select random word
    int word_index = rand() % WORD_COUNT;
    strcpy(game->word, word_list[word_index]);
    game->word_length = strlen(game->word);
    
    // Initialize guessed word with underscores
    for (int i = 0; i < game->word_length; i++) {
        game->guessed_word[i] = '_';
    }
    game->guessed_word[game->word_length] = '\0';
    
    // Initialize other game state
    game->wrong_guesses = 0;
    game->letters_guessed_count = 0;
    
    // Initialize guessed letters array
    for (int i = 0; i < 26; i++) {
        game->guessed_letters[i] = 0;
    }
}

void display_game_state(const HangmanGame* game) {
    draw_hangman(game->wrong_guesses);
    
    printf("\nWord: ");
    for (int i = 0; i < game->word_length; i++) {
        printf("%c ", game->guessed_word[i]);
    }
    printf("\n");
    
    printf("\nWrong guesses left: %d\n", MAX_WRONG_GUESSES - game->wrong_guesses);
    
    if (game->letters_guessed_count > 0) {
        printf("Letters guessed: ");
        for (int i = 0; i < 26; i++) {
            if (game->guessed_letters[i]) {
                printf("%c ", 'A' + i);
            }
        }
        printf("\n");
    }
}

void display_hangman_rules(void) {
    printf("\n===========================================\n");
    printf("               HANGMAN\n");
    printf("===========================================\n");
    printf("How to play:\n");
    printf("* Guess the hidden word one letter at a time\n");
    printf("* You have 6 wrong guesses before you lose\n");
    printf("* All words are related to computers/programming\n");
    printf("* Good luck!\n");
    printf("-------------------------------------------\n");
}

int is_letter_already_guessed(const HangmanGame* game, char letter) {
    return game->guessed_letters[letter - 'A'];
}

int process_guess(HangmanGame* game, char letter) {
    // Convert to uppercase
    letter = toupper(letter);
    
    // Check if already guessed
    if (is_letter_already_guessed(game, letter)) {
        return -1; // Already guessed
    }
    
    // Mark as guessed
    game->guessed_letters[letter - 'A'] = 1;
    game->letters_guessed_count++;
    
    // Check if letter is in word
    int found = 0;
    for (int i = 0; i < game->word_length; i++) {
        if (game->word[i] == letter) {
            game->guessed_word[i] = letter;
            found = 1;
        }
    }
    
    if (!found) {
        game->wrong_guesses++;
        return 0; // Wrong guess
    }
    
    return 1; // Correct guess
}

int is_word_complete(const HangmanGame* game) {
    for (int i = 0; i < game->word_length; i++) {
        if (game->guessed_word[i] == '_') {
            return 0;
        }
    }
    return 1;
}

char get_player_guess(void) {
    char guess;
    printf("\nEnter your guess (a letter): ");
    
    if (scanf(" %c", &guess) != 1) {
        clear_input_buffer();
        return '\0';
    }
    
    clear_input_buffer();
    
    // Check if it's a valid letter
    if (!isalpha(guess)) {
        printf("Please enter a valid letter!\n");
        return '\0';
    }
    
    return guess;
}

void display_game_result(const HangmanGame* game, int won) {
    printf("\n===========================================\n");
    if (won) {
        printf("*** CONGRATULATIONS! YOU WON! ***\n");
        printf("You guessed the word: %s\n", game->word);
        printf("Wrong guesses: %d/%d\n", game->wrong_guesses, MAX_WRONG_GUESSES);
        
        if (game->wrong_guesses == 0) {
            printf("*** PERFECT! No wrong guesses! ***\n");
        } else if (game->wrong_guesses <= 2) {
            printf("*** EXCELLENT! Great guessing! ***\n");
        } else if (game->wrong_guesses <= 4) {
            printf("*** GOOD JOB! Well done! ***\n");
        } else {
            printf("*** PHEW! That was close! ***\n");
        }
    } else {
        printf("*** GAME OVER! YOU LOST! ***\n");
        printf("The word was: %s\n", game->word);
        printf("Better luck next time!\n");
    }
    printf("===========================================\n");
}

void play_hangman(void) {
    HangmanGame game;
    char guess;
    int guess_result;
    
    display_hangman_rules();
    
    while (1) {
        init_game(&game);
        
        printf("\n🎮 New game started!\n");
        printf("Word length: %d letters\n", game.word_length);
        
        while (game.wrong_guesses < MAX_WRONG_GUESSES && !is_word_complete(&game)) {
            display_game_state(&game);
            
            guess = get_player_guess();
            if (guess == '\0') {
                continue; // Invalid input, try again
            }
            
            guess_result = process_guess(&game, guess);
            
            if (guess_result == -1) {
                printf("You already guessed that letter! Try a different one.\n");
            } else if (guess_result == 0) {
                printf("❌ Wrong! '%c' is not in the word.\n", toupper(guess));
            } else {
                printf("✅ Good guess! '%c' is in the word.\n", toupper(guess));
            }
        }
        
        // Display final game state
        display_game_state(&game);
        
        // Display result
        int won = is_word_complete(&game);
        display_game_result(&game, won);
        
        // Ask if player wants to play again
        printf("\nPlay another game? (y/n): ");
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
    
    printf("\nThanks for playing Hangman! 🎯\n");
}
