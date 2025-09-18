#include "games/games.h"

void display_menu(void) {
    printf("\n+==========================================+\n");
    printf("|           CLI GAMES PACK v1.9            |\n");
    printf("|         Welcome to Fun Gaming!           |\n");
    printf("+==========================================+\n");
    printf("|                                          |\n");
    printf("|  1. Rock, Paper, Scissors                |\n");
    printf("|  2. Guess the Number                     |\n");
    printf("|  3. Tic Tac Toe (2 Players)              |\n");
    printf("|  4. Hangman                              |\n");
    printf("|  5. Word Scramble                        |\n");
    printf("|  6. Coin Flip                            |\n");
    printf("|  7. Blackjack (21)                       |\n");
    printf("|  8. Bulls & Cows (Mastermind)            |\n");
    printf("|  9. ASCII Racing Game                    |\n");
    printf("| 10. 2048                                 |\n");
    printf("| 11. Snake                                |\n");
    printf("| 12. Slot Machine                         |\n");
    printf("| 13. Minesweeper                          |\n");
    printf("| 14. F1 Reaction Start                    |\n");
    printf("| 15. Space Invaders                       |\n");
    printf("| 16. Simon Says (Memory)                  |\n");
    printf("| 17. Flappy Bird                          |\n");
    printf("| 18. Chrome Dino Runner                   |\n");
    printf("| 19. Russian Roulette                     |\n");
    printf("| 20. Exit                                 |\n");
    printf("|                                          |\n");
    printf("+==========================================+\n");
    printf("\nPlease enter your choice (1-20): ");
}

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_and_continue(void) {
    printf("\nPress Enter to return to main menu...");
    clear_input_buffer();
    getchar();
}

int main(void) {
    int choice;
    int running = 1;
    
    // Seed random number generator
    srand((unsigned int)time(NULL));
    
    printf("Welcome to CLI Games Pack!\n");
    printf("Developed with <3 in C\n");
    
    while (running) {
        display_menu();
        
        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number between 1-20.\n");
            clear_input_buffer();
            pause_and_continue();
            continue;
        }
        
        clear_input_buffer(); // Clear remaining input
        
        switch (choice) {
            case 1:
                printf("\n>>> Starting Rock, Paper, Scissors...\n");
                play_rock_paper_scissors();
                pause_and_continue();
                break;
                
            case 2:
                printf("\n>>> Starting Guess the Number...\n");
                play_guess_number();
                pause_and_continue();
                break;
                
            case 3:
                printf("\n>>> Starting Tic Tac Toe...\n");
                play_tic_tac_toe();
                pause_and_continue();
                break;
                
            case 4:
                printf("\n>>> Starting Hangman...\n");
                play_hangman();
                pause_and_continue();
                break;
                
            case 5:
                printf("\n>>> Starting Word Scramble...\n");
                play_word_scramble();
                pause_and_continue();
                break;
                
            case 6:
                printf("\n>>> Starting Coin Flip...\n");
                play_coin_flip();
                pause_and_continue();
                break;
                
            case 7:
                printf("\n>>> Starting Blackjack...\n");
                play_blackjack();
                pause_and_continue();
                break;
                
            case 8:
                printf("\n>>> Starting Bulls & Cows (Mastermind)...\n");
                play_bulls_and_cows();
                pause_and_continue();
                break;
                
            case 9:
                printf("\n>>> Starting ASCII Racing Game...\n");
                play_ascii_racing();
                pause_and_continue();
                break;
                
            case 10:
                printf("\n>>> Starting 2048...\n");
                play_2048();
                pause_and_continue();
                break;
                
            case 11:
                printf("\n>>> Starting Snake...\n");
                play_snake();
                pause_and_continue();
                break;
                
            case 12:
                printf("\n>>> Starting Slot Machine...\n");
                play_slot_machine();
                pause_and_continue();
                break;
                
            case 13:
                printf("\n>>> Starting Minesweeper...\n");
                play_minesweeper();
                pause_and_continue();
                break;
                
            case 14:
                printf("\n>>> Starting F1 Reaction Start...\n");
                play_f1_reaction();
                pause_and_continue();
                break;
                
            case 15:
                printf("\n>>> Starting Space Invaders...\n");
                play_space_invaders();
                pause_and_continue();
                break;
                
            case 16:
                printf("\n>>> Starting Simon Says (Memory)...\n");
                play_simon_says();
                pause_and_continue();
                break;
                
            case 17:
                printf("\n>>> Starting Flappy Bird...\n");
                play_flappy_bird();
                pause_and_continue();
                break;
                
            case 18:
                printf("\n>>> Starting Chrome Dino Runner...\n");
                play_dino_runner();
                pause_and_continue();
                break;
                
            case 19:
                printf("\n>>> Starting Russian Roulette...\n");
                play_russian_roulette();
                pause_and_continue();
                break;
                
            case 20:
                printf("\n>>> Thanks for playing! Goodbye!\n");
                running = 0;
                break;
                
            default:
                printf("\nInvalid choice! Please select a number between 1-20.\n");
                pause_and_continue();
                break;
        }
    }
    
    return 0;
}
