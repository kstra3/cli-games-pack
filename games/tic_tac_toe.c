#include "games.h"

#define BOARD_SIZE 3
#define EMPTY ' '
#define PLAYER_X 'X'
#define PLAYER_O 'O'

typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];
    char current_player;
    int moves_made;
} TicTacToeGame;

void init_board(TicTacToeGame* game) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            game->board[i][j] = EMPTY;
        }
    }
    game->current_player = PLAYER_X;
    game->moves_made = 0;
}

void display_board(const TicTacToeGame* game) {
    printf("\n");
    printf("     1   2   3\n");
    printf("   +---+---+---+\n");
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf(" %d | %c | %c | %c |\n", i + 1, 
               game->board[i][0], game->board[i][1], game->board[i][2]);
        if (i < BOARD_SIZE - 1) {
            printf("   +---+---+---+\n");
        }
    }
    printf("   +---+---+---+\n");
}

void display_instructions(void) {
    printf("\n===========================================\n");
    printf("              TIC TAC TOE\n");
    printf("===========================================\n");
    printf("How to play:\n");
    printf("* Player 1 is X, Player 2 is O\n");
    printf("* Take turns placing your mark\n");
    printf("* Get 3 in a row (horizontal, vertical, or diagonal) to win!\n");
    printf("* Enter row and column (1-3) to make your move\n");
    printf("-------------------------------------------\n");
}

int check_winner(const TicTacToeGame* game) {
    // Check rows
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (game->board[i][0] != EMPTY &&
            game->board[i][0] == game->board[i][1] &&
            game->board[i][1] == game->board[i][2]) {
            return game->board[i][0];
        }
    }
    
    // Check columns
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (game->board[0][j] != EMPTY &&
            game->board[0][j] == game->board[1][j] &&
            game->board[1][j] == game->board[2][j]) {
            return game->board[0][j];
        }
    }
    
    // Check diagonals
    if (game->board[0][0] != EMPTY &&
        game->board[0][0] == game->board[1][1] &&
        game->board[1][1] == game->board[2][2]) {
        return game->board[0][0];
    }
    
    if (game->board[0][2] != EMPTY &&
        game->board[0][2] == game->board[1][1] &&
        game->board[1][1] == game->board[2][0]) {
        return game->board[0][2];
    }
    
    return EMPTY; // No winner yet
}

int is_board_full(const TicTacToeGame* game) {
    return game->moves_made == BOARD_SIZE * BOARD_SIZE;
}

int make_move(TicTacToeGame* game, int row, int col) {
    // Convert to 0-based indexing
    row--;
    col--;
    
    // Check bounds
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return 0; // Invalid move
    }
    
    // Check if position is empty
    if (game->board[row][col] != EMPTY) {
        return 0; // Position already taken
    }
    
    // Make the move
    game->board[row][col] = game->current_player;
    game->moves_made++;
    
    return 1; // Valid move made
}

void switch_player(TicTacToeGame* game) {
    game->current_player = (game->current_player == PLAYER_X) ? PLAYER_O : PLAYER_X;
}

int get_player_move(TicTacToeGame* game, int* row, int* col) {
    char player_name = game->current_player;
    
    printf("\nPlayer %c's turn!\n", player_name);
    printf("Enter row (1-3) and column (1-3), separated by space: ");
    
    if (scanf("%d %d", row, col) != 2) {
        printf("Invalid input! Please enter two numbers separated by space.\n");
        clear_input_buffer();
        return 0;
    }
    
    clear_input_buffer();
    
    if (*row < 1 || *row > 3 || *col < 1 || *col > 3) {
        printf("Invalid position! Please enter numbers between 1 and 3.\n");
        return 0;
    }
    
    return 1;
}

void display_winner(char winner) {
    printf("\n*** GAME OVER! ***\n");
    if (winner == PLAYER_X) {
        printf("*** Player X (1) WINS! ***\n");
    } else if (winner == PLAYER_O) {
        printf("*** Player O (2) WINS! ***\n");
    } else {
        printf("*** It's a TIE! Well played both players! ***\n");
    }
    printf("-------------------------------------------\n");
}

void play_tic_tac_toe(void) {
    TicTacToeGame game;
    char winner;
    int row, col;
    int valid_move;
    
    display_instructions();
    
    while (1) {
        init_board(&game);
        
        printf("\n*** New Game Started!\n");
        printf("Player 1: X\n");
        printf("Player 2: O\n");
        
        while (1) {
            display_board(&game);
            
            // Get player move
            do {
                valid_move = get_player_move(&game, &row, &col);
                if (valid_move) {
                    if (!make_move(&game, row, col)) {
                        printf("That position is already taken! Try again.\n");
                        valid_move = 0;
                    }
                }
            } while (!valid_move);
            
            // Check for winner
            winner = check_winner(&game);
            if (winner != EMPTY) {
                display_board(&game);
                display_winner(winner);
                break;
            }
            
            // Check for tie
            if (is_board_full(&game)) {
                display_board(&game);
                display_winner(EMPTY); // Tie game
                break;
            }
            
            // Switch to other player
            switch_player(&game);
        }
        
        // Ask if players want to play again
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
    
    printf("\nThanks for playing Tic Tac Toe! ***\n");
}
