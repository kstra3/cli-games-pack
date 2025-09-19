#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define BOARD_SIZE 4
#define EMPTY_TILE 0

typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
    int empty_row, empty_col;
    int moves;
} SlidingPuzzle;

// Function prototypes
void display_puzzle_board(SlidingPuzzle *puzzle);
void display_puzzle_menu(void);
void init_solved_board(SlidingPuzzle *puzzle);
void shuffle_board(SlidingPuzzle *puzzle, int difficulty);
int is_solvable(SlidingPuzzle *puzzle);
int move_tile(SlidingPuzzle *puzzle, char direction);
int is_solved(SlidingPuzzle *puzzle);
void play_game(SlidingPuzzle *puzzle);
void show_solution_animation(void);
void show_instructions(void);

void play_sliding_puzzle() {
    int choice;
    SlidingPuzzle puzzle;
    
    printf("\n");
    printf("=====================================\n");
    printf("   WELCOME TO 15-PUZZLE CHALLENGE   \n");
    printf("=====================================\n");
    printf("   Slide tiles to arrange numbers   \n");
    printf("        1-15 in order!              \n");
    printf("=====================================\n\n");

    while (1) {
        display_puzzle_menu();
        printf("Enter your choice (1-5): ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input! Please enter a number.\n\n");
            continue;
        }
        while (getchar() != '\n'); // Clear remaining input

        switch (choice) {
            case 1: // Easy (50 moves)
                init_solved_board(&puzzle);
                shuffle_board(&puzzle, 50);
                printf("\nStarting EASY game (50 shuffles)...\n");
                play_game(&puzzle);
                break;
                
            case 2: // Medium (100 moves)
                init_solved_board(&puzzle);
                shuffle_board(&puzzle, 100);
                printf("\nStarting MEDIUM game (100 shuffles)...\n");
                play_game(&puzzle);
                break;
                
            case 3: // Hard (200 moves)
                init_solved_board(&puzzle);
                shuffle_board(&puzzle, 200);
                printf("\nStarting HARD game (200 shuffles)...\n");
                play_game(&puzzle);
                break;
                
            case 4: // Instructions
                show_instructions();
                break;
                
            case 5: // Solution demo
                show_solution_animation();
                break;
                
            case 6: // Exit
                printf("Thanks for playing 15-Puzzle! Goodbye!\n");
                return;
                
            default:
                printf("Invalid choice! Please select 1-6.\n\n");
        }
    }
}

void display_puzzle_menu(void) {
    printf("+---------------------------------------+\n");
    printf("|         15-PUZZLE MENU              |\n");
    printf("+---------------------------------------+\n");
    printf("|  1. Play Easy   (50 shuffles)      |\n");
    printf("|  2. Play Medium (100 shuffles)     |\n");
    printf("|  3. Play Hard   (200 shuffles)     |\n");
    printf("|  4. How to Play                     |\n");
    printf("|  5. See Solution Demo               |\n");
    printf("|  6. Exit                            |\n");
    printf("+---------------------------------------+\n");
}

void init_solved_board(SlidingPuzzle *puzzle) {
    int num = 1;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (i == BOARD_SIZE - 1 && j == BOARD_SIZE - 1) {
                puzzle->board[i][j] = EMPTY_TILE;
                puzzle->empty_row = i;
                puzzle->empty_col = j;
            } else {
                puzzle->board[i][j] = num++;
            }
        }
    }
    puzzle->moves = 0;
}

void shuffle_board(SlidingPuzzle *puzzle, int difficulty) {
    srand(time(NULL));
    char directions[] = {'w', 'a', 's', 'd'};
    
    for (int i = 0; i < difficulty; i++) {
        char dir = directions[rand() % 4];
        move_tile(puzzle, dir);
    }
    puzzle->moves = 0; // Reset move counter after shuffling
}

int is_solvable(SlidingPuzzle *puzzle) {
    // Convert 2D array to 1D for inversion counting
    int tiles[BOARD_SIZE * BOARD_SIZE - 1];
    int index = 0;
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (puzzle->board[i][j] != EMPTY_TILE) {
                tiles[index++] = puzzle->board[i][j];
            }
        }
    }
    
    // Count inversions
    int inversions = 0;
    for (int i = 0; i < index - 1; i++) {
        for (int j = i + 1; j < index; j++) {
            if (tiles[i] > tiles[j]) {
                inversions++;
            }
        }
    }
    
    // For 4x4 grid: solvable if empty is on even row from bottom and inversions are odd,
    // or empty is on odd row from bottom and inversions are even
    int empty_row_from_bottom = BOARD_SIZE - puzzle->empty_row;
    
    if (empty_row_from_bottom % 2 == 0) {
        return inversions % 2 == 1;
    } else {
        return inversions % 2 == 0;
    }
}

void display_puzzle_board(SlidingPuzzle *puzzle) {
    printf("\n");
    printf("     Moves: %d\n", puzzle->moves);
    printf("   +----+----+----+----+\n");
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("   |");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (puzzle->board[i][j] == EMPTY_TILE) {
                printf("    ");
            } else {
                printf(" %2d ", puzzle->board[i][j]);
            }
            printf("|");
        }
        printf("\n");
        
        if (i < BOARD_SIZE - 1) {
            printf("   +----+----+----+----+\n");
        }
    }
    printf("   +----+----+----+----+\n");
    printf("\nControls: W(up) A(left) S(down) D(right) Q(quit)\n");
}

int move_tile(SlidingPuzzle *puzzle, char direction) {
    int new_row = puzzle->empty_row;
    int new_col = puzzle->empty_col;
    
    switch (tolower(direction)) {
        case 'w': // Move tile down (empty goes up)
            new_row = puzzle->empty_row - 1;
            break;
        case 's': // Move tile up (empty goes down)
            new_row = puzzle->empty_row + 1;
            break;
        case 'a': // Move tile right (empty goes left)
            new_col = puzzle->empty_col - 1;
            break;
        case 'd': // Move tile left (empty goes right)
            new_col = puzzle->empty_col + 1;
            break;
        default:
            return 0; // Invalid move
    }
    
    // Check bounds
    if (new_row < 0 || new_row >= BOARD_SIZE || new_col < 0 || new_col >= BOARD_SIZE) {
        return 0; // Out of bounds
    }
    
    // Swap the tile with empty space
    puzzle->board[puzzle->empty_row][puzzle->empty_col] = puzzle->board[new_row][new_col];
    puzzle->board[new_row][new_col] = EMPTY_TILE;
    
    puzzle->empty_row = new_row;
    puzzle->empty_col = new_col;
    puzzle->moves++;
    
    return 1; // Successful move
}

int is_solved(SlidingPuzzle *puzzle) {
    int expected = 1;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (i == BOARD_SIZE - 1 && j == BOARD_SIZE - 1) {
                if (puzzle->board[i][j] != EMPTY_TILE) {
                    return 0;
                }
            } else {
                if (puzzle->board[i][j] != expected) {
                    return 0;
                }
                expected++;
            }
        }
    }
    return 1;
}

void play_game(SlidingPuzzle *puzzle) {
    char input;
    
    // Ensure puzzle is solvable
    while (!is_solvable(puzzle)) {
        shuffle_board(puzzle, 10);
    }
    
    printf("\nPuzzle generated! Let's solve it!\n");
    
    while (1) {
        display_puzzle_board(puzzle);
        
        if (is_solved(puzzle)) {
            printf("\n");
            printf("🎉 CONGRATULATIONS! 🎉\n");
            printf("You solved the puzzle in %d moves!\n", puzzle->moves);
            printf("Press any key to continue...");
            getchar();
            return;
        }
        
        printf("Your move: ");
        input = getchar();
        while (getchar() != '\n'); // Clear input buffer
        
        if (tolower(input) == 'q') {
            printf("Game quit. Returning to menu...\n");
            return;
        }
        
        if (!move_tile(puzzle, input)) {
            printf("Invalid move! Use W/A/S/D to move tiles.\n");
            printf("Press any key to continue...");
            getchar();
        }
    }
}

void show_instructions(void) {
    printf("\n");
    printf("===============================================\n");
    printf("            HOW TO PLAY 15-PUZZLE            \n");
    printf("===============================================\n");
    printf("\n");
    printf("OBJECTIVE:\n");
    printf("  Arrange the numbered tiles in order from\n");
    printf("  1 to 15, with the empty space at the\n");
    printf("  bottom-right corner.\n");
    printf("\n");
    printf("HOW TO MOVE:\n");
    printf("  • Use W, A, S, D keys to move tiles\n");
    printf("  • W = Move tile DOWN (into empty space)\n");
    printf("  • A = Move tile RIGHT (into empty space)\n");
    printf("  • S = Move tile UP (into empty space)\n");
    printf("  • D = Move tile LEFT (into empty space)\n");
    printf("  • Q = Quit current game\n");
    printf("\n");
    printf("DIFFICULTY LEVELS:\n");
    printf("  • Easy: 50 random shuffles\n");
    printf("  • Medium: 100 random shuffles\n");
    printf("  • Hard: 200 random shuffles\n");
    printf("\n");
    printf("TIPS:\n");
    printf("  • Start by getting the top row correct\n");
    printf("  • Work row by row from top to bottom\n");
    printf("  • The last two rows require special techniques\n");
    printf("  • Not all random arrangements are solvable!\n");
    printf("\n");
    printf("Press any key to return to menu...");
    getchar();
}

void show_solution_animation(void) {
    SlidingPuzzle demo;
    
    printf("\n");
    printf("===============================================\n");
    printf("            SOLUTION DEMONSTRATION            \n");
    printf("===============================================\n");
    printf("\nThis is what a solved 15-puzzle looks like:\n");
    
    init_solved_board(&demo);
    display_puzzle_board(&demo);
    
    printf("\nPress any key to return to menu...");
    getchar();
}