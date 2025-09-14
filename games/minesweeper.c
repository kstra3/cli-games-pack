#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
#else
    #include <unistd.h>
    #define CLEAR_SCREEN() system("clear")
    #define Sleep(x) usleep((x) * 1000)
#endif

// Game constants
#define MAX_WIDTH 30
#define MAX_HEIGHT 16
#define MIN_WIDTH 5
#define MIN_HEIGHT 5

// Difficulty presets
typedef enum {
    DIFFICULTY_BEGINNER = 0,
    DIFFICULTY_INTERMEDIATE = 1,
    DIFFICULTY_EXPERT = 2,
    DIFFICULTY_CUSTOM = 3
} Difficulty;

// Cell states
typedef enum {
    CELL_HIDDEN = 0,
    CELL_REVEALED = 1,
    CELL_FLAGGED = 2
} CellState;

// Game structure
typedef struct {
    int width;
    int height;
    int mine_count;
    int revealed_count;
    int flag_count;
    int flags_placed;
    bool game_over;
    bool victory;
    time_t start_time;
    int games_played;
    int games_won;
    int best_time_beginner;
    int best_time_intermediate;
    int best_time_expert;
    
    // Grid data
    bool mines[MAX_HEIGHT][MAX_WIDTH];
    CellState state[MAX_HEIGHT][MAX_WIDTH];
    int numbers[MAX_HEIGHT][MAX_WIDTH];
    bool first_click;
} MinesweeperGame;

// Global game instance
static MinesweeperGame game;

// Function declarations
void init_minesweeper(void);
void setup_difficulty(Difficulty diff);
void generate_mines(int start_row, int start_col);
void calculate_numbers(void);
int count_adjacent_mines(int row, int col);
void display_game(void);
void display_minesweeper_instructions(void);
void display_minesweeper_statistics(void);
char get_cell_display(int row, int col);
bool is_valid_position(int row, int col);
void reveal_cell(int row, int col);
void reveal_adjacent_cells(int row, int col);
void toggle_flag(int row, int col);
bool check_victory(void);
void game_over_sequence(bool won);
void play_game_loop(void);
bool parse_input(char* input, int* row, int* col, char* action);
void minesweeper_clear_input_buffer(void);

// Initialize the minesweeper game
void init_minesweeper(void) {
    memset(&game, 0, sizeof(MinesweeperGame));
    game.first_click = true;
    game.best_time_beginner = -1;
    game.best_time_intermediate = -1;
    game.best_time_expert = -1;
}

// Setup game based on difficulty
void setup_difficulty(Difficulty diff) {
    switch (diff) {
        case DIFFICULTY_BEGINNER:
            game.width = 9;
            game.height = 9;
            game.mine_count = 10;
            break;
        case DIFFICULTY_INTERMEDIATE:
            game.width = 16;
            game.height = 16;
            game.mine_count = 40;
            break;
        case DIFFICULTY_EXPERT:
            game.width = 30;
            game.height = 16;
            game.mine_count = 99;
            break;
        case DIFFICULTY_CUSTOM:
            // Custom settings will be handled separately
            break;
    }
    
    // Initialize game state
    game.revealed_count = 0;
    game.flag_count = game.mine_count;
    game.flags_placed = 0;
    game.game_over = false;
    game.victory = false;
    game.first_click = true;
    
    // Clear grids
    for (int i = 0; i < game.height; i++) {
        for (int j = 0; j < game.width; j++) {
            game.mines[i][j] = false;
            game.state[i][j] = CELL_HIDDEN;
            game.numbers[i][j] = 0;
        }
    }
}

// Generate mines randomly, avoiding the first click position
void generate_mines(int start_row, int start_col) {
    srand(time(NULL));
    int mines_placed = 0;
    
    while (mines_placed < game.mine_count) {
        int row = rand() % game.height;
        int col = rand() % game.width;
        
        // Don't place mine on first click or if already has mine
        if ((row == start_row && col == start_col) || game.mines[row][col]) {
            continue;
        }
        
        game.mines[row][col] = true;
        mines_placed++;
    }
    
    calculate_numbers();
}

// Calculate number of adjacent mines for each cell
void calculate_numbers(void) {
    for (int i = 0; i < game.height; i++) {
        for (int j = 0; j < game.width; j++) {
            if (!game.mines[i][j]) {
                game.numbers[i][j] = count_adjacent_mines(i, j);
            }
        }
    }
}

// Count mines adjacent to a position
int count_adjacent_mines(int row, int col) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (is_valid_position(nr, nc) && game.mines[nr][nc]) {
                count++;
            }
        }
    }
    return count;
}

// Check if position is within grid bounds
bool is_valid_position(int row, int col) {
    return row >= 0 && row < game.height && col >= 0 && col < game.width;
}

// Get display character for a cell
char get_cell_display(int row, int col) {
    if (game.state[row][col] == CELL_FLAGGED) {
        return 'F';
    } else if (game.state[row][col] == CELL_HIDDEN) {
        return '.';
    } else if (game.mines[row][col]) {
        return '*';
    } else if (game.numbers[row][col] == 0) {
        return ' ';
    } else {
        return '0' + game.numbers[row][col];
    }
}

// Display the game board
void display_game(void) {
    CLEAR_SCREEN();
    
    printf("\n+==========================================+\n");
    printf("|            MINESWEEPER v1.0              |\n");
    printf("+==========================================+\n");
    
    // Game statistics
    int elapsed = game.first_click ? 0 : (int)(time(NULL) - game.start_time);
    printf("| Mines: %-3d  Flags: %-3d  Time: %02d:%02d    |\n", 
           game.mine_count, game.flags_placed, elapsed / 60, elapsed % 60);
    printf("| Size: %dx%-2d  Remaining: %-3d            |\n", 
           game.width, game.height, game.flag_count - game.flags_placed);
    printf("+==========================================+\n\n");
    
    // Column headers
    printf("     ");
    for (int j = 0; j < game.width; j++) {
        printf("%c ", 'A' + j);
    }
    printf("\n");
    
    printf("   +");
    for (int j = 0; j < game.width; j++) {
        printf("--");
    }
    printf("+\n");
    
    // Game grid
    for (int i = 0; i < game.height; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < game.width; j++) {
            printf("%c ", get_cell_display(i, j));
        }
        printf("|\n");
    }
    
    printf("   +");
    for (int j = 0; j < game.width; j++) {
        printf("--");
    }
    printf("+\n");
    
    // Game status
    if (game.game_over) {
        if (game.victory) {
            printf("\n*** CONGRATULATIONS! YOU WON! ***\n");
            printf("All mines found in %02d:%02d!\n", elapsed / 60, elapsed % 60);
        } else {
            printf("\n*** GAME OVER! ***\n");
            printf("You hit a mine! Better luck next time.\n");
        }
    } else {
        printf("\nCommands: R A1 (reveal), F A1 (flag), H (help), Q (quit)\n");
        printf("Enter command: ");
    }
}

// Display game instructions
void display_minesweeper_instructions(void) {
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|          MINESWEEPER INSTRUCTIONS       |\n");
    printf("+==========================================+\n");
    printf("|\n");
    printf("| OBJECTIVE:\n");
    printf("| Find all mines without detonating any!\n");
    printf("|\n");
    printf("| HOW TO PLAY:\n");
    printf("| • Numbers show count of adjacent mines\n");
    printf("| • Use logic to deduce mine locations\n");
    printf("| • Flag suspected mines for safety\n");
    printf("| • Reveal all non-mine cells to win\n");
    printf("|\n");
    printf("| COMMANDS:\n");
    printf("| R A1  - Reveal cell at column A, row 1\n");
    printf("| F B3  - Flag/unflag cell at column B, row 3\n");
    printf("| H     - Show this help\n");
    printf("| Q     - Quit game\n");
    printf("|\n");
    printf("| DIFFICULTY LEVELS:\n");
    printf("| Beginner:     9x9,   10 mines\n");
    printf("| Intermediate: 16x16, 40 mines\n");
    printf("| Expert:       30x16, 99 mines\n");
    printf("|\n");
    printf("| TIPS:\n");
    printf("| • First click is always safe\n");
    printf("| • Numbers reveal mine patterns\n");
    printf("| • Flag obvious mines first\n");
    printf("| • Use process of elimination\n");
    printf("|\n");
    printf("+==========================================+\n");
    printf("\nPress Enter to continue...");
    getchar();
}

// Display player statistics
void display_minesweeper_statistics(void) {
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|         MINESWEEPER STATISTICS          |\n");
    printf("+==========================================+\n");
    printf("|\n");
    printf("| Games Played: %-3d\n", game.games_played);
    printf("| Games Won:    %-3d\n", game.games_won);
    if (game.games_played > 0) {
        printf("| Win Rate:     %.1f%%\n", (float)game.games_won / game.games_played * 100);
    } else {
        printf("| Win Rate:     N/A\n");
    }
    printf("|\n");
    printf("| BEST TIMES:\n");
    if (game.best_time_beginner >= 0) {
        printf("| Beginner:     %02d:%02d\n", game.best_time_beginner / 60, game.best_time_beginner % 60);
    } else {
        printf("| Beginner:     --:--\n");
    }
    if (game.best_time_intermediate >= 0) {
        printf("| Intermediate: %02d:%02d\n", game.best_time_intermediate / 60, game.best_time_intermediate % 60);
    } else {
        printf("| Intermediate: --:--\n");
    }
    if (game.best_time_expert >= 0) {
        printf("| Expert:       %02d:%02d\n", game.best_time_expert / 60, game.best_time_expert % 60);
    } else {
        printf("| Expert:       --:--\n");
    }
    printf("|\n");
    printf("+==========================================+\n");
    printf("\nPress Enter to continue...");
    getchar();
}

// Reveal a cell and handle cascading reveals
void reveal_cell(int row, int col) {
    if (!is_valid_position(row, col) || game.state[row][col] != CELL_HIDDEN) {
        return;
    }
    
    // Handle first click
    if (game.first_click) {
        generate_mines(row, col);
        game.start_time = time(NULL);
        game.first_click = false;
    }
    
    game.state[row][col] = CELL_REVEALED;
    game.revealed_count++;
    
    // Check if hit a mine
    if (game.mines[row][col]) {
        game.game_over = true;
        game.victory = false;
        return;
    }
    
    // If cell has no adjacent mines, reveal surrounding cells
    if (game.numbers[row][col] == 0) {
        reveal_adjacent_cells(row, col);
    }
    
    // Check for victory
    if (check_victory()) {
        game_over_sequence(true);
    }
}

// Reveal all adjacent cells (for auto-reveal)
void reveal_adjacent_cells(int row, int col) {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (is_valid_position(nr, nc) && game.state[nr][nc] == CELL_HIDDEN) {
                reveal_cell(nr, nc);
            }
        }
    }
}

// Toggle flag on a cell
void toggle_flag(int row, int col) {
    if (!is_valid_position(row, col) || game.state[row][col] == CELL_REVEALED) {
        return;
    }
    
    if (game.state[row][col] == CELL_FLAGGED) {
        game.state[row][col] = CELL_HIDDEN;
        game.flags_placed--;
    } else if (game.flags_placed < game.mine_count) {
        game.state[row][col] = CELL_FLAGGED;
        game.flags_placed++;
    }
}

// Check if player has won
bool check_victory(void) {
    int non_mine_cells = game.width * game.height - game.mine_count;
    return game.revealed_count == non_mine_cells;
}

// Handle game over sequence
void game_over_sequence(bool won) {
    game.game_over = true;
    game.victory = won;
    game.games_played++;
    
    if (won) {
        game.games_won++;
        int elapsed = (int)(time(NULL) - game.start_time);
        
        // Update best times
        if (game.width == 9 && game.height == 9) { // Beginner
            if (game.best_time_beginner < 0 || elapsed < game.best_time_beginner) {
                game.best_time_beginner = elapsed;
            }
        } else if (game.width == 16 && game.height == 16) { // Intermediate
            if (game.best_time_intermediate < 0 || elapsed < game.best_time_intermediate) {
                game.best_time_intermediate = elapsed;
            }
        } else if (game.width == 30 && game.height == 16) { // Expert
            if (game.best_time_expert < 0 || elapsed < game.best_time_expert) {
                game.best_time_expert = elapsed;
            }
        }
    }
    
    // Reveal all mines
    for (int i = 0; i < game.height; i++) {
        for (int j = 0; j < game.width; j++) {
            if (game.mines[i][j]) {
                game.state[i][j] = CELL_REVEALED;
            }
        }
    }
}

// Parse user input
bool parse_input(char* input, int* row, int* col, char* action) {
    // Remove newline if present
    char* newline = strchr(input, '\n');
    if (newline) *newline = '\0';
    
    // Convert to uppercase
    for (int i = 0; input[i]; i++) {
        input[i] = toupper(input[i]);
    }
    
    // Single character commands
    if (strlen(input) == 1) {
        *action = input[0];
        return *action == 'H' || *action == 'Q' || *action == 'S';
    }
    
    // Two or three character commands (R A1, F B3)
    if (strlen(input) >= 3 && (input[0] == 'R' || input[0] == 'F')) {
        *action = input[0];
        
        // Find column letter and row number
        char col_char = 0;
        int row_num = 0;
        
        // Parse format like "R A1" or "RA1"
        if (input[1] == ' ') {
            if (strlen(input) >= 4) {
                col_char = input[2];
                row_num = atoi(&input[3]);
            }
        } else {
            col_char = input[1];
            row_num = atoi(&input[2]);
        }
        
        // Validate column and row
        if (col_char >= 'A' && col_char < 'A' + game.width && 
            row_num >= 1 && row_num <= game.height) {
            *col = col_char - 'A';
            *row = row_num - 1;
            return true;
        }
    }
    
    return false;
}

// Clear input buffer
void minesweeper_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Main game loop
void play_game_loop(void) {
    char input[20];
    int row, col;
    char action;
    
    while (!game.game_over) {
        display_game();
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        if (parse_input(input, &row, &col, &action)) {
            switch (action) {
                case 'R':
                    reveal_cell(row, col);
                    break;
                case 'F':
                    toggle_flag(row, col);
                    break;
                case 'H':
                    display_minesweeper_instructions();
                    break;
                case 'Q':
                    return;
                case 'S':
                    display_minesweeper_statistics();
                    break;
            }
        } else {
            printf("Invalid command! Press Enter to continue...");
            getchar();
        }
    }
    
    // Show final game state
    display_game();
    printf("\nPress Enter to continue...");
    getchar();
}

// Main minesweeper function
void play_minesweeper(void) {
    init_minesweeper();
    
    while (true) {
        CLEAR_SCREEN();
        printf("\n+==========================================+\n");
        printf("|            MINESWEEPER v1.0              |\n");
        printf("+==========================================+\n");
        printf("|\n");
        printf("| Select Difficulty:                       |\n");
        printf("|\n");
        printf("| 1. Beginner    (9x9,   10 mines)        |\n");
        printf("| 2. Intermediate (16x16, 40 mines)       |\n");
        printf("| 3. Expert      (30x16, 99 mines)        |\n");
        printf("| 4. Custom      (Choose your own)        |\n");
        printf("| 5. Statistics                            |\n");
        printf("| 6. Instructions                          |\n");
        printf("| 7. Return to Main Menu                   |\n");
        printf("|\n");
        printf("+==========================================+\n");
        printf("\nChoice (1-7): ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            minesweeper_clear_input_buffer();
            continue;
        }
        minesweeper_clear_input_buffer();
        
        switch (choice) {
            case 1:
                setup_difficulty(DIFFICULTY_BEGINNER);
                play_game_loop();
                break;
            case 2:
                setup_difficulty(DIFFICULTY_INTERMEDIATE);
                play_game_loop();
                break;
            case 3:
                setup_difficulty(DIFFICULTY_EXPERT);
                play_game_loop();
                break;
            case 4:
                printf("Enter width (5-%d): ", MAX_WIDTH);
                if (scanf("%d", &game.width) != 1 || game.width < MIN_WIDTH || game.width > MAX_WIDTH) {
                    printf("Invalid width!\n");
                    minesweeper_clear_input_buffer();
                    Sleep(1000);
                    continue;
                }
                printf("Enter height (5-%d): ", MAX_HEIGHT);
                if (scanf("%d", &game.height) != 1 || game.height < MIN_HEIGHT || game.height > MAX_HEIGHT) {
                    printf("Invalid height!\n");
                    minesweeper_clear_input_buffer();
                    Sleep(1000);
                    continue;
                }
                int max_mines = (game.width * game.height) / 4;
                printf("Enter mine count (1-%d): ", max_mines);
                if (scanf("%d", &game.mine_count) != 1 || game.mine_count < 1 || game.mine_count > max_mines) {
                    printf("Invalid mine count!\n");
                    minesweeper_clear_input_buffer();
                    Sleep(1000);
                    continue;
                }
                minesweeper_clear_input_buffer();
                
                game.revealed_count = 0;
                game.flag_count = game.mine_count;
                game.flags_placed = 0;
                game.game_over = false;
                game.victory = false;
                game.first_click = true;
                
                // Clear grids
                for (int i = 0; i < game.height; i++) {
                    for (int j = 0; j < game.width; j++) {
                        game.mines[i][j] = false;
                        game.state[i][j] = CELL_HIDDEN;
                        game.numbers[i][j] = 0;
                    }
                }
                
                play_game_loop();
                break;
            case 5:
                display_minesweeper_statistics();
                break;
            case 6:
                display_minesweeper_instructions();
                break;
            case 7:
                return;
            default:
                printf("Invalid choice! Press Enter to continue...");
                getchar();
                break;
        }
    }
}