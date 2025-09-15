#include "games.h"
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define SLEEP(ms) Sleep(ms)
    #define CLEAR_SCREEN() system("cls")
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define SLEEP(ms) usleep(ms * 1000)
    #define CLEAR_SCREEN() system("clear")
#endif

// Game constants
#define GRID_WIDTH 25
#define GRID_HEIGHT 20
#define MAX_SNAKE_LENGTH 400
#define INITIAL_SPEED 200
#define SPEED_INCREASE 15
#define INITIAL_LENGTH 3

// Direction constants
#define DIR_UP 1
#define DIR_DOWN 2
#define DIR_LEFT 3
#define DIR_RIGHT 4

// Food types
#define FOOD_NORMAL 1
#define FOOD_SPECIAL 2
#define FOOD_POWERUP 3

// Game structures
typedef struct {
    int x, y;
} Position;

typedef struct {
    Position segments[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int next_direction;  // For smooth direction changes
} Snake;

typedef struct {
    Position pos;
    int type;
    int value;
    int active;
} Food;

// Global game variables
static Snake snake;
static Food food;
static int game_speed;
static int score;
static int high_score;
static int food_eaten;
static int game_running;
static int speed_level;

// Function to check if a key is pressed (cross-platform)
#ifdef _WIN32
int snake_kbhit(void) {
    return _kbhit();
}

int snake_get_key(void) {
    return _getch();
}
#else
int snake_kbhit(void) {
    struct termios oldt, newt;
    int oldf;
    int ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    return (ch != EOF) ? ch : 0;
}

int snake_get_key(void) {
    return getchar();
}
#endif

// Function to move cursor to specific position
void snake_goto_xy(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
#endif
}

// Function to hide cursor
void snake_hide_cursor(void) {
#ifdef _WIN32
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
    printf("\033[?25l");
#endif
}

// Function to show cursor
void snake_show_cursor(void) {
#ifdef _WIN32
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
    printf("\033[?25h");
#endif
}

// Initialize game state
void init_snake_game(void) {
    // Initialize snake in the center
    snake.length = INITIAL_LENGTH;
    snake.direction = DIR_RIGHT;
    snake.next_direction = DIR_RIGHT;
    
    int start_x = GRID_WIDTH / 2;
    int start_y = GRID_HEIGHT / 2;
    
    for (int i = 0; i < snake.length; i++) {
        snake.segments[i].x = start_x - i;
        snake.segments[i].y = start_y;
    }
    
    // Initialize game variables
    game_speed = INITIAL_SPEED;
    score = 0;
    food_eaten = 0;
    game_running = 1;
    speed_level = 1;
    
    // Initialize food
    food.active = 0;
    
    srand(time(NULL));
    
    // Load high score (simplified - could be saved to file)
    high_score = 500; // Default high score
}

// Check if position is occupied by snake
int is_snake_position(int x, int y) {
    for (int i = 0; i < snake.length; i++) {
        if (snake.segments[i].x == x && snake.segments[i].y == y) {
            return 1;
        }
    }
    return 0;
}

// Spawn new food randomly
void spawn_food(void) {
    if (food.active) return;
    
    int attempts = 0;
    do {
        food.pos.x = rand() % GRID_WIDTH;
        food.pos.y = rand() % GRID_HEIGHT;
        attempts++;
    } while (is_snake_position(food.pos.x, food.pos.y) && attempts < 100);
    
    // Determine food type (90% normal, 8% special, 2% powerup)
    int food_type_rand = rand() % 100;
    if (food_type_rand < 90) {
        food.type = FOOD_NORMAL;
        food.value = 10;
    } else if (food_type_rand < 98) {
        food.type = FOOD_SPECIAL;
        food.value = 50;
    } else {
        food.type = FOOD_POWERUP;
        food.value = 100;
    }
    
    food.active = 1;
}

// Draw the game grid
void draw_snake_grid(void) {
    CLEAR_SCREEN();
    
    // Draw top border
    printf("+");
    for (int i = 0; i < GRID_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Draw grid with borders
    for (int y = 0; y < GRID_HEIGHT; y++) {
        printf("|");
        
        for (int x = 0; x < GRID_WIDTH; x++) {
            char cell = ' ';
            
            // Check if this is snake head
            if (snake.segments[0].x == x && snake.segments[0].y == y) {
                cell = '@';
            }
            // Check if this is snake body
            else {
                for (int i = 1; i < snake.length; i++) {
                    if (snake.segments[i].x == x && snake.segments[i].y == y) {
                        cell = '#';
                        break;
                    }
                }
            }
            
            // Check if this is food
            if (food.active && food.pos.x == x && food.pos.y == y) {
                if (food.type == FOOD_NORMAL) cell = '*';
                else if (food.type == FOOD_SPECIAL) cell = '$';
                else if (food.type == FOOD_POWERUP) cell = '!';
            }
            
            printf("%c", cell);
        }
        
        printf("|\n");
    }
    
    // Draw bottom border
    printf("+");
    for (int i = 0; i < GRID_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Display game info
    printf("\nSNAKE GAME\n");
    printf("Score: %d | High Score: %d\n", score, high_score);
    printf("Length: %d | Food Eaten: %d\n", snake.length, food_eaten);
    printf("Speed Level: %d\n", speed_level);
    printf("\nControls: WASD to move, Q to quit\n");
    printf("Eat food (*$!) to grow and score points!\n");
}

// Handle player input
void handle_snake_input(void) {
    if (snake_kbhit()) {
        int key = snake_get_key();
        
        switch (key) {
            case 'w':
            case 'W':
                if (snake.direction != DIR_DOWN) {
                    snake.next_direction = DIR_UP;
                }
                break;
            case 's':
            case 'S':
                if (snake.direction != DIR_UP) {
                    snake.next_direction = DIR_DOWN;
                }
                break;
            case 'a':
            case 'A':
                if (snake.direction != DIR_RIGHT) {
                    snake.next_direction = DIR_LEFT;
                }
                break;
            case 'd':
            case 'D':
                if (snake.direction != DIR_LEFT) {
                    snake.next_direction = DIR_RIGHT;
                }
                break;
            case 'q':
            case 'Q':
                game_running = 0;
                break;
#ifdef _WIN32
            case 224:  // Arrow key prefix on Windows
                key = snake_get_key();
                switch (key) {
                    case 72:  // Up arrow
                        if (snake.direction != DIR_DOWN) {
                            snake.next_direction = DIR_UP;
                        }
                        break;
                    case 80:  // Down arrow
                        if (snake.direction != DIR_UP) {
                            snake.next_direction = DIR_DOWN;
                        }
                        break;
                    case 75:  // Left arrow
                        if (snake.direction != DIR_RIGHT) {
                            snake.next_direction = DIR_LEFT;
                        }
                        break;
                    case 77:  // Right arrow
                        if (snake.direction != DIR_LEFT) {
                            snake.next_direction = DIR_RIGHT;
                        }
                        break;
                }
                break;
#else
            case 27:  // ESC sequence for arrow keys on Unix
                if (snake_get_key() == 91) {
                    key = snake_get_key();
                    switch (key) {
                        case 65:  // Up arrow
                            if (snake.direction != DIR_DOWN) {
                                snake.next_direction = DIR_UP;
                            }
                            break;
                        case 66:  // Down arrow
                            if (snake.direction != DIR_UP) {
                                snake.next_direction = DIR_DOWN;
                            }
                            break;
                        case 68:  // Left arrow
                            if (snake.direction != DIR_RIGHT) {
                                snake.next_direction = DIR_LEFT;
                            }
                            break;
                        case 67:  // Right arrow
                            if (snake.direction != DIR_LEFT) {
                                snake.next_direction = DIR_RIGHT;
                            }
                            break;
                    }
                }
                break;
#endif
        }
    }
}

// Move snake based on current direction
void move_snake(void) {
    // Update direction (allows for smooth direction changes)
    snake.direction = snake.next_direction;
    
    // Calculate new head position
    Position new_head = snake.segments[0];
    
    switch (snake.direction) {
        case DIR_UP:
            new_head.y--;
            break;
        case DIR_DOWN:
            new_head.y++;
            break;
        case DIR_LEFT:
            new_head.x--;
            break;
        case DIR_RIGHT:
            new_head.x++;
            break;
    }
    
    // Check wall collision
    if (new_head.x < 0 || new_head.x >= GRID_WIDTH || 
        new_head.y < 0 || new_head.y >= GRID_HEIGHT) {
        game_running = 0;
        return;
    }
    
    // Check self collision
    for (int i = 0; i < snake.length; i++) {
        if (snake.segments[i].x == new_head.x && snake.segments[i].y == new_head.y) {
            game_running = 0;
            return;
        }
    }
    
    // Move body segments
    for (int i = snake.length - 1; i > 0; i--) {
        snake.segments[i] = snake.segments[i - 1];
    }
    
    // Set new head position
    snake.segments[0] = new_head;
    
    // Check food collision
    if (food.active && new_head.x == food.pos.x && new_head.y == food.pos.y) {
        // Grow snake
        snake.length++;
        
        // Add score
        score += food.value;
        
        // Apply length multiplier for higher scores
        if (snake.length > 10) {
            score += (snake.length - 10) * 2;
        }
        
        food_eaten++;
        food.active = 0;
        
        // Update high score
        if (score > high_score) {
            high_score = score;
        }
    }
}

// Increase game difficulty over time
void increase_snake_difficulty(void) {
    int new_speed_level = (score / 50) + 1;
    
    if (new_speed_level > speed_level && game_speed > 50) {
        speed_level = new_speed_level;
        game_speed -= SPEED_INCREASE;
        if (game_speed < 50) game_speed = 50;  // Minimum speed limit
    }
}

// Display game over screen
void display_snake_game_over(void) {
    CLEAR_SCREEN();
    
    printf("\n");
    printf("+===========================================+\n");
    printf("|              GAME OVER!                   |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    
    // Determine cause of death
    Position head = snake.segments[0];
    if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) {
        printf("| *** Your snake crashed into the wall! *** |\n");
    } else {
        printf("| *** Your snake bit itself! Game over! *** |\n");
    }
    
    printf("|                                           |\n");
    printf("| Final Score: %-4d                         |\n", score);
    printf("| Snake Length: %-3d                         |\n", snake.length);
    printf("| Food Eaten: %-3d                           |\n", food_eaten);
    printf("| Speed Level: %-2d                           |\n", speed_level);
    printf("|                                           |\n");
    
    // Performance evaluation
    if (score >= 1000) {
        printf("| Performance: LEGENDARY! [CROWN]           |\n");
    } else if (score >= 500) {
        printf("| Performance: EXCELLENT! [TROPHY]          |\n");
    } else if (score >= 300) {
        printf("| Performance: GREAT! [SILVER]              |\n");
    } else if (score >= 150) {
        printf("| Performance: GOOD! [BRONZE]               |\n");
    } else {
        printf("| Performance: Keep practicing! [TRAIN]     |\n");
    }
    
    printf("|                                           |\n");
    
    // New high score message
    if (score == high_score && score > 500) {
        printf("| *** NEW HIGH SCORE ACHIEVED! ***         |\n");
        printf("|                                           |\n");
    }
    
    printf("| Thanks for playing Snake Game!            |\n");
    printf("+===========================================+\n");
}

// Display game rules and instructions
void display_snake_rules(void) {
    printf("\n+==========================================+\n");
    printf("|             SNAKE GAME RULES              |\n");
    printf("+==========================================+\n");
    printf("| [@] Control your snake head with WASD    |\n");
    printf("| [#] Your snake body grows when eating    |\n");
    printf("| [*] Normal food (+10 points)             |\n");
    printf("| [$] Special food (+50 points)            |\n");
    printf("| [!] Power food (+100 points)             |\n");
    printf("| [WASD] Use WASD or arrow keys to move    |\n");
    printf("| [GROW] Eat food to grow and score points |\n");
    printf("| [WALL] Don't hit walls or yourself!      |\n");
    printf("| [SPD] Speed increases as score grows     |\n");
    printf("| [Q] Press Q anytime to quit              |\n");
    printf("|                                          |\n");
    printf("| Goal: Grow as long as possible and       |\n");
    printf("| achieve the highest score you can!       |\n");
    printf("+==========================================+\n");
    printf("\nPress any key to start slithering...");
    getchar();
}

// Main game function
void play_snake(void) {
    printf("\n+==========================================+\n");
    printf("|        [SNAKE] SNAKE GAME [SLITHER]      |\n");
    printf("+==========================================+\n");
    
    display_snake_rules();
    
    // Initialize game
    init_snake_game();
    snake_hide_cursor();
    
    // Main game loop
    while (game_running) {
        // Spawn food if needed
        spawn_food();
        
        // Handle input
        handle_snake_input();
        
        // Move snake
        move_snake();
        
        // Check difficulty increase
        increase_snake_difficulty();
        
        // Draw game
        draw_snake_grid();
        
        // Game speed delay
        SLEEP(game_speed);
    }
    
    // Show cursor again and display game over
    snake_show_cursor();
    display_snake_game_over();
}