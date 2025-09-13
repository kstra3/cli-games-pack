#include "games.h"
#include <time.h>
#include <stdlib.h>

// Game constants
#define GRID_SIZE 4
#define WIN_TILE 2048
#define EMPTY_CELL 0

// Game structure
typedef struct {
    int grid[GRID_SIZE][GRID_SIZE];
    int score;
    int moved;
    int game_won;
    int game_over;
} Game2048;

// Function prototypes
void init_2048_game(Game2048* game);
void display_2048_grid(const Game2048* game);
void display_2048_rules(void);
void add_random_tile(Game2048* game);
int move_left(Game2048* game);
int move_right(Game2048* game);
int move_up(Game2048* game);
int move_down(Game2048* game);
int check_game_over(const Game2048* game);
int has_empty_cells(const Game2048* game);
int can_merge(const Game2048* game);

// Main game function
void play_2048(void) {
    Game2048 game;
    char input;
    
    display_2048_rules();
    printf("Press Enter to start...\n");
    getchar();
    
    init_2048_game(&game);
    
    while (!game.game_over) {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
        
        display_2048_grid(&game);
        
        if (game.game_won && !game.game_over) {
            printf("\n*** CONGRATULATIONS! You reached 2048! ***\n");
            printf("Continue playing? (y/n): ");
            input = getchar();
            getchar(); // consume newline
            if (input == 'n' || input == 'N') break;
            game.game_won = 0; // Continue playing
        }
        
        printf("\nUse WASD to move tiles (Q to quit): ");
        input = getchar();
        if (input != '\n') {
            getchar(); // consume newline if needed
        }
        
        game.moved = 0;
        
        switch (input) {
            case 'w': case 'W':
                move_up(&game);
                break;
            case 'a': case 'A':
                move_left(&game);
                break;
            case 's': case 'S':
                move_down(&game);
                break;
            case 'd': case 'D':
                move_right(&game);
                break;
            case 'q': case 'Q':
                printf("\nThanks for playing 2048!\n");
                printf("Press Enter to return to main menu...");
                getchar();
                return;
            default:
                printf("Invalid input! Use WASD keys.\n");
                printf("Press Enter to continue...");
                getchar();
                continue;
        }
        
        if (game.moved) {
            add_random_tile(&game);
            if (check_game_over(&game)) {
                game.game_over = 1;
            }
        }
    }
    
    // Game over screen
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    
    display_2048_grid(&game);
    printf("\n");
    printf("+==========================================+\n");
    printf("|               GAME OVER!                 |\n");
    printf("+==========================================+\n");
    printf("| Final Score: %-27d |\n", game.score);
    if (game.game_won) {
        printf("| Status: YOU WON! [TROPHY]                |\n");
    } else {
        printf("| Status: No more moves possible           |\n");
    }
    printf("| Thanks for playing 2048!                 |\n");
    printf("+==========================================+\n");
    
    printf("\nPress Enter to return to main menu...");
    getchar();
}

// Initialize the game
void init_2048_game(Game2048* game) {
    // Clear the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            game->grid[i][j] = EMPTY_CELL;
        }
    }
    
    game->score = 0;
    game->moved = 0;
    game->game_won = 0;
    game->game_over = 0;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Add two initial tiles
    add_random_tile(game);
    add_random_tile(game);
}

// Display the game rules
void display_2048_rules(void) {
    printf("\n+==========================================+\n");
    printf("|                2048 GAME                 |\n");
    printf("+==========================================+\n");
    printf("| HOW TO PLAY:                             |\n");
    printf("| * Use WASD keys to move tiles            |\n");
    printf("| * When two tiles with same number touch, |\n");
    printf("|   they merge into one!                   |\n");
    printf("| * Goal: Create a tile with 2048          |\n");
    printf("| * Game ends when no moves are possible   |\n");
    printf("|                                          |\n");
    printf("| CONTROLS:                                |\n");
    printf("| W = Move Up    S = Move Down             |\n");
    printf("| A = Move Left  D = Move Right            |\n");
    printf("| Q = Quit Game                            |\n");
    printf("+==========================================+\n\n");
}

// Display the current grid
void display_2048_grid(const Game2048* game) {
    printf("\n+==========================================+\n");
    printf("|               2048 GAME                  |\n");
    printf("+==========================================+\n");
    printf("| Score: %-30d |\n", game->score);
    printf("+==========================================+\n");
    
    printf("\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("+------+------+------+------+\n");
        printf("|");
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == EMPTY_CELL) {
                printf("      |");
            } else {
                printf(" %4d |", game->grid[i][j]);
            }
        }
        printf("\n");
    }
    printf("+------+------+------+------+\n");
}

// Add a random tile (2 or 4) to an empty cell
void add_random_tile(Game2048* game) {
    int empty_cells[GRID_SIZE * GRID_SIZE][2];
    int empty_count = 0;
    
    // Find all empty cells
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == EMPTY_CELL) {
                empty_cells[empty_count][0] = i;
                empty_cells[empty_count][1] = j;
                empty_count++;
            }
        }
    }
    
    if (empty_count > 0) {
        int random_index = rand() % empty_count;
        int row = empty_cells[random_index][0];
        int col = empty_cells[random_index][1];
        
        // 90% chance for 2, 10% chance for 4
        game->grid[row][col] = (rand() % 10 == 0) ? 4 : 2;
    }
}

// Move tiles left
int move_left(Game2048* game) {
    int moved = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        int temp[GRID_SIZE] = {0};
        int index = 0;
        
        // Move non-zero tiles to the left
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] != EMPTY_CELL) {
                temp[index++] = game->grid[i][j];
            }
        }
        
        // Merge adjacent equal tiles
        for (int j = 0; j < index - 1; j++) {
            if (temp[j] == temp[j + 1]) {
                temp[j] *= 2;
                game->score += temp[j];
                if (temp[j] == WIN_TILE) {
                    game->game_won = 1;
                }
                // Shift remaining tiles
                for (int k = j + 1; k < index - 1; k++) {
                    temp[k] = temp[k + 1];
                }
                temp[index - 1] = EMPTY_CELL;
                index--;
            }
        }
        
        // Check if row changed
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] != temp[j]) {
                moved = 1;
            }
            game->grid[i][j] = temp[j];
        }
    }
    
    game->moved = moved;
    return moved;
}

// Move tiles right
int move_right(Game2048* game) {
    int moved = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        int temp[GRID_SIZE] = {0};
        int index = GRID_SIZE - 1;
        
        // Move non-zero tiles to the right
        for (int j = GRID_SIZE - 1; j >= 0; j--) {
            if (game->grid[i][j] != EMPTY_CELL) {
                temp[index--] = game->grid[i][j];
            }
        }
        
        // Merge adjacent equal tiles (from right)
        for (int j = GRID_SIZE - 1; j > index + 1; j--) {
            if (temp[j] == temp[j - 1]) {
                temp[j] *= 2;
                game->score += temp[j];
                if (temp[j] == WIN_TILE) {
                    game->game_won = 1;
                }
                // Shift remaining tiles
                for (int k = j - 1; k > index + 1; k--) {
                    temp[k] = temp[k - 1];
                }
                temp[index + 1] = EMPTY_CELL;
                index++;
            }
        }
        
        // Check if row changed
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] != temp[j]) {
                moved = 1;
            }
            game->grid[i][j] = temp[j];
        }
    }
    
    game->moved = moved;
    return moved;
}

// Move tiles up
int move_up(Game2048* game) {
    int moved = 0;
    
    for (int j = 0; j < GRID_SIZE; j++) {
        int temp[GRID_SIZE] = {0};
        int index = 0;
        
        // Move non-zero tiles up
        for (int i = 0; i < GRID_SIZE; i++) {
            if (game->grid[i][j] != EMPTY_CELL) {
                temp[index++] = game->grid[i][j];
            }
        }
        
        // Merge adjacent equal tiles
        for (int i = 0; i < index - 1; i++) {
            if (temp[i] == temp[i + 1]) {
                temp[i] *= 2;
                game->score += temp[i];
                if (temp[i] == WIN_TILE) {
                    game->game_won = 1;
                }
                // Shift remaining tiles
                for (int k = i + 1; k < index - 1; k++) {
                    temp[k] = temp[k + 1];
                }
                temp[index - 1] = EMPTY_CELL;
                index--;
            }
        }
        
        // Check if column changed
        for (int i = 0; i < GRID_SIZE; i++) {
            if (game->grid[i][j] != temp[i]) {
                moved = 1;
            }
            game->grid[i][j] = temp[i];
        }
    }
    
    game->moved = moved;
    return moved;
}

// Move tiles down
int move_down(Game2048* game) {
    int moved = 0;
    
    for (int j = 0; j < GRID_SIZE; j++) {
        int temp[GRID_SIZE] = {0};
        int index = GRID_SIZE - 1;
        
        // Move non-zero tiles down
        for (int i = GRID_SIZE - 1; i >= 0; i--) {
            if (game->grid[i][j] != EMPTY_CELL) {
                temp[index--] = game->grid[i][j];
            }
        }
        
        // Merge adjacent equal tiles (from bottom)
        for (int i = GRID_SIZE - 1; i > index + 1; i--) {
            if (temp[i] == temp[i - 1]) {
                temp[i] *= 2;
                game->score += temp[i];
                if (temp[i] == WIN_TILE) {
                    game->game_won = 1;
                }
                // Shift remaining tiles
                for (int k = i - 1; k > index + 1; k--) {
                    temp[k] = temp[k - 1];
                }
                temp[index + 1] = EMPTY_CELL;
                index++;
            }
        }
        
        // Check if column changed
        for (int i = 0; i < GRID_SIZE; i++) {
            if (game->grid[i][j] != temp[i]) {
                moved = 1;
            }
            game->grid[i][j] = temp[i];
        }
    }
    
    game->moved = moved;
    return moved;
}

// Check if game is over
int check_game_over(const Game2048* game) {
    // If there are empty cells, game is not over
    if (has_empty_cells(game)) {
        return 0;
    }
    
    // If tiles can still be merged, game is not over
    if (can_merge(game)) {
        return 0;
    }
    
    return 1; // Game over
}

// Check if there are empty cells
int has_empty_cells(const Game2048* game) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == EMPTY_CELL) {
                return 1;
            }
        }
    }
    return 0;
}

// Check if any tiles can be merged
int can_merge(const Game2048* game) {
    // Check horizontal merges
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            if (game->grid[i][j] == game->grid[i][j + 1]) {
                return 1;
            }
        }
    }
    
    // Check vertical merges
    for (int i = 0; i < GRID_SIZE - 1; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == game->grid[i + 1][j]) {
                return 1;
            }
        }
    }
    
    return 0;
}