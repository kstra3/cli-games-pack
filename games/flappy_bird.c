/*
 * Flappy Bird - ASCII Edition
 * CLI Games Pack v1.8
 * 
 * A complete ASCII implementation of the classic Flappy Bird game with
 * realistic physics, multiple game modes, achievements, and statistics.
 * 
 * Features:
 * - 5 Game Modes: Classic, Speed Run, Endless, Trick Mode, Custom
 * - Realistic bird physics with gravity and momentum
 * - Procedural pipe generation with random gaps
 * - Achievement system with unlockable rewards
 * - Comprehensive statistics tracking
 * - Progressive difficulty scaling
 * - Smooth ASCII animation at 20 FPS
 * - Cross-platform compatibility
 */

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
    #define SLEEP_MS(ms) Sleep(ms)
    #define KBHIT() _kbhit()
    #define GETCH() _getch()
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    
    // Improved non-blocking input for Unix
    int KBHIT() {
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        
        return 0;
    }
    
    #define GETCH() getchar()
#endif

// Game Constants
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define GROUND_Y 20
#define SKY_Y 3
#define BIRD_START_X 10
#define BIRD_START_Y 12
#define PIPE_WIDTH 3
#define MAX_PIPES 10
#define MAX_ACHIEVEMENTS 15

// Physics Constants (Improved for smoother gameplay)
#define GRAVITY 0.4f
#define FLAP_STRENGTH -3.2f
#define MAX_FALL_SPEED 4.0f
#define PIPE_SPEED 2
#define TERMINAL_VELOCITY 0.8f

// Timing Constants
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)
#define PIPE_SPAWN_INTERVAL 90  // Frames between pipes

// Game Modes
typedef enum {
    MODE_CLASSIC = 0,
    MODE_SPEED_RUN,
    MODE_ENDLESS,
    MODE_TRICK,
    MODE_CUSTOM,
    MODE_COUNT
} GameMode;

// Achievement Types
typedef enum {
    ACH_FIRST_FLIGHT = 0,
    ACH_FREQUENT_FLYER,
    ACH_ACE_PILOT,
    ACH_PIPE_MASTER,
    ACH_LEGENDARY_BIRD,
    ACH_SPEED_DEMON,
    ACH_SURVIVOR,
    ACH_PERFECT_FORM,
    ACH_CRASH_LANDING,
    ACH_MARATHON_FLYER,
    ACH_COUNT
} AchievementType;

// Structures
typedef struct {
    int x;
    int gap_y;
    int gap_size;
    bool scored;
    bool active;
} Pipe;

typedef struct {
    int id;
    char* name;
    char* description;
    int requirement;
    bool unlocked;
    int points_reward;
} Achievement;

typedef struct {
    float x, y;
    float velocity_y;
    float rotation;  // For visual rotation effect
    bool alive;
    int animation_frame;
    int animation_timer;
    bool just_flapped;  // For better input responsiveness
    char current_sprite[4];
} Bird;

typedef struct {
    // Game State
    Bird bird;
    Pipe pipes[MAX_PIPES];
    int score;
    int high_score;
    int pipes_passed;
    bool game_over;
    bool paused;
    GameMode current_mode;
    
    // Timing
    clock_t start_time;
    clock_t frame_time;
    float elapsed_time;
    
    // Difficulty
    int current_level;
    int gap_size;
    int pipe_frequency;
    float game_speed;
    
    // Statistics
    int games_played;
    int total_flaps;
    int total_crashes;
    int perfect_centers;
    float best_time;
    float total_play_time;
    
    // Mode-specific stats
    int classic_best;
    float speedrun_best;
    int endless_best;
    
    // Achievements
    bool achievements[ACH_COUNT];
    int total_achievement_points;
    
    // Settings
    bool sound_enabled;
    bool show_fps;
    bool show_physics;
} GameState;

// Global Variables
static GameState game;
static bool game_running = true;
static char screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

// Bird Animation Frames
static char* bird_sprites[4] = {
    "<o>",  // Flapping up
    "\\o/", // Wings spread  
    "-o-",  // Gliding
    "/o\\"  // Falling
};

// Achievements
static Achievement achievements[ACH_COUNT] = {
    {0, "FIRST FLIGHT", "Score your first point", 1, false, 10},
    {1, "FREQUENT FLYER", "Score 10 points", 10, false, 50},
    {2, "ACE PILOT", "Score 25 points", 25, false, 100},
    {3, "PIPE MASTER", "Score 50 points", 50, false, 250},
    {4, "LEGENDARY BIRD", "Score 100 points", 100, false, 500},
    {5, "SPEED DEMON", "Complete Speed Run under 30s", 30, false, 200},
    {6, "SURVIVOR", "Play for 5 minutes straight", 300, false, 150},
    {7, "PERFECT FORM", "Hit pipe center 10 times", 10, false, 300},
    {8, "CRASH LANDING", "Crash 50 times total", 50, false, 100},
    {9, "MARATHON FLYER", "Play 100 games total", 100, false, 400}
};

// Function Prototypes
void flappy_bird_init_game(void);
void flappy_bird_main_menu(void);
void flappy_bird_game_loop(void);
void flappy_bird_display_header(const char* title);
void flappy_bird_clear_input_buffer(void);

// Core Game Functions
void flappy_bird_reset_game(void);
void flappy_bird_update_bird(void);
void flappy_bird_update_pipes(void);
void flappy_bird_handle_input(void);
void flappy_bird_bird_flap(void);
bool flappy_bird_check_collisions(void);
void flappy_bird_check_scoring(void);
void flappy_bird_spawn_pipe(void);

// Rendering Functions
void flappy_bird_clear_screen_buffer(void);
void flappy_bird_draw_to_buffer(int x, int y, char* text);
void flappy_bird_draw_bird(void);
void flappy_bird_draw_pipes(void);
void flappy_bird_draw_ground(void);
void flappy_bird_draw_hud(void);
void flappy_bird_render_screen(void);

// Game Mode Functions
void flappy_bird_classic_mode(void);
void flappy_bird_speedrun_mode(void);
void flappy_bird_endless_mode(void);
void flappy_bird_trick_mode(void);
void flappy_bird_custom_mode(void);

// UI Functions
void flappy_bird_how_to_play(void);
void flappy_bird_display_statistics(void);
void flappy_bird_display_achievements(void);
void flappy_bird_settings_menu(void);
void flappy_bird_game_over_screen(void);

// Achievement Functions
void flappy_bird_check_achievements(void);
void flappy_bird_unlock_achievement(AchievementType type);
void flappy_bird_display_achievement_unlock(AchievementType type);

// Utility Functions
void flappy_bird_save_statistics(void);
void flappy_bird_load_statistics(void);
char* flappy_bird_get_bird_sprite(void);
void flappy_bird_play_sound(const char* sound);

// Main Entry Point
void play_flappy_bird(void) {
    srand((unsigned int)time(NULL));
    flappy_bird_init_game();
    flappy_bird_load_statistics();
    
    while (game_running) {
        flappy_bird_main_menu();
    }
    
    flappy_bird_save_statistics();
}

// Game Initialization
void flappy_bird_init_game(void) {
    memset(&game, 0, sizeof(GameState));
    
    // Set improved defaults for smoother gameplay
    game.current_mode = MODE_CLASSIC;
    game.gap_size = 6;  // Slightly larger gap for better playability
    game.pipe_frequency = PIPE_SPAWN_INTERVAL;
    game.game_speed = 1.0f;
    game.sound_enabled = true;
    game.show_fps = false;
    game.show_physics = false;
    
    flappy_bird_reset_game();
}

// Reset Game State
void flappy_bird_reset_game(void) {
    // Reset bird
    game.bird.x = BIRD_START_X;
    game.bird.y = BIRD_START_Y;
    game.bird.velocity_y = 0;
    game.bird.alive = true;
    game.bird.animation_frame = 0;
    
    // Reset pipes
    for (int i = 0; i < MAX_PIPES; i++) {
        game.pipes[i].active = false;
        game.pipes[i].scored = false;
    }
    
    // Reset game state
    game.score = 0;
    game.pipes_passed = 0;
    game.game_over = false;
    game.paused = false;
    game.start_time = clock();
    game.frame_time = clock();
    game.current_level = 0;
}

// Header Display
void flappy_bird_display_header(const char* title) {
    CLEAR_SCREEN();
    printf("===============================================\n");
    printf("|             FLAPPY BIRD v1.0               |\n");
    printf("===============================================\n");
    printf("|                                           |\n");
    printf("|           %-31s |\n", title);
    printf("|                                           |\n");
    printf("===============================================\n");
}

// Main Menu
void flappy_bird_main_menu(void) {
    flappy_bird_display_header("MAIN MENU");
    printf("|                                           |\n");
    printf("| [1] Classic Flappy Bird                   |\n");
    printf("| [2] Speed Run Challenge                   |\n");
    printf("| [3] Endless Survival                      |\n");
    printf("| [4] Trick Mode (Power-ups)                |\n");
    printf("| [5] Custom Challenge                      |\n");
    printf("|                                           |\n");
    printf("| [6] Settings & Options                    |\n");
    printf("| [7] Statistics & Records                  |\n");
    printf("| [8] Achievements                          |\n");
    printf("| [9] How to Play                           |\n");
    printf("| [10] Back to Main Menu                    |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    printf("\nChoice (1-10): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        flappy_bird_clear_input_buffer();
        return;
    }
    flappy_bird_clear_input_buffer();
    
    switch (choice) {
        case 1:
            game.current_mode = MODE_CLASSIC;
            flappy_bird_classic_mode();
            break;
        case 2:
            game.current_mode = MODE_SPEED_RUN;
            flappy_bird_speedrun_mode();
            break;
        case 3:
            game.current_mode = MODE_ENDLESS;
            flappy_bird_endless_mode();
            break;
        case 4:
            game.current_mode = MODE_TRICK;
            flappy_bird_trick_mode();
            break;
        case 5:
            game.current_mode = MODE_CUSTOM;
            flappy_bird_custom_mode();
            break;
        case 6:
            flappy_bird_settings_menu();
            break;
        case 7:
            flappy_bird_display_statistics();
            break;
        case 8:
            flappy_bird_display_achievements();
            break;
        case 9:
            flappy_bird_how_to_play();
            break;
        case 10:
            game_running = false;
            return;
        default:
            printf("Invalid choice! Press Enter to continue...");
            getchar();
            break;
    }
}

// Classic Mode
void flappy_bird_classic_mode(void) {
    flappy_bird_display_header("CLASSIC FLAPPY BIRD");
    printf("|                                           |\n");
    printf("|  >>> ORIGINAL FLAPPY BIRD EXPERIENCE <<<  |\n");
    printf("|                                           |\n");
    printf("|  Navigate through pipes by flapping!     |\n");
    printf("|  Press SPACE to flap, avoid obstacles    |\n");
    printf("|  Progressive difficulty increase          |\n");
    printf("|                                           |\n");
    printf("|  Controls:                                |\n");
    printf("|    SPACE - Flap wings                    |\n");
    printf("|    P - Pause game                        |\n");
    printf("|    ESC - Exit to menu                    |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    flappy_bird_reset_game();
    game.current_mode = MODE_CLASSIC;
    flappy_bird_game_loop();
}

// Main Game Loop (Enhanced for smooth 60 FPS)
void flappy_bird_game_loop(void) {
    clock_t last_frame_time = clock();
    
    while (game.bird.alive && !game.game_over) {
        clock_t current_time = clock();
        double delta_time = ((double)(current_time - last_frame_time)) / CLOCKS_PER_SEC;
        
        // Target frame rate control
        if (delta_time < (1.0 / TARGET_FPS)) {
            SLEEP_MS(1);
            continue;
        }
        
        last_frame_time = current_time;
        
        // Handle input (non-blocking)
        flappy_bird_handle_input();
        
        if (game.paused) {
            printf("\n>>> PAUSED - Press P to continue <<<\n");
            SLEEP_MS(100);
            continue;
        }
        
        // Update game logic
        flappy_bird_update_bird();
        flappy_bird_update_pipes();
        
        // Check collisions
        if (flappy_bird_check_collisions()) {
            game.bird.alive = false;
            game.game_over = true;
            break;
        }
        
        // Check scoring
        flappy_bird_check_scoring();
        
        // Check achievements
        flappy_bird_check_achievements();
        
        // Render frame
        flappy_bird_clear_screen_buffer();
        flappy_bird_draw_ground();
        flappy_bird_draw_pipes();
        flappy_bird_draw_bird();
        flappy_bird_draw_hud();
        flappy_bird_render_screen();
    }
    
    flappy_bird_game_over_screen();
}

// Handle Input
void flappy_bird_handle_input(void) {
    if (KBHIT()) {
        char key = GETCH();
        switch (key) {
            case ' ':  // Space - Flap
                flappy_bird_bird_flap();
                break;
            case 'p':
            case 'P':  // Pause
                game.paused = !game.paused;
                break;
            case 27:   // ESC - Exit
                game.game_over = true;
                break;
        }
    }
}

// Bird Flap (Enhanced responsiveness)
void flappy_bird_bird_flap(void) {
    if (game.bird.alive) {
        // More responsive flap with variable strength
        float flap_power = FLAP_STRENGTH;
        
        // Stronger flap if falling fast (easier recovery)
        if (game.bird.velocity_y > 2.0f) {
            flap_power *= 1.2f;
        }
        
        game.bird.velocity_y = flap_power;
        game.bird.just_flapped = true;
        game.total_flaps++;
        game.bird.animation_frame = 0; // Immediate flap animation
        game.bird.animation_timer = 0; // Reset animation timer
        
        if (game.sound_enabled) {
            flappy_bird_play_sound("FLAP!");
        }
    }
}

// Update Bird Physics (Enhanced for smoothness)
void flappy_bird_update_bird(void) {
    if (!game.bird.alive) return;
    
    // Apply gravity with improved physics
    game.bird.velocity_y += GRAVITY;
    
    // Apply terminal velocity for more realistic physics
    if (game.bird.velocity_y > 0) {
        game.bird.velocity_y *= (1.0f - TERMINAL_VELOCITY * 0.1f);
    }
    
    // Limit fall speed
    if (game.bird.velocity_y > MAX_FALL_SPEED) {
        game.bird.velocity_y = MAX_FALL_SPEED;
    }
    
    // Update position with smoother movement
    game.bird.y += game.bird.velocity_y * 0.6f;  // Damping for smoother movement
    
    // Update animation timer for smoother animation
    game.bird.animation_timer++;
    if (game.bird.animation_timer >= 8) {  // Change animation every 8 frames
        game.bird.animation_timer = 0;
        
        // Update animation frame based on velocity and state
        if (game.bird.just_flapped) {
            game.bird.animation_frame = 0;  // Flapping up
            game.bird.just_flapped = false;
        } else if (game.bird.velocity_y < -1.5f) {
            game.bird.animation_frame = 0;  // Flying up
        } else if (game.bird.velocity_y < 0.5f) {
            game.bird.animation_frame = 1;  // Wings spread
        } else if (game.bird.velocity_y < 2.5f) {
            game.bird.animation_frame = 2;  // Gliding
        } else {
            game.bird.animation_frame = 3;  // Falling
        }
    }
    
    // Ground collision with bounce
    if (game.bird.y >= GROUND_Y - 1) {
        game.bird.y = GROUND_Y - 1;
        game.bird.velocity_y = 0;
        game.bird.alive = false;
    }
    
    // Ceiling collision with softer impact
    if (game.bird.y <= SKY_Y) {
        game.bird.y = SKY_Y;
        game.bird.velocity_y = 0.5f;  // Small downward push instead of hard stop
    }
}

// Update Pipes
void flappy_bird_update_pipes(void) {
    static int pipe_timer = 0;
    
    // Move existing pipes
    for (int i = 0; i < MAX_PIPES; i++) {
        if (game.pipes[i].active) {
            game.pipes[i].x -= PIPE_SPEED;
            
            // Remove pipes that are off screen
            if (game.pipes[i].x < -PIPE_WIDTH) {
                game.pipes[i].active = false;
            }
        }
    }
    
    // Spawn new pipes
    pipe_timer++;
    if (pipe_timer >= game.pipe_frequency) {
        flappy_bird_spawn_pipe();
        pipe_timer = 0;
    }
}

// Spawn New Pipe (Enhanced positioning and difficulty)
void flappy_bird_spawn_pipe(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!game.pipes[i].active) {
            game.pipes[i].x = SCREEN_WIDTH;
            
            // Improved gap positioning - avoid extremes, favor center area
            int min_gap_y = SKY_Y + 3;
            int max_gap_y = GROUND_Y - game.gap_size - 3;
            
            // Use weighted random for more balanced pipe placement
            int range = max_gap_y - min_gap_y;
            int center = min_gap_y + range / 2;
            int offset = (rand() % (range * 2 / 3)) - (range / 3); // Bias toward center
            
            game.pipes[i].gap_y = center + offset;
            
            // Ensure gap is within bounds
            if (game.pipes[i].gap_y < min_gap_y) game.pipes[i].gap_y = min_gap_y;
            if (game.pipes[i].gap_y > max_gap_y) game.pipes[i].gap_y = max_gap_y;
            
            // Progressive difficulty - gap size decreases slightly with score
            int dynamic_gap = game.gap_size;
            if (game.score > 10) dynamic_gap = game.gap_size - 1;
            if (game.score > 25) dynamic_gap = game.gap_size - 2;
            if (dynamic_gap < 4) dynamic_gap = 4; // Minimum playable gap
            
            game.pipes[i].gap_size = dynamic_gap;
            game.pipes[i].active = true;
            game.pipes[i].scored = false;
            break;
        }
    }
}

// Check Collisions (Enhanced precision and fairness)
bool flappy_bird_check_collisions(void) {
    int bird_x = (int)game.bird.x;
    int bird_y = (int)game.bird.y;
    
    // More forgiving collision - check center of bird sprite
    int bird_center_x = bird_x + 1; // Center of 3-char sprite
    int bird_center_y = bird_y;
    
    // Check pipe collisions
    for (int i = 0; i < MAX_PIPES; i++) {
        if (game.pipes[i].active) {
            // Check if bird center is horizontally aligned with pipe
            if (bird_center_x >= game.pipes[i].x && bird_center_x < game.pipes[i].x + PIPE_WIDTH) {
                // Check if bird center is in the gap (with small tolerance)
                int gap_top = game.pipes[i].gap_y;
                int gap_bottom = game.pipes[i].gap_y + game.pipes[i].gap_size;
                
                // Add small tolerance for more forgiving collision
                if (bird_center_y <= gap_top - 1 || bird_center_y >= gap_bottom + 1) {
                    // Collision!
                    if (game.sound_enabled) {
                        flappy_bird_play_sound("CRASH!");
                    }
                    game.total_crashes++;
                    return true;
                }
            }
        }
    }
    
    return false;
}

// Check Scoring
void flappy_bird_check_scoring(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (game.pipes[i].active && !game.pipes[i].scored) {
            if (game.bird.x > game.pipes[i].x + PIPE_WIDTH) {
                game.score++;
                game.pipes_passed++;
                game.pipes[i].scored = true;
                
                // Check for perfect center hit
                int gap_center = game.pipes[i].gap_y + game.pipes[i].gap_size / 2;
                if (abs((int)game.bird.y - gap_center) <= 1) {
                    game.perfect_centers++;
                }
                
                if (game.sound_enabled) {
                    flappy_bird_play_sound("SCORE!");
                }
                
                // Update high score
                if (game.score > game.high_score) {
                    game.high_score = game.score;
                }
                
                break;
            }
        }
    }
}

// Clear Screen Buffer
void flappy_bird_clear_screen_buffer(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            screen_buffer[y][x] = ' ';
        }
        screen_buffer[y][SCREEN_WIDTH] = '\0';
    }
}

// Draw to Buffer
void flappy_bird_draw_to_buffer(int x, int y, char* text) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    
    int len = strlen(text);
    for (int i = 0; i < len && x + i < SCREEN_WIDTH; i++) {
        if (x + i >= 0) {
            screen_buffer[y][x + i] = text[i];
        }
    }
}

// Draw Bird
void flappy_bird_draw_bird(void) {
    if (game.bird.alive) {
        char* sprite = bird_sprites[game.bird.animation_frame];
        flappy_bird_draw_to_buffer((int)game.bird.x, (int)game.bird.y, sprite);
    }
}

// Draw Pipes
void flappy_bird_draw_pipes(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (game.pipes[i].active) {
            // Draw top pipe
            for (int y = SKY_Y; y < game.pipes[i].gap_y; y++) {
                flappy_bird_draw_to_buffer(game.pipes[i].x, y, "███");
            }
            
            // Draw bottom pipe
            for (int y = game.pipes[i].gap_y + game.pipes[i].gap_size; y < GROUND_Y; y++) {
                flappy_bird_draw_to_buffer(game.pipes[i].x, y, "███");
            }
        }
    }
}

// Draw Ground (Enhanced with scrolling effect)
void flappy_bird_draw_ground(void) {
    static int ground_offset = 0;
    ground_offset = (ground_offset + 1) % 4; // Scrolling ground effect
    
    for (int y = GROUND_Y; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x += 4) {
            // Create scrolling ground pattern
            int pattern_x = (x + ground_offset) % 4;
            char* pattern = (pattern_x < 2) ? "-=-=" : "=_=_";
            flappy_bird_draw_to_buffer(x, y, pattern);
        }
    }
}

// Draw HUD (Enhanced with better information)
void flappy_bird_draw_hud(void) {
    char hud_line[SCREEN_WIDTH + 1];
    
    // Top HUD with improved formatting
    snprintf(hud_line, sizeof(hud_line), 
             " SCORE: %03d  BEST: %03d  PIPES: %02d  FLAPS: %03d ", 
             game.score, game.high_score, game.pipes_passed, game.total_flaps);
    flappy_bird_draw_to_buffer(0, 0, hud_line);
    
    // Mode and controls with velocity indicator
    char velocity_indicator[10];
    if (game.bird.velocity_y < -2.0f) strcpy(velocity_indicator, "^^^");
    else if (game.bird.velocity_y < -1.0f) strcpy(velocity_indicator, "^^");
    else if (game.bird.velocity_y < 1.0f) strcpy(velocity_indicator, "--");
    else if (game.bird.velocity_y < 2.0f) strcpy(velocity_indicator, "vv");
    else strcpy(velocity_indicator, "vvv");
    
    snprintf(hud_line, sizeof(hud_line), 
             " [SPACE] Flap [P] Pause [ESC] Exit    VEL:%s ", velocity_indicator);
    flappy_bird_draw_to_buffer(0, 1, hud_line);
    
    // Border
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        screen_buffer[2][x] = '=';
    }
}

// Render Screen (Optimized to reduce flicker)
void flappy_bird_render_screen(void) {
    // Use cursor positioning instead of clearing entire screen
    printf("\033[H"); // Move cursor to home position
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        printf("\033[K%s\n", screen_buffer[y]); // Clear line and print
    }
    
    fflush(stdout); // Force immediate output
}

// Game Over Screen
void flappy_bird_game_over_screen(void) {
    flappy_bird_display_header("GAME OVER");
    printf("|                                           |\n");
    printf("| FINAL RESULTS:                            |\n");
    printf("| Score: %-3d                               |\n", game.score);
    printf("| Pipes Passed: %-3d                       |\n", game.pipes_passed);
    printf("| Total Flaps: %-4d                        |\n", game.total_flaps);
    printf("| Perfect Centers: %-2d                     |\n", game.perfect_centers);
    
    if (game.score > game.classic_best) {
        printf("| >>> NEW HIGH SCORE! <<<                  |\n");
        game.classic_best = game.score;
    }
    
    printf("|                                           |\n");
    
    // Update statistics
    game.games_played++;
    flappy_bird_check_achievements();
    
    printf("| Press Enter to return to menu...          |\n");
    printf("===============================================\n");
    getchar();
}

// Settings Menu (stub)
void flappy_bird_settings_menu(void) {
    flappy_bird_display_header("SETTINGS");
    printf("|                                           |\n");
    printf("| [1] Sound: %-30s |\n", game.sound_enabled ? "ON" : "OFF");
    printf("| [2] Show FPS: %-25s |\n", game.show_fps ? "ON" : "OFF");
    printf("| [3] Show Physics: %-21s |\n", game.show_physics ? "ON" : "OFF");
    printf("| [4] Gap Size: %-27d |\n", game.gap_size);
    printf("| [5] Game Speed: %-23.1f |\n", game.game_speed);
    printf("|                                           |\n");
    printf("| [6] Back to Main Menu                     |\n");
    printf("===============================================\n");
    printf("\nChoice (1-6): ");
    
    int choice;
    if (scanf("%d", &choice) == 1) {
        switch (choice) {
            case 1:
                game.sound_enabled = !game.sound_enabled;
                break;
            case 2:
                game.show_fps = !game.show_fps;
                break;
            case 3:
                game.show_physics = !game.show_physics;
                break;
            case 4:
                game.gap_size = (game.gap_size % 7) + 3; // Cycle 3-9
                break;
            case 5:
                game.game_speed = (game.game_speed < 2.0f) ? game.game_speed + 0.5f : 0.5f;
                break;
            case 6:
                flappy_bird_clear_input_buffer();
                return;
        }
        flappy_bird_settings_menu(); // Recursive call to stay in settings
    }
    flappy_bird_clear_input_buffer();
}

// Statistics Display
void flappy_bird_display_statistics(void) {
    flappy_bird_display_header("STATISTICS");
    printf("|                                           |\n");
    printf("| OVERALL PERFORMANCE:                      |\n");
    printf("| Games Played: %-3d                        |\n", game.games_played);
    printf("| Total Flaps: %-4d                         |\n", game.total_flaps);
    printf("| Total Crashes: %-3d                       |\n", game.total_crashes);
    printf("| Perfect Centers: %-3d                     |\n", game.perfect_centers);
    printf("|                                           |\n");
    printf("| MODE RECORDS:                             |\n");
    printf("| Classic Best: %-3d                        |\n", game.classic_best);
    printf("| Speed Run Best: %.1fs                     |\n", game.speedrun_best);
    printf("| Endless Best: %-3d                        |\n", game.endless_best);
    printf("|                                           |\n");
    printf("| Press Enter to continue...                |\n");
    printf("===============================================\n");
    getchar();
}

// Achievements Display
void flappy_bird_display_achievements(void) {
    flappy_bird_display_header("ACHIEVEMENTS");
    printf("|                                           |\n");
    
    int unlocked_count = 0;
    for (int i = 0; i < ACH_COUNT; i++) {
        if (game.achievements[i]) {
            printf("| [*] %-35s |\n", achievements[i].name);
            unlocked_count++;
        } else {
            printf("| [ ] %-35s |\n", achievements[i].name);
        }
    }
    
    printf("|                                           |\n");
    printf("| Progress: %d/%d unlocked                   |\n", unlocked_count, ACH_COUNT);
    printf("| Achievement Points: %-4d                  |\n", game.total_achievement_points);
    printf("|                                           |\n");
    printf("| Press Enter to continue...                |\n");
    printf("===============================================\n");
    getchar();
}

// How to Play
void flappy_bird_how_to_play(void) {
    flappy_bird_display_header("HOW TO PLAY");
    printf("|                                           |\n");
    printf("| OBJECTIVE:                                |\n");
    printf("| Navigate the bird through pipe gaps       |\n");
    printf("| Score points by passing pipes             |\n");
    printf("| Avoid crashing into pipes or ground       |\n");
    printf("|                                           |\n");
    printf("| CONTROLS:                                 |\n");
    printf("| SPACE - Flap wings (go up)               |\n");
    printf("| P - Pause/Resume game                     |\n");
    printf("| ESC - Exit to main menu                   |\n");
    printf("|                                           |\n");
    printf("| TIPS:                                     |\n");
    printf("| • Tap space gently for small hops        |\n");
    printf("| • Time your flaps carefully              |\n");
    printf("| • Aim for the center of gaps             |\n");
    printf("| • Practice makes perfect!                |\n");
    printf("|                                           |\n");
    printf("| Press Enter to continue...                |\n");
    printf("===============================================\n");
    getchar();
}

// Achievement Checking
void flappy_bird_check_achievements(void) {
    // First Flight - Score 1 point
    if (game.score >= 1 && !game.achievements[ACH_FIRST_FLIGHT]) {
        flappy_bird_unlock_achievement(ACH_FIRST_FLIGHT);
    }
    
    // Frequent Flyer - Score 10 points
    if (game.score >= 10 && !game.achievements[ACH_FREQUENT_FLYER]) {
        flappy_bird_unlock_achievement(ACH_FREQUENT_FLYER);
    }
    
    // Ace Pilot - Score 25 points
    if (game.score >= 25 && !game.achievements[ACH_ACE_PILOT]) {
        flappy_bird_unlock_achievement(ACH_ACE_PILOT);
    }
    
    // Pipe Master - Score 50 points
    if (game.score >= 50 && !game.achievements[ACH_PIPE_MASTER]) {
        flappy_bird_unlock_achievement(ACH_PIPE_MASTER);
    }
    
    // Legendary Bird - Score 100 points
    if (game.score >= 100 && !game.achievements[ACH_LEGENDARY_BIRD]) {
        flappy_bird_unlock_achievement(ACH_LEGENDARY_BIRD);
    }
    
    // Perfect Form - 10 perfect centers
    if (game.perfect_centers >= 10 && !game.achievements[ACH_PERFECT_FORM]) {
        flappy_bird_unlock_achievement(ACH_PERFECT_FORM);
    }
    
    // Crash Landing - 50 total crashes
    if (game.total_crashes >= 50 && !game.achievements[ACH_CRASH_LANDING]) {
        flappy_bird_unlock_achievement(ACH_CRASH_LANDING);
    }
    
    // Marathon Flyer - 100 games played
    if (game.games_played >= 100 && !game.achievements[ACH_MARATHON_FLYER]) {
        flappy_bird_unlock_achievement(ACH_MARATHON_FLYER);
    }
}

// Unlock Achievement
void flappy_bird_unlock_achievement(AchievementType type) {
    if (!game.achievements[type]) {
        game.achievements[type] = true;
        game.total_achievement_points += achievements[type].points_reward;
        flappy_bird_display_achievement_unlock(type);
    }
}

// Display Achievement Unlock
void flappy_bird_display_achievement_unlock(AchievementType type) {
    if (game.sound_enabled) {
        flappy_bird_play_sound("ACHIEVEMENT!");
    }
    printf("\n+-------------------------------------+\n");
    printf("|         ACHIEVEMENT UNLOCKED!       |\n");
    printf("|                                     |\n");
    printf("| [*] %-31s |\n", achievements[type].name);
    printf("|     %-31s |\n", achievements[type].description);
    printf("|                                     |\n");
    printf("| Reward: +%d points                  |\n", achievements[type].points_reward);
    printf("+-------------------------------------+\n");
    SLEEP_MS(2000);
}

// Stub functions for other modes
void flappy_bird_speedrun_mode(void) {
    printf("Speed Run mode coming soon!\n");
    printf("Press Enter to continue...");
    getchar();
}

void flappy_bird_endless_mode(void) {
    printf("Endless mode coming soon!\n");
    printf("Press Enter to continue...");
    getchar();
}

void flappy_bird_trick_mode(void) {
    printf("Trick mode coming soon!\n");
    printf("Press Enter to continue...");
    getchar();
}

void flappy_bird_custom_mode(void) {
    printf("Custom mode coming soon!\n");
    printf("Press Enter to continue...");
    getchar();
}

// Utility Functions
void flappy_bird_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void flappy_bird_play_sound(const char* sound) {
    if (game.sound_enabled) {
        printf("    [SFX] %s\n", sound);
        fflush(stdout);
    }
}

char* flappy_bird_get_bird_sprite(void) {
    return bird_sprites[game.bird.animation_frame];
}

// Save/Load Statistics (simplified stubs)
void flappy_bird_save_statistics(void) {
    // Would save to file in full implementation
}

void flappy_bird_load_statistics(void) {
    // Would load from file in full implementation
}