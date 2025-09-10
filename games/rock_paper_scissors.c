#include "games.h"

typedef enum {
    ROCK = 1,
    PAPER,
    SCISSORS
} Choice;

const char* choice_names[] = {"", "Rock", "Paper", "Scissors"};

void display_rps_rules(void) {
    printf("\n===========================================\n");
    printf("           ROCK PAPER SCISSORS\n");
    printf("===========================================\n");
    printf("Rules:\n");
    printf("* Rock crushes Scissors\n");
    printf("* Scissors cuts Paper\n");
    printf("* Paper covers Rock\n");
    printf("-------------------------------------------\n");
}

int get_computer_choice(void) {
    return (rand() % 3) + 1; // Random choice between 1-3
}

int get_player_choice(void) {
    int choice;
    
    printf("\nMake your choice:\n");
    printf("1. Rock\n");
    printf("2. Paper\n");
    printf("3. Scissors\n");
    printf("0. Return to main menu\n");
    printf("\nEnter your choice (0-3): ");
    
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return -1; // Invalid input
    }
    
    clear_input_buffer();
    return choice;
}

int determine_winner(int player, int computer) {
    if (player == computer) {
        return 0; // Tie
    }
    
    if ((player == ROCK && computer == SCISSORS) ||
        (player == PAPER && computer == ROCK) ||
        (player == SCISSORS && computer == PAPER)) {
        return 1; // Player wins
    }
    
    return -1; // Computer wins
}

void display_choices(int player, int computer) {
    printf("\n-------------------------------------------\n");
    printf("You chose:      %s\n", choice_names[player]);
    printf("Computer chose: %s\n", choice_names[computer]);
    printf("-------------------------------------------\n");
}

void play_rock_paper_scissors(void) {
    int player_choice, computer_choice;
    int player_score = 0, computer_score = 0;
    int rounds_played = 0;
    int result;
    
    display_rps_rules();
    
    printf("\nLet's play! (Enter 0 anytime to return to main menu)\n");
    
    while (1) {
        player_choice = get_player_choice();
        
        if (player_choice == 0) {
            break; // Return to main menu
        }
        
        if (player_choice < 1 || player_choice > 3) {
            printf("Invalid choice! Please enter 1, 2, or 3.\n");
            continue;
        }
        
        computer_choice = get_computer_choice();
        display_choices(player_choice, computer_choice);
        
        result = determine_winner(player_choice, computer_choice);
        rounds_played++;
        
        if (result == 1) {
            printf("*** You WIN this round! ***\n");
            player_score++;
        } else if (result == -1) {
            printf("*** Computer WINS this round! ***\n");
            computer_score++;
        } else {
            printf("*** It's a TIE! ***\n");
        }
        
        printf("\n*** Current Score - You: %d | Computer: %d | Rounds: %d ***\n",
               player_score, computer_score, rounds_played);
        
        printf("\nPlay another round? (y/n): ");
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
    
    if (rounds_played > 0) {
        printf("\n===========================================\n");
        printf("            FINAL RESULTS\n");
        printf("===========================================\n");
        printf("Your Score:     %d\n", player_score);
        printf("Computer Score: %d\n", computer_score);
        printf("Rounds Played:  %d\n", rounds_played);
        
        if (player_score > computer_score) {
            printf("\n*** CONGRATULATIONS! You are the CHAMPION! ***\n");
        } else if (computer_score > player_score) {
            printf("\n*** Computer wins overall! Better luck next time! ***\n");
        } else {
            printf("\n*** It's a tie overall! Great game! ***\n");
        }
        printf("===========================================\n");
    }
}
