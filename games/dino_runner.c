/*
 * Chrome Dino Runner - Enhanced ASCII Edition
 * Part of CLI Games Pack v1.8
 * 
 * A faithful recreation of the Chrome offline dinosaur game
 * with enhanced features, multiple game modes, and achievements
 * 
 * Features:
 * - Realistic physics with gravity and momentum
 * - Multiple obstacle types (cactus, rocks, birds)
 * - Day/night cycle with visual changes
 * - Progressive difficulty scaling
 * - Achievement system with 15+ achievements
 * - Multiple game modes (Classic, Sprint, Marathon, etc.)
 * - Statistics tracking and high scores
 * - Smooth ASCII animations
 * - Sound effects (text-based)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "games.h"

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP_MS(ms) Sleep(ms)
    #define DINO_KBHIT() games_kbhit()
    #define GETCH() _getch()
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define DINO_KBHIT() games_kbhit()
    #define GETCH() getchar()
#endif

// Game Constants (Enhanced for smoother gameplay)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define GROUND_Y 18
#define SKY_Y 3
#define DINO_X 8
#define DINO_START_Y (GROUND_Y - 3)
#define MAX_OBSTACLES 20
#define MAX_CLOUDS 10
#define MAX_ACHIEVEMENTS 20

// Enhanced Physics Constants
#define GRAVITY 0.6f           // Slightly increased gravity for better jump feel
#define JUMP_POWER -6.0f       // Reduced jump height for better playability
#define DUCK_SPEED 1.2f        // Faster ducking response
#define DUCK_DURATION 12       // Shorter duck duration for better control
#define MAX_FALL_SPEED 6.0f    // Reduced terminal velocity for smoother landing
#define GROUND_FRICTION 0.92f  // Subtle ground friction
#define JUMP_BUFFER_TIME 5     // Frames to buffer jump input
#define COYOTE_TIME 3          // Frames after leaving ground where jump still works

// Timing and Performance
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)
#define MAX_GAME_SPEED 20      // Slightly reduced max speed for better control
#define DAY_NIGHT_CYCLE 800    // Slower day/night transitions

// Enums
typedef enum {
    DINO_RUNNING,
    DINO_JUMPING,
    DINO_DUCKING,
    DINO_DEAD
} DinoState;

typedef enum {
    OBSTACLE_SMALL_CACTUS,
    OBSTACLE_LARGE_CACTUS,
    OBSTACLE_ROCK,
    OBSTACLE_BIRD_HIGH,
    OBSTACLE_BIRD_LOW,
    OBSTACLE_DOUBLE_CACTUS,
    OBSTACLE_TRIPLE_CACTUS,    // New: Three cacti in a row
    OBSTACLE_BIRD_SWARM,       // New: Multiple birds
    OBSTACLE_ROLLING_ROCK,     // New: Moving rock
    OBSTACLE_TALL_TREE,        // New: Very tall obstacle
    OBSTACLE_LOW_BRANCH,       // New: Low hanging branch
    OBSTACLE_SPIKE_TRAP,       // New: Ground spikes
    OBSTACLE_COUNT
} ObstacleType;

typedef enum {
    TIME_DAY,
    TIME_SUNSET,
    TIME_NIGHT,
    TIME_SUNRISE
} TimeOfDay;

typedef enum {
    MODE_CLASSIC,
    MODE_SPRINT,
    MODE_MARATHON,
    MODE_OBSTACLE_COURSE,
    MODE_CUSTOM
} GameMode;

typedef enum {
    ACH_FIRST_JUMP,
    ACH_SCORE_100,
    ACH_SCORE_500,
    ACH_SCORE_1000,
    ACH_SCORE_2500,
    ACH_SCORE_5000,
    ACH_MARATHON_RUNNER,
    ACH_SPEED_DEMON,
    ACH_NIGHT_RUNNER,
    ACH_DUCK_MASTER,
    ACH_PERFECT_TIMING,
    ACH_CLOSE_CALLS,
    ACH_SURVIVAL_EXPERT,
    ACH_DINO_LEGEND,
    ACH_EXTINCTION_AVOIDED,
    ACH_COUNT
} AchievementType;

// Structures
typedef struct {
    float x, y;
    float velocity_y;
    DinoState state;
    int animation_frame;
    int animation_timer;
    bool on_ground;
    int duck_timer;
    int jump_buffer;       // Jump input buffering
    int coyote_timer;      // Coyote time for late jumps
    bool duck_held;        // Whether duck is being held
    float last_ground_y;   // For smoother ground detection
} Dinosaur;

typedef struct {
    float x, y;
    ObstacleType type;
    bool active;
    bool scored;
    int width, height;
} Obstacle;

typedef struct {
    float x, y;
    bool active;
} Cloud;

typedef struct {
    int id;
    char* name;
    char* description;
    int requirement;
    bool unlocked;
    int points_reward;
} Achievement;

typedef struct {
    // Game state
    Dinosaur dino;
    Obstacle obstacles[MAX_OBSTACLES];
    Cloud clouds[MAX_CLOUDS];
    
    // Game variables
    int score;
    int high_score;
    float game_speed;
    bool game_running;
    bool game_over;
    GameMode current_mode;
    TimeOfDay time_of_day;
    int day_night_timer;
    bool is_night;
    
    // Statistics
    int total_jumps;
    int total_ducks;
    int obstacles_dodged;
    int close_calls;
    int perfect_jumps;
    float play_time;
    int games_played;
    
    // Ground animation
    int ground_offset;
    
    // Achievements
    Achievement achievements[MAX_ACHIEVEMENTS];
} GameState;

// Global game state
static GameState game;
static char screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

// ASCII Art Sprites
static char* dino_running_sprites[2] = {
    "  >o)\n /_/|\n  / \\",
    "  >o)\n /_/|\n /\\ _"
};

static char* dino_jumping_sprite = "  >o)\n  /|\n ^   ";

static char* dino_ducking_sprites[2] = {
    "     \n>o)__\n    /",
    "     \n>o)__\n   / "
};

static char* dino_dead_sprite = "  X_X\n /_/|\n  / \\";

static char* obstacle_sprites[OBSTACLE_COUNT] = {
    "|\n|",           // Small cactus
    "|||\n|||",       // Large cactus  
    "###\n###",       // Rock
    "^v^",            // Bird high
    "   \n^v^",       // Bird low
    "| |\n| |",       // Double cactus
    "||||\n||||",     // Triple cactus
    "^v^^v^",         // Bird swarm
    "@@@\n@@@",       // Rolling rock
    "|||\n|||\n|||",  // Tall tree
    "   \n---",       // Low branch
    "/\\/\\\n\\/\\/",   // Spike trap
};

static int obstacle_widths[OBSTACLE_COUNT] = {1, 3, 3, 3, 3, 3, 4, 6, 3, 3, 3, 4};
static int obstacle_heights[OBSTACLE_COUNT] = {2, 2, 2, 1, 1, 2, 2, 1, 2, 3, 1, 2};

// Function Declarations
void dino_runner_init_game(void);
void dino_runner_main_menu(void);
void dino_runner_game_loop(void);
void dino_runner_display_header(const char* title);
void dino_runner_clear_input_buffer(void);

// Game Functions
void dino_runner_reset_game(void);
void dino_runner_update_game(void);
void dino_runner_handle_input(void);
void dino_runner_update_dino(void);
void dino_runner_update_obstacles(void);
void dino_runner_update_clouds(void);
void dino_runner_check_collisions(void);
void dino_runner_spawn_obstacle(void);

// Rendering Functions
void dino_runner_clear_screen_buffer(void);
void dino_runner_draw_to_buffer(int x, int y, const char* text);
void dino_runner_draw_dino(void);
void dino_runner_draw_obstacles(void);
void dino_runner_draw_clouds(void);
void dino_runner_draw_ground(void);
void dino_runner_draw_hud(void);
void dino_runner_render_screen(void);

// Game Mode Functions
void dino_runner_classic_mode(void);
void dino_runner_sprint_mode(void);
void dino_runner_marathon_mode(void);
void dino_runner_obstacle_course_mode(void);
void dino_runner_custom_mode(void);

// UI Functions
void dino_runner_how_to_play(void);
void dino_runner_display_statistics(void);
void dino_runner_display_achievements(void);
void dino_runner_settings_menu(void);
void dino_runner_game_over_screen(void);

// Achievement Functions
void dino_runner_check_achievements(void);
void dino_runner_unlock_achievement(AchievementType type);
void dino_runner_display_achievement_unlock(AchievementType type);

// Utility Functions
void dino_runner_save_statistics(void);
void dino_runner_load_statistics(void);
void dino_runner_play_sound(const char* sound);
void dino_runner_update_day_night_cycle(void);
float dino_runner_calculate_distance(float x1, float y1, float x2, float y2);

// Main Entry Point
void play_dino_runner(void);

// Implementation

void dino_runner_init_game(void) {
    // Initialize achievements
    Achievement initial_achievements[MAX_ACHIEVEMENTS] = {
        {ACH_FIRST_JUMP, "First Steps", "Make your first jump", 1, false, 10},
        {ACH_SCORE_100, "Getting Started", "Score 100 points", 100, false, 25},
        {ACH_SCORE_500, "Warming Up", "Score 500 points", 500, false, 50},
        {ACH_SCORE_1000, "Dino Runner", "Score 1000 points", 1000, false, 100},
        {ACH_SCORE_2500, "Experienced Runner", "Score 2500 points", 2500, false, 200},
        {ACH_SCORE_5000, "Dino Master", "Score 5000 points", 5000, false, 500},
        {ACH_MARATHON_RUNNER, "Marathon Runner", "Play for 5 minutes", 300, false, 150},
        {ACH_SPEED_DEMON, "Speed Demon", "Reach maximum speed", 1, false, 200},
        {ACH_NIGHT_RUNNER, "Night Runner", "Experience day/night cycle", 1, false, 75},
        {ACH_DUCK_MASTER, "Duck Master", "Duck under 50 birds", 50, false, 125},
        {ACH_PERFECT_TIMING, "Perfect Timing", "Make 25 perfect jumps", 25, false, 175},
        {ACH_CLOSE_CALLS, "Close Calls", "Survive 10 near misses", 10, false, 100},
        {ACH_SURVIVAL_EXPERT, "Survival Expert", "Dodge 100 obstacles", 100, false, 250},
        {ACH_DINO_LEGEND, "Dino Legend", "Score 10000 points", 10000, false, 1000},
        {ACH_EXTINCTION_AVOIDED, "Extinction Avoided", "Play 50 games", 50, false, 300}
    };
    
    memcpy(game.achievements, initial_achievements, sizeof(initial_achievements));
    
    dino_runner_load_statistics();
    srand((unsigned int)time(NULL));
}

void dino_runner_main_menu(void) {
    int choice;
    
    while (1) {
        CLEAR_SCREEN();
        dino_runner_display_header("CHROME DINO RUNNER");
        
        printf("+===========================================+\n");
        printf("|                                           |\n");
        printf("|               >o)                         |\n");
        printf("|              /_/|                         |\n");
        printf("|               / \\                         |\n");
        printf("|                                           |\n");
        printf("|         1. Classic Mode                   |\n");
        printf("|         2. Sprint Mode                    |\n");
        printf("|         3. Marathon Mode                  |\n");
        printf("|         4. Obstacle Course                |\n");
        printf("|         5. Custom Mode                    |\n");
        printf("|                                           |\n");
        printf("|         6. How to Play                    |\n");
        printf("|         7. Statistics                     |\n");
        printf("|         8. Achievements                   |\n");
        printf("|         9. Settings                       |\n");
        printf("|                                           |\n");
        printf("|         0. Back to Main Menu              |\n");
        printf("|                                           |\n");
        printf("+===========================================+\n");
        printf("\n> Enter your choice (0-9): ");
        
        if (scanf("%d", &choice) != 1) {
            dino_runner_clear_input_buffer();
            choice = -1;
        }
        
        switch (choice) {
            case 1:
                game.current_mode = MODE_CLASSIC;
                dino_runner_classic_mode();
                break;
            case 2:
                game.current_mode = MODE_SPRINT;
                dino_runner_sprint_mode();
                break;
            case 3:
                game.current_mode = MODE_MARATHON;
                dino_runner_marathon_mode();
                break;
            case 4:
                game.current_mode = MODE_OBSTACLE_COURSE;
                dino_runner_obstacle_course_mode();
                break;
            case 5:
                game.current_mode = MODE_CUSTOM;
                dino_runner_custom_mode();
                break;
            case 6:
                dino_runner_how_to_play();
                break;
            case 7:
                dino_runner_display_statistics();
                break;
            case 8:
                dino_runner_display_achievements();
                break;
            case 9:
                dino_runner_settings_menu();
                break;
            case 0:
                return;
            default:
                printf("\n[!] Invalid choice! Please try again.\n");
                printf("Press any key to continue...");
                GETCH();
                break;
        }
    }
}

void dino_runner_classic_mode(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("CLASSIC MODE");
    
    printf("+===========================================+\n");
    printf("|            CHROME DINO CLASSIC           |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  * Authentic Chrome experience            |\n");
    printf("|  * Progressive speed increase             |\n");
    printf("|  * Day/night cycle                       |\n");
    printf("|  * Classic scoring system                |\n");
    printf("|  * Endless gameplay                      |\n");
    printf("|                                           |\n");
    printf("|  Controls:                                |\n");
    printf("|    [SPACE] - Jump                         |\n");
    printf("|    [DOWN]  - Duck                         |\n");
    printf("|    [ESC]   - Pause/Exit                   |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to start...");
    GETCH();
    
    dino_runner_reset_game();
    dino_runner_game_loop();
}

void dino_runner_sprint_mode(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("SPRINT MODE");
    
    printf("+===========================================+\n");
    printf("|             SPRINT CHALLENGE              |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  * Race to 1000 points!                  |\n");
    printf("|  * Fixed high speed                      |\n");
    printf("|  * Timer displayed                       |\n");
    printf("|  * Leaderboard for best times            |\n");
    printf("|  * Intense obstacle patterns             |\n");
    printf("|                                           |\n");
    printf("|  Goal: Reach 1000 points as fast         |\n");
    printf("|        as possible!                       |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to start sprint...");
    GETCH();
    
    game.game_speed = 15; // Fixed high speed for sprint
    dino_runner_reset_game();
    dino_runner_game_loop();
}

void dino_runner_marathon_mode(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("MARATHON MODE");
    
    printf("+===========================================+\n");
    printf("|            MARATHON ENDURANCE             |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  * Extreme speeds and challenges          |\n");
    printf("|  * Special marathon achievements          |\n");
    printf("|  * How far can you survive?               |\n");
    printf("|  * Ludicrous speed awaits...              |\n");
    printf("|  * Only for the brave!                    |\n");
    printf("|                                           |\n");
    printf("|  Warning: This mode is INTENSE!           |\n");
    printf("|           Prepare for chaos!              |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to start marathon...");
    GETCH();
    
    dino_runner_reset_game();
    game.game_speed = 8; // Start faster
    dino_runner_game_loop();
}

void dino_runner_obstacle_course_mode(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("OBSTACLE COURSE");
    
    printf("+===========================================+\n");
    printf("|          PRECISION CHALLENGE             |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  * 20 handcrafted levels                  |\n");
    printf("|  * Precise timing required                |\n");
    printf("|  * Star rating system                     |\n");
    printf("|  * Perfect jumps and ducks                |\n");
    printf("|  * Unlock new courses                     |\n");
    printf("|                                           |\n");
    printf("|  Coming Soon in v2.0!                     |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to return...");
    GETCH();
}

void dino_runner_custom_mode(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("CUSTOM MODE");
    
    printf("+===========================================+\n");
    printf("|            CUSTOMIZE EXPERIENCE          |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  * Set your own speed                     |\n");
    printf("|  * Choose obstacle types                  |\n");
    printf("|  * Design your challenge                  |\n");
    printf("|  * Save custom presets                    |\n");
    printf("|  * Share with friends                     |\n");
    printf("|                                           |\n");
    printf("|  Coming Soon in v2.0!                     |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to return...");
    GETCH();
}

void dino_runner_reset_game(void) {
    // Reset dinosaur with enhanced fields
    game.dino.x = DINO_X;
    game.dino.y = DINO_START_Y;
    game.dino.velocity_y = 0;
    game.dino.state = DINO_RUNNING;
    game.dino.animation_frame = 0;
    game.dino.animation_timer = 0;
    game.dino.on_ground = true;
    game.dino.duck_timer = 0;
    game.dino.jump_buffer = 0;      // Initialize jump buffering
    game.dino.coyote_timer = 0;     // Initialize coyote time
    game.dino.duck_held = false;    // Initialize duck state
    game.dino.last_ground_y = DINO_START_Y; // Initialize ground reference
    
    // Reset obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game.obstacles[i].active = false;
        game.obstacles[i].scored = false;
    }
    
    // Reset clouds
    for (int i = 0; i < MAX_CLOUDS; i++) {
        game.clouds[i].x = SCREEN_WIDTH + rand() % 100;
        game.clouds[i].y = rand() % 8 + 2;
        game.clouds[i].active = true;
    }
    
    // Reset game state
    game.score = 0;
    if (game.current_mode != MODE_SPRINT) {
        game.game_speed = 6;
    }
    game.game_running = true;
    game.game_over = false;
    game.time_of_day = TIME_DAY;
    game.day_night_timer = 0;
    game.is_night = false;
    game.ground_offset = 0;
    
    game.games_played++;
}

void dino_runner_game_loop(void) {
    clock_t start_time = clock();
    
    // Enhanced 60 FPS Game Loop with Delta Timing
    clock_t last_frame_time = clock();
    float frame_accumulator = 0.0f;
    const float target_frame_time = 1000.0f / TARGET_FPS; // ~16.67ms for 60 FPS
    
    while (game.game_running) {
        clock_t current_time = clock();
        float delta_time = (float)(current_time - last_frame_time) / CLOCKS_PER_SEC * 1000.0f;
        last_frame_time = current_time;
        
        frame_accumulator += delta_time;
        
        // Handle input at 60 FPS rate
        dino_runner_handle_input();
        
        // Update game logic at consistent rate
        if (frame_accumulator >= target_frame_time) {
            if (!game.game_over) {
                dino_runner_update_game();
            }
            
            // Smooth rendering with minimal flicker
            dino_runner_render_screen();
            
            frame_accumulator -= target_frame_time;
        }
        
        // Calculate play time
        game.play_time = (float)(clock() - start_time) / CLOCKS_PER_SEC;
        
        // Sleep for smooth 60 FPS (reduced from 50ms to ~16ms)
        SLEEP_MS(16);
    }
    
    dino_runner_save_statistics();
}

void dino_runner_handle_input(void) {
    // Enhanced input handling with buffering and improved responsiveness
    static bool duck_key_held = false;
    
    // Update input timers
    if (game.dino.jump_buffer > 0) game.dino.jump_buffer--;
    if (game.dino.coyote_timer > 0) game.dino.coyote_timer--;
    
    // Check for new input
    bool jump_pressed = false;
    bool duck_pressed = false;
    bool duck_released = false;
    
    if (DINO_KBHIT()) {
        int key = GETCH();
        
        switch (key) {
            case ' ': // Space - Jump
            case 'w': // W key for jump (alternative)
            case 'W':
                jump_pressed = true;
                game.dino.jump_buffer = JUMP_BUFFER_TIME; // Buffer the jump input
                break;
                
            case 's': // Down arrow - Duck
            case 'S':
                duck_pressed = true;
                duck_key_held = true;
                break;
                
            case 27: // ESC - Pause/Exit
                if (game.game_over) {
                    game.game_running = false;
                } else {
                    printf("\n\n[PAUSED] Press any key to continue or ESC to exit...");
                    int pause_key = GETCH();
                    if (pause_key == 27) {
                        game.game_running = false;
                    }
                }
                return;
                
            case 'r': // Restart when game over
            case 'R':
                if (game.game_over) {
                    dino_runner_reset_game();
                    return;
                }
                break;
        }
    } else {
        // No input detected - check if duck was released
        if (duck_key_held) {
            duck_released = true;
            duck_key_held = false;
        }
    }
    
    // Handle jump with buffering and coyote time
    if ((jump_pressed || game.dino.jump_buffer > 0) && !game.game_over) {
        bool can_jump = game.dino.on_ground || game.dino.coyote_timer > 0;
        
        if (can_jump) {
            game.dino.velocity_y = JUMP_POWER;
            game.dino.on_ground = false;
            game.dino.state = DINO_JUMPING;
            game.dino.jump_buffer = 0; // Consume the buffered jump
            game.dino.coyote_timer = 0; // Reset coyote time
            game.total_jumps++;
            dino_runner_play_sound("BOING!");
            
            // First jump achievement
            if (game.total_jumps == 1) {
                dino_runner_unlock_achievement(ACH_FIRST_JUMP);
            }
        }
    }
    
    // Handle ducking with improved responsiveness
    if (duck_pressed && game.dino.on_ground && !game.game_over) {
        game.dino.state = DINO_DUCKING;
        game.dino.duck_timer = DUCK_DURATION;
        game.dino.duck_held = true;
        game.total_ducks++;
    }
    
    // Handle duck release for instant stand-up
    if (duck_released && game.dino.state == DINO_DUCKING) {
        game.dino.duck_timer = 0;
        game.dino.duck_held = false;
        if (game.dino.on_ground) {
            game.dino.state = DINO_RUNNING;
        }
    }
}

void dino_runner_update_game(void) {
    dino_runner_update_dino();
    dino_runner_update_obstacles();
    dino_runner_update_clouds();
    dino_runner_check_collisions();
    dino_runner_update_day_night_cycle();
    
    // Update game speed (classic mode only)
    if (game.current_mode == MODE_CLASSIC) {
        float new_speed = 6 + (game.score / 100.0f);
        if (new_speed > MAX_GAME_SPEED) new_speed = MAX_GAME_SPEED;
        if (new_speed > game.game_speed) {
            game.game_speed = new_speed;
            dino_runner_play_sound("Speed increased!");
            
            // Speed demon achievement
            if (game.game_speed >= MAX_GAME_SPEED) {
                dino_runner_unlock_achievement(ACH_SPEED_DEMON);
            }
        }
    }
    
    // Ground animation
    game.ground_offset = (game.ground_offset + (int)game.game_speed) % 4;
    
    dino_runner_check_achievements();
}

void dino_runner_update_dino(void) {
    // Update animation timer
    game.dino.animation_timer++;
    
    // Store previous ground state for coyote time
    bool was_on_ground = game.dino.on_ground;
    
    // Handle duck mechanics with improved responsiveness
    if (game.dino.duck_timer > 0 && !game.dino.duck_held) {
        game.dino.duck_timer -= DUCK_SPEED;
        if (game.dino.duck_timer <= 0 && game.dino.on_ground) {
            game.dino.duck_timer = 0;
            game.dino.state = DINO_RUNNING;
        }
    }
    
    // Enhanced Physics System with improved responsiveness
    if (!game.dino.on_ground) {
        // Apply gravity with smoother acceleration
        game.dino.velocity_y += GRAVITY;
        
        // Apply terminal velocity limit
        if (game.dino.velocity_y > MAX_FALL_SPEED) {
            game.dino.velocity_y = MAX_FALL_SPEED;
        }
        
        // Update position with improved precision
        game.dino.y += game.dino.velocity_y;
        
        // Improved ground landing detection
        if (game.dino.y >= DINO_START_Y) {
            game.dino.y = DINO_START_Y;
            game.dino.velocity_y = 0;
            game.dino.on_ground = true;
            game.dino.last_ground_y = DINO_START_Y;
            
            // Determine landing state based on duck input
            if (game.dino.duck_held || game.dino.duck_timer > 0) {
                game.dino.state = DINO_DUCKING;
            } else {
                game.dino.state = DINO_RUNNING;
            }
            dino_runner_play_sound("*thud*");
        }
    } else {
        // Ground friction and state management
        game.dino.last_ground_y = game.dino.y;
        
        if (game.dino.velocity_y != 0) {
            game.dino.velocity_y *= GROUND_FRICTION;
        }
    }
    
    // Set coyote time when leaving ground
    if (was_on_ground && !game.dino.on_ground) {
        game.dino.coyote_timer = COYOTE_TIME;
    }
    
    // Enhanced running animation with variable speed
    if (game.dino.state == DINO_RUNNING) {
        int anim_speed = 8 - (int)(game.game_speed / 5); // Faster animation at higher speeds
        if (anim_speed < 4) anim_speed = 4; // Minimum animation speed
        
        if (game.dino.animation_timer % anim_speed == 0) {
            game.dino.animation_frame = (game.dino.animation_frame + 1) % 2;
        }
    }
    
    // Improved ducking animation with faster response
    if (game.dino.state == DINO_DUCKING) {
        int duck_anim_speed = 4; // Faster duck animation
        if (game.dino.animation_timer % duck_anim_speed == 0) {
            game.dino.animation_frame = (game.dino.animation_frame + 1) % 2;
        }
    }
}

void dino_runner_update_obstacles(void) {
    // Move existing obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game.obstacles[i].active) {
            game.obstacles[i].x -= game.game_speed;
            
            // Remove obstacles that are off screen
            if (game.obstacles[i].x < -10) {
                game.obstacles[i].active = false;
            }
            
            // Score points for passing obstacles
            if (!game.obstacles[i].scored && game.obstacles[i].x < DINO_X) {
                game.obstacles[i].scored = true;
                game.score += 10;
                game.obstacles_dodged++;
                
                // Score-based sound effects
                if (game.score % 100 == 0) {
                    dino_runner_play_sound("100 POINTS!");
                }
            }
        }
    }
    
    // Spawn new obstacles
    dino_runner_spawn_obstacle();
}

void dino_runner_spawn_obstacle(void) {
    static int spawn_timer = 0;
    static int last_obstacle_type = -1;
    static int pattern_counter = 0;
    
    spawn_timer--;
    
    if (spawn_timer <= 0) {
        // Find empty obstacle slot
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!game.obstacles[i].active) {
                game.obstacles[i].x = SCREEN_WIDTH;
                game.obstacles[i].active = true;
                game.obstacles[i].scored = false;
                
                // Enhanced obstacle selection based on score and patterns
                int obstacle_type;
                
                if (game.score < 100) {
                    // Early game: Simple obstacles only
                    obstacle_type = rand() % 3; // Small cactus, large cactus, rock
                } else if (game.score < 300) {
                    // Mid game: Add birds and double cactus
                    obstacle_type = rand() % 6; // Include basic bird types
                } else if (game.score < 600) {
                    // Advanced game: Add triple cactus and more variety
                    obstacle_type = rand() % 9; // More complex obstacles
                } else {
                    // Expert game: All obstacle types with strategic patterns
                    obstacle_type = rand() % OBSTACLE_COUNT;
                    
                    // Create challenging patterns at high scores
                    if (pattern_counter % 3 == 0) {
                        // Every third obstacle in sequence creates a pattern
                        if (last_obstacle_type == OBSTACLE_BIRD_HIGH) {
                            obstacle_type = OBSTACLE_SPIKE_TRAP; // Force duck after bird
                        } else if (last_obstacle_type == OBSTACLE_SPIKE_TRAP) {
                            obstacle_type = OBSTACLE_BIRD_LOW; // Force jump after duck
                        }
                    }
                }
                
                // Avoid consecutive identical obstacles for variety
                if (obstacle_type == last_obstacle_type && rand() % 3 == 0) {
                    obstacle_type = (obstacle_type + 1 + rand() % 3) % OBSTACLE_COUNT;
                }
                
                game.obstacles[i].type = obstacle_type;
                last_obstacle_type = obstacle_type;
                pattern_counter++;
                
                // Enhanced position setting based on obstacle type
                switch (game.obstacles[i].type) {
                    case OBSTACLE_BIRD_HIGH:
                    case OBSTACLE_BIRD_SWARM:
                        game.obstacles[i].y = GROUND_Y - 8;
                        break;
                    case OBSTACLE_BIRD_LOW:
                        game.obstacles[i].y = GROUND_Y - 4;
                        break;
                    case OBSTACLE_LOW_BRANCH:
                        game.obstacles[i].y = GROUND_Y - 6;
                        break;
                    case OBSTACLE_TALL_TREE:
                        game.obstacles[i].y = GROUND_Y - 1; // Taller obstacle
                        break;
                    default:
                        game.obstacles[i].y = GROUND_Y; // Ground level
                        break;
                }
                
                game.obstacles[i].width = obstacle_widths[game.obstacles[i].type];
                game.obstacles[i].height = obstacle_heights[game.obstacles[i].type];
                
                break;
            }
        }
        
        // Dynamic spawn timing based on score and game speed
        int base_spawn_time = 90 - (game.score / 20); // Gets faster with score
        base_spawn_time -= (int)(game.game_speed * 2); // Also affected by speed
        
        // Ensure minimum and maximum spawn intervals
        if (base_spawn_time < 25) base_spawn_time = 25; // Minimum gap
        if (base_spawn_time > 120) base_spawn_time = 120; // Maximum gap
        
        // Add controlled randomness for unpredictability
        int randomness = 15 + rand() % 25; // 15-40 frames of variance
        spawn_timer = base_spawn_time + randomness;
        
        // Special patterns at higher scores
        if (game.score > 1000 && rand() % 5 == 0) {
            spawn_timer /= 2; // Occasionally create tight spacing
        }
    }
}

void dino_runner_update_clouds(void) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (game.clouds[i].active) {
            game.clouds[i].x -= 1; // Clouds move slowly
            
            // Respawn clouds that move off screen
            if (game.clouds[i].x < -10) {
                game.clouds[i].x = SCREEN_WIDTH + rand() % 100;
                game.clouds[i].y = rand() % 8 + 2;
            }
        }
    }
}

void dino_runner_check_collisions(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game.obstacles[i].active) {
            float obs_x = game.obstacles[i].x;
            float obs_y = game.obstacles[i].y;
            float obs_w = game.obstacles[i].width;
            float obs_h = game.obstacles[i].height;
            
            // Enhanced collision box calculation for fairer gameplay
            float dino_x, dino_y, dino_w, dino_h;
            
            if (game.dino.state == DINO_DUCKING) {
                // Ducking hitbox - smaller and lower
                dino_x = game.dino.x + 2;  // More forgiving horizontal margin
                dino_y = game.dino.y + 2;  // Lower position when ducking
                dino_w = 2;                // Narrower when ducking
                dino_h = 1;                // Much shorter when ducking
            } else {
                // Normal/jumping hitbox - center-focused for fairness
                dino_x = game.dino.x + 1.5f;  // Center the hitbox better
                dino_y = game.dino.y + 1;     // Slight vertical margin
                dino_w = 2;                   // Reasonable width
                dino_h = 2;                   // Standard height
            }
            
            // Enhanced collision detection with pixel-perfect precision
            bool collision = (dino_x < obs_x + obs_w - 0.5f && 
                             dino_x + dino_w > obs_x + 0.5f && 
                             dino_y < obs_y + obs_h - 0.5f && 
                             dino_y + dino_h > obs_y + 0.5f);
            
            if (collision) {
                // Game over with enhanced feedback
                game.game_over = true;
                game.dino.state = DINO_DEAD;
                dino_runner_play_sound("ROAAAAR! *CRASH*");
                
                // Update high score with celebration
                if (game.score > game.high_score) {
                    game.high_score = game.score;
                    dino_runner_play_sound("NEW HIGH SCORE!");
                }
                
                dino_runner_game_over_screen();
                return;
            }
            
            // Enhanced near miss detection for excitement
            float distance = dino_runner_calculate_distance(dino_x, dino_y, obs_x, obs_y);
            if (distance < 5 && !game.obstacles[i].scored) {
                game.close_calls++;
            }
        }
    }
}

void dino_runner_update_day_night_cycle(void) {
    game.day_night_timer++;
    
    if (game.day_night_timer >= DAY_NIGHT_CYCLE) {
        game.day_night_timer = 0;
        game.time_of_day = (game.time_of_day + 1) % 4;
        
        if (game.time_of_day == TIME_NIGHT || game.time_of_day == TIME_DAY) {
            game.is_night = (game.time_of_day == TIME_NIGHT);
            dino_runner_play_sound(game.is_night ? "Night falls..." : "Dawn breaks...");
            
            // Night runner achievement
            if (game.is_night) {
                dino_runner_unlock_achievement(ACH_NIGHT_RUNNER);
            }
        }
    }
}

void dino_runner_clear_screen_buffer(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            screen_buffer[y][x] = (game.is_night) ? '.' : ' ';
        }
        screen_buffer[y][SCREEN_WIDTH] = '\0';
    }
}

void dino_runner_draw_to_buffer(int x, int y, const char* text) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    
    int len = strlen(text);
    for (int i = 0; i < len && x + i < SCREEN_WIDTH; i++) {
        if (x + i >= 0) {
            screen_buffer[y][x + i] = text[i];
        }
    }
}

void dino_runner_draw_clouds(void) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (game.clouds[i].active && game.clouds[i].x >= -5 && game.clouds[i].x < SCREEN_WIDTH + 5) {
            dino_runner_draw_to_buffer((int)game.clouds[i].x, (int)game.clouds[i].y, "~");
            dino_runner_draw_to_buffer((int)game.clouds[i].x + 1, (int)game.clouds[i].y, "~~");
            dino_runner_draw_to_buffer((int)game.clouds[i].x + 2, (int)game.clouds[i].y, "~");
        }
    }
}

void dino_runner_draw_dino(void) {
    char* sprite = NULL;
    
    switch (game.dino.state) {
        case DINO_RUNNING:
            sprite = dino_running_sprites[game.dino.animation_frame % 2];
            break;
        case DINO_JUMPING:
            sprite = dino_jumping_sprite;
            break;
        case DINO_DUCKING:
            sprite = dino_ducking_sprites[game.dino.animation_frame % 2];
            break;
        case DINO_DEAD:
            sprite = dino_dead_sprite;
            break;
    }
    
    // Create a local copy of the sprite since strtok modifies the string
    char sprite_copy[100];
    strncpy(sprite_copy, sprite, sizeof(sprite_copy) - 1);
    sprite_copy[sizeof(sprite_copy) - 1] = '\0';
    
    // Draw multi-line sprite
    char* line = strtok(sprite_copy, "\n");
    int line_y = (int)game.dino.y;
    
    while (line != NULL && line_y < SCREEN_HEIGHT) {
        dino_runner_draw_to_buffer((int)game.dino.x, line_y, line);
        line = strtok(NULL, "\n");
        line_y++;
    }
}

void dino_runner_draw_obstacles(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game.obstacles[i].active && game.obstacles[i].x >= -10 && game.obstacles[i].x < SCREEN_WIDTH + 10) {
            char* sprite = obstacle_sprites[game.obstacles[i].type];
            
            // Draw multi-line sprite
            char sprite_copy[64];
            strcpy(sprite_copy, sprite);
            
            char* line = strtok(sprite_copy, "\n");
            int line_y = (int)game.obstacles[i].y - game.obstacles[i].height + 1;
            
            while (line != NULL && line_y < SCREEN_HEIGHT) {
                dino_runner_draw_to_buffer((int)game.obstacles[i].x, line_y, line);
                line = strtok(NULL, "\n");
                line_y++;
            }
        }
    }
}

void dino_runner_draw_ground(void) {
    // Ground texture
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        int pattern_x = (x + game.ground_offset) % 4;
        if (pattern_x == 0) {
            dino_runner_draw_to_buffer(x, GROUND_Y + 1, (game.is_night) ? "#" : "=");
        } else {
            dino_runner_draw_to_buffer(x, GROUND_Y + 1, (game.is_night) ? "." : "_");
        }
    }
}

void dino_runner_draw_hud(void) {
    char hud_text[100];
    
    // Score
    sprintf(hud_text, "HI: %05d", game.high_score);
    dino_runner_draw_to_buffer(2, 1, hud_text);
    
    sprintf(hud_text, "SCORE: %05d", game.score);
    dino_runner_draw_to_buffer(15, 1, hud_text);
    
    // Speed meter
    int speed_bars = (int)((game.game_speed / MAX_GAME_SPEED) * 10);
    sprintf(hud_text, "SPEED: ");
    for (int i = 0; i < speed_bars; i++) {
        strcat(hud_text, "|");
    }
    dino_runner_draw_to_buffer(35, 1, hud_text);
    
    // Time of day indicator
    char* time_indicator[] = {"DAY", "SUNSET", "NIGHT", "SUNRISE"};
    dino_runner_draw_to_buffer(60, 1, time_indicator[game.time_of_day]);
    
    // Game mode
    char* mode_names[] = {"CLASSIC", "SPRINT", "MARATHON", "COURSE", "CUSTOM"};
    sprintf(hud_text, "Mode: %s", mode_names[game.current_mode]);
    dino_runner_draw_to_buffer(2, SCREEN_HEIGHT - 2, hud_text);
    
    // Controls
    if (game.game_over) {
        dino_runner_draw_to_buffer(25, SCREEN_HEIGHT - 2, "[SPACE] Jump [S] Duck [R] Restart [ESC] Exit");
    } else {
        dino_runner_draw_to_buffer(25, SCREEN_HEIGHT - 2, "[SPACE] Jump [S] Duck [ESC] Pause");
    }
}

void dino_runner_render_screen(void) {
    // Enhanced rendering with flicker reduction
    dino_runner_clear_screen_buffer();
    
    // Draw all game elements to buffer first
    dino_runner_draw_clouds();
    dino_runner_draw_obstacles();
    dino_runner_draw_dino();
    dino_runner_draw_ground();
    dino_runner_draw_hud();
    
    // Optimized screen rendering - minimal flicker using ANSI escape sequences
    printf("\033[H"); // Move cursor to home position instead of clearing
    dino_runner_display_header("CHROME DINO RUNNER");
    
    // Render buffer with improved output
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        printf("\033[%d;1H", y + 4); // Position cursor at specific line
        printf("%s", screen_buffer[y]);
        
        // Clear remaining characters on the line to prevent artifacts
        for (int clear_x = strlen(screen_buffer[y]); clear_x < SCREEN_WIDTH; clear_x++) {
            printf(" ");
        }
    }
    
    // Enhanced game over display with better positioning
    if (game.game_over) {
        printf("\033[%d;1H", SCREEN_HEIGHT + 5); // Position for game over screen
        printf("+===========================================+\n");
        printf("|              GAME OVER!                  |\n");
        printf("+===========================================+\n");
        printf("|  Final Score: %-24d   |\n", game.score);
        printf("|  Obstacles Dodged: %-18d   |\n", game.obstacles_dodged);
        printf("|  Play Time: %.1f seconds                |\n", game.play_time);
        printf("|                                           |\n");
        printf("|  [R] Restart  [ESC] Exit                  |\n");
        printf("+===========================================+\n");
    }
    
    // Flush output for smoother rendering
    fflush(stdout);
}

void dino_runner_game_over_screen(void) {
    // This is called when collision occurs
    // The actual game over display is in render_screen
}

void dino_runner_check_achievements(void) {
    // Score-based achievements
    if (game.score >= 100 && !game.achievements[ACH_SCORE_100].unlocked) {
        dino_runner_unlock_achievement(ACH_SCORE_100);
    }
    if (game.score >= 500 && !game.achievements[ACH_SCORE_500].unlocked) {
        dino_runner_unlock_achievement(ACH_SCORE_500);
    }
    if (game.score >= 1000 && !game.achievements[ACH_SCORE_1000].unlocked) {
        dino_runner_unlock_achievement(ACH_SCORE_1000);
    }
    if (game.score >= 2500 && !game.achievements[ACH_SCORE_2500].unlocked) {
        dino_runner_unlock_achievement(ACH_SCORE_2500);
    }
    if (game.score >= 5000 && !game.achievements[ACH_SCORE_5000].unlocked) {
        dino_runner_unlock_achievement(ACH_SCORE_5000);
    }
    if (game.score >= 10000 && !game.achievements[ACH_DINO_LEGEND].unlocked) {
        dino_runner_unlock_achievement(ACH_DINO_LEGEND);
    }
    
    // Duck master achievement
    if (game.total_ducks >= 50 && !game.achievements[ACH_DUCK_MASTER].unlocked) {
        dino_runner_unlock_achievement(ACH_DUCK_MASTER);
    }
    
    // Close calls achievement
    if (game.close_calls >= 10 && !game.achievements[ACH_CLOSE_CALLS].unlocked) {
        dino_runner_unlock_achievement(ACH_CLOSE_CALLS);
    }
    
    // Survival expert
    if (game.obstacles_dodged >= 100 && !game.achievements[ACH_SURVIVAL_EXPERT].unlocked) {
        dino_runner_unlock_achievement(ACH_SURVIVAL_EXPERT);
    }
    
    // Marathon runner (5 minutes = 300 seconds)
    if (game.play_time >= 300 && !game.achievements[ACH_MARATHON_RUNNER].unlocked) {
        dino_runner_unlock_achievement(ACH_MARATHON_RUNNER);
    }
    
    // Games played achievement
    if (game.games_played >= 50 && !game.achievements[ACH_EXTINCTION_AVOIDED].unlocked) {
        dino_runner_unlock_achievement(ACH_EXTINCTION_AVOIDED);
    }
}

void dino_runner_unlock_achievement(AchievementType type) {
    if (type < 0 || type >= ACH_COUNT) return;
    
    if (!game.achievements[type].unlocked) {
        game.achievements[type].unlocked = true;
        dino_runner_display_achievement_unlock(type);
    }
}

void dino_runner_display_achievement_unlock(AchievementType type) {
    if (type < 0 || type >= ACH_COUNT) return;
    
    Achievement* ach = &game.achievements[type];
    printf("\n");
    printf("+===========================================+\n");
    printf("|         ACHIEVEMENT UNLOCKED!            |\n");
    printf("+===========================================+\n");
    printf("|  %s%-37s   |\n", "* ", ach->name);
    printf("|  %-38s   |\n", ach->description);
    printf("|  Reward: %d points                      |\n", ach->points_reward);
    printf("+===========================================+\n");
    
    dino_runner_play_sound("ACHIEVEMENT UNLOCKED!");
    SLEEP_MS(2000);
}

void dino_runner_how_to_play(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("HOW TO PLAY");
    
    printf("+===========================================+\n");
    printf("|            CHROME DINO RUNNER            |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Help the T-Rex survive as long as       |\n");
    printf("|  possible by avoiding obstacles!          |\n");
    printf("|                                           |\n");
    printf("|  CONTROLS:                                |\n");
    printf("|  [SPACE] - Jump over obstacles            |\n");
    printf("|  [S]     - Duck under flying birds       |\n");
    printf("|  [ESC]   - Pause game or exit             |\n");
    printf("|                                           |\n");
    printf("|  OBSTACLES:                               |\n");
    printf("|  | |  - Cactus (jump over)                |\n");
    printf("|  ###  - Rocks (jump over)                 |\n");
    printf("|  ^v^  - Birds (duck or jump)              |\n");
    printf("|                                           |\n");
    printf("|  FEATURES:                                |\n");
    printf("|  * Day/night cycle changes scenery       |\n");
    printf("|  * Speed increases as you progress        |\n");
    printf("|  * Earn achievements for milestones       |\n");
    printf("|  * Multiple game modes available          |\n");
    printf("|                                           |\n");
    printf("|  TIPS:                                    |\n");
    printf("|  * Time your jumps carefully             |\n");
    printf("|  * Duck under high-flying birds          |\n");
    printf("|  * Watch for double obstacles             |\n");
    printf("|  * Practice makes perfect!                |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to return to menu...");
    GETCH();
}

void dino_runner_display_statistics(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("STATISTICS");
    
    float average_score = (game.games_played > 0) ? (float)game.score / game.games_played : 0;
    int achievements_unlocked = 0;
    
    for (int i = 0; i < ACH_COUNT; i++) {
        if (game.achievements[i].unlocked) {
            achievements_unlocked++;
        }
    }
    
    printf("+===========================================+\n");
    printf("|              PLAYER STATS                |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  High Score: %-25d   |\n", game.high_score);
    printf("|  Games Played: %-22d   |\n", game.games_played);
    printf("|  Total Playtime: %.1f minutes           |\n", game.play_time / 60.0f);
    printf("|  Average Score: %.1f                    |\n", average_score);
    printf("|                                           |\n");
    printf("|  ACTIONS:                                 |\n");
    printf("|  Total Jumps: %-23d   |\n", game.total_jumps);
    printf("|  Total Ducks: %-23d   |\n", game.total_ducks);
    printf("|  Obstacles Dodged: %-18d   |\n", game.obstacles_dodged);
    printf("|  Close Calls: %-23d   |\n", game.close_calls);
    printf("|                                           |\n");
    printf("|  PROGRESS:                                |\n");
    printf("|  Achievements: %d/%d unlocked            |\n", achievements_unlocked, ACH_COUNT);
    printf("|  Completion: %.1f%%                     |\n", (float)achievements_unlocked / ACH_COUNT * 100);
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\nPress any key to return to menu...");
    GETCH();
}

void dino_runner_display_achievements(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("ACHIEVEMENTS");
    
    printf("+===========================================+\n");
    printf("|              ACHIEVEMENTS                |\n");
    printf("+===========================================+\n");
    
    for (int i = 0; i < ACH_COUNT; i++) {
        Achievement* ach = &game.achievements[i];
        char status = ach->unlocked ? '*' : ' ';
        printf("| [%c] %-35s   |\n", status, ach->name);
        printf("|     %-35s   |\n", ach->description);
        printf("|                                           |\n");
    }
    
    printf("+===========================================+\n");
    printf("\n[*] = Unlocked    [ ] = Locked\n");
    printf("\nPress any key to return to menu...");
    GETCH();
}

void dino_runner_settings_menu(void) {
    CLEAR_SCREEN();
    dino_runner_display_header("SETTINGS");
    
    printf("+===========================================+\n");
    printf("|                SETTINGS                  |\n");
    printf("+===========================================+\n");
    printf("|                                           |\n");
    printf("|  1. Reset Statistics                      |\n");
    printf("|  2. Reset Achievements                    |\n");
    printf("|  3. Reset All Data                        |\n");
    printf("|  4. Export Statistics                     |\n");
    printf("|  5. Game Information                      |\n");
    printf("|                                           |\n");
    printf("|  0. Back to Menu                          |\n");
    printf("|                                           |\n");
    printf("+===========================================+\n");
    printf("\n> Enter choice (0-5): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        choice = -1;
    }
    
    switch (choice) {
        case 1:
            printf("\nReset statistics? (y/N): ");
            char confirm;
            scanf(" %c", &confirm);
            if (confirm == 'y' || confirm == 'Y') {
                // Reset stats
                game.high_score = 0;
                game.games_played = 0;
                game.total_jumps = 0;
                game.total_ducks = 0;
                game.obstacles_dodged = 0;
                game.close_calls = 0;
                game.play_time = 0;
                printf("Statistics reset!");
                SLEEP_MS(1000);
            }
            break;
            
        case 5:
            CLEAR_SCREEN();
            dino_runner_display_header("GAME INFORMATION");
            printf("+===========================================+\n");
            printf("|          CHROME DINO RUNNER              |\n");
            printf("+===========================================+\n");
            printf("|                                           |\n");
            printf("|  Version: 1.0 Enhanced Edition           |\n");
            printf("|  Part of: CLI Games Pack v1.8            |\n");
            printf("|                                           |\n");
            printf("|  Features:                                |\n");
            printf("|  * 5 Game modes                           |\n");
            printf("|  * 15+ Achievements                       |\n");
            printf("|  * Day/night cycle                        |\n");
            printf("|  * Progressive difficulty                 |\n");
            printf("|  * Statistics tracking                    |\n");
            printf("|  * Multiple obstacle types                |\n");
            printf("|                                           |\n");
            printf("|  Based on the Chrome offline game         |\n");
            printf("|  Enhanced for CLI Games Pack              |\n");
            printf("|                                           |\n");
            printf("+===========================================+\n");
            printf("\nPress any key to return...");
            GETCH();
            break;
            
        default:
            break;
    }
}

void dino_runner_display_header(const char* title) {
    printf("================================================\n");
    printf("|              %-26s |\n", title);
    printf("================================================\n");
}

void dino_runner_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void dino_runner_play_sound(const char* sound) {
    printf("    [SFX] %s\n", sound);
    fflush(stdout);
}

void dino_runner_save_statistics(void) {
    // Simple file save (basic implementation)
    FILE* file = fopen("dino_stats.dat", "wb");
    if (file) {
        fwrite(&game.high_score, sizeof(int), 1, file);
        fwrite(&game.games_played, sizeof(int), 1, file);
        fwrite(&game.total_jumps, sizeof(int), 1, file);
        fwrite(&game.total_ducks, sizeof(int), 1, file);
        fwrite(&game.obstacles_dodged, sizeof(int), 1, file);
        fwrite(&game.close_calls, sizeof(int), 1, file);
        fwrite(&game.achievements, sizeof(Achievement), ACH_COUNT, file);
        fclose(file);
    }
}

void dino_runner_load_statistics(void) {
    FILE* file = fopen("dino_stats.dat", "rb");
    if (file) {
        fread(&game.high_score, sizeof(int), 1, file);
        fread(&game.games_played, sizeof(int), 1, file);
        fread(&game.total_jumps, sizeof(int), 1, file);
        fread(&game.total_ducks, sizeof(int), 1, file);
        fread(&game.obstacles_dodged, sizeof(int), 1, file);
        fread(&game.close_calls, sizeof(int), 1, file);
        
        // Load achievements (but preserve the structure)
        Achievement saved_achievements[ACH_COUNT];
        fread(saved_achievements, sizeof(Achievement), ACH_COUNT, file);
        
        // Only copy the unlocked status
        for (int i = 0; i < ACH_COUNT; i++) {
            game.achievements[i].unlocked = saved_achievements[i].unlocked;
        }
        
        fclose(file);
    }
}

float dino_runner_calculate_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

void play_dino_runner(void) {
    dino_runner_init_game();
    game.current_mode = MODE_CLASSIC;
    dino_runner_classic_mode();
}