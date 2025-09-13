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
#define TRACK_WIDTH 20
#define TRACK_HEIGHT 15
#define MAX_OBSTACLES 5
#define INITIAL_SPEED 300
#define SPEED_INCREASE 10

// Game structures
typedef struct {
    int x, y;
    int active;
} Obstacle;

typedef struct {
    int x;
    int y;
} Car;

// Global game variables
static Car player_car;
static Obstacle obstacles[MAX_OBSTACLES];
static int game_speed;
static int score;
static int game_running;

// Function to check if a key is pressed (cross-platform)
#ifdef _WIN32
int kbhit(void) {
    return _kbhit();
}

int get_key(void) {
    return _getch();
}
#else
int kbhit(void) {
    int ch = getchar();
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int get_key(void) {
    return getchar();
}
#endif

// Function to move cursor to specific position
void goto_xy(int x, int y) {
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
void hide_cursor(void) {
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
void show_cursor(void) {
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
void init_racing_game(void) {
    player_car.x = TRACK_WIDTH / 2;
    player_car.y = TRACK_HEIGHT - 2;
    game_speed = INITIAL_SPEED;
    score = 0;
    game_running = 1;
    
    // Initialize all obstacles as inactive
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = 0;
        obstacles[i].x = 0;
        obstacles[i].y = 0;
    }
    
    srand(time(NULL));
}

// Draw the game track with borders
void draw_track(void) {
    CLEAR_SCREEN();
    
    // Draw top border
    printf("+");
    for (int i = 0; i < TRACK_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Draw track with side borders
    for (int y = 0; y < TRACK_HEIGHT; y++) {
        printf("|");
        
        for (int x = 0; x < TRACK_WIDTH; x++) {
            int is_car = (x == player_car.x && y == player_car.y);
            int is_obstacle = 0;
            
            // Check if there's an obstacle at this position
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (obstacles[i].active && obstacles[i].x == x && obstacles[i].y == y) {
                    is_obstacle = 1;
                    break;
                }
            }
            
            if (is_car) {
                printf("A");  // Player car
            } else if (is_obstacle) {
                printf("X");  // Obstacle
            } else {
                printf(" ");  // Empty space
            }
        }
        
        printf("|\n");
    }
    
    // Draw bottom border
    printf("+");
    for (int i = 0; i < TRACK_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Display game info
    printf("\nASCII RACING GAME\n");
    printf("Score: %d\n", score);
    printf("Speed Level: %d\n", (INITIAL_SPEED - game_speed) / SPEED_INCREASE + 1);
    printf("\nControls: A/D or Left/Right arrows to move, Q to quit\n");
    printf("Avoid the obstacles (X) and survive as long as possible!\n");
}

// Spawn new obstacles randomly
void spawn_obstacles(void) {
    // Only spawn new obstacle with a certain probability
    if (rand() % 100 < 30) {  // 30% chance to spawn
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) {
                obstacles[i].active = 1;
                obstacles[i].x = rand() % TRACK_WIDTH;
                obstacles[i].y = 0;
                break;  // Only spawn one obstacle at a time
            }
        }
    }
}

// Update obstacle positions (move them down)
void update_obstacles(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            obstacles[i].y++;
            
            // Remove obstacle if it reaches the bottom
            if (obstacles[i].y >= TRACK_HEIGHT) {
                obstacles[i].active = 0;
                score += 10;  // Player gets points for avoiding obstacles
            }
        }
    }
}

// Check for collision between car and obstacles
int check_collision(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active && 
            obstacles[i].x == player_car.x && 
            obstacles[i].y == player_car.y) {
            return 1;  // Collision detected
        }
    }
    return 0;  // No collision
}

// Handle player input
void handle_input(void) {
    if (kbhit()) {
        int key = get_key();
        
        // Handle different key inputs
        switch (key) {
            case 'a':
            case 'A':
                if (player_car.x > 0) {
                    player_car.x--;
                }
                break;
                
            case 'd':
            case 'D':
                if (player_car.x < TRACK_WIDTH - 1) {
                    player_car.x++;
                }
                break;
                
            case 'q':
            case 'Q':
                game_running = 0;
                break;
                
            // Handle arrow keys (platform specific)
#ifdef _WIN32
            case 224:  // Arrow key prefix on Windows
                key = get_key();
                switch (key) {
                    case 75:  // Left arrow
                        if (player_car.x > 0) {
                            player_car.x--;
                        }
                        break;
                    case 77:  // Right arrow
                        if (player_car.x < TRACK_WIDTH - 1) {
                            player_car.x++;
                        }
                        break;
                }
                break;
#else
            case 27:  // ESC sequence for arrow keys on Unix
                if (kbhit() && get_key() == 91) {  // [
                    key = get_key();
                    switch (key) {
                        case 68:  // Left arrow
                            if (player_car.x > 0) {
                                player_car.x--;
                            }
                            break;
                        case 67:  // Right arrow
                            if (player_car.x < TRACK_WIDTH - 1) {
                                player_car.x++;
                            }
                            break;
                    }
                }
                break;
#endif
        }
    }
}

// Increase game difficulty over time
void increase_difficulty(void) {
    static int score_threshold = 100;
    
    if (score >= score_threshold && game_speed > 100) {
        game_speed -= SPEED_INCREASE;
        score_threshold += 100;
    }
}

// Display game over screen
void display_game_over(void) {
    CLEAR_SCREEN();
    
    printf("\n");
    printf("+===========================================+\n");
    printf("|              GAME OVER!                   |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("| *** Your car crashed into an obstacle! ***|\n");
    printf("|                                           |\n");
    printf("| Final Score: %-4d                         |\n", score);
    printf("| Speed Level Reached: %-2d                 |\n", (INITIAL_SPEED - game_speed) / SPEED_INCREASE + 1);
    printf("|                                           |\n");
    
    // Performance evaluation
    if (score >= 500) {
        printf("| Performance: EXCELLENT! [TROPHY]        |\n");
    } else if (score >= 300) {
        printf("| Performance: GREAT! [SILVER]            |\n");
    } else if (score >= 150) {
        printf("| Performance: GOOD! [BRONZE]             |\n");
    } else {
        printf("| Performance: Keep practicing! [TRAIN]   |\n");
    }
    
    printf("|                                          |\n");
    printf("| Thanks for playing ASCII Racing Game!    |\n");
    printf("+==========================================+\n");
}

// Display game rules and instructions
void display_racing_rules(void) {
    printf("\n+==========================================+\n");
    printf("|          ASCII RACING GAME RULES          |\n");
    printf("+==========================================+\n");
    printf("| [CAR] Control your car 'A' on the track  |\n");
    printf("| [X]   Avoid obstacles 'X' falling down   |\n");
    printf("| [<->] Use A/D keys or arrow keys to move |\n");
    printf("| [SPD] Speed increases as you survive     |\n");
    printf("| [PTS] Score points by avoiding obstacles |\n");
    printf("| [END] Game ends when you hit an obstacle |\n");
    printf("| [Q]   Press Q anytime to quit            |\n");
    printf("|                                          |\n");
    printf("| Goal: Survive as long as possible and    |\n");
    printf("| achieve the highest score you can!       |\n");
    printf("+==========================================+\n");
    printf("\nPress any key to start the race...");
    getchar();
}

// Main game function
void play_ascii_racing(void) {
    printf("\n+==========================================+\n");
    printf("|      [RACING] ASCII RACING GAME [RACE]  |\n");
    printf("+==========================================+\n");
    
    display_racing_rules();
    
    // Initialize game
    init_racing_game();
    hide_cursor();
    
    // Main game loop
    while (game_running) {
        // Handle player input
        handle_input();
        
        // Update game state
        spawn_obstacles();
        update_obstacles();
        increase_difficulty();
        
        // Check for collisions
        if (check_collision()) {
            game_running = 0;
            break;
        }
        
        // Draw the current game state
        draw_track();
        
        // Control game speed
        SLEEP(game_speed);
    }
    
    // Show cursor again and display game over
    show_cursor();
    display_game_over();
}
