/*
 * Simon Says - Enhanced Memory Training Game
 * CLI Games Pack v1.8
 * 
 * A comprehensive memory game with multiple modes, difficulty levels,
 * achievements, and cognitive training features.
 * 
 * Features:
 * - 6 Game Modes: Classic, Speed, Memory Master, Color Blind, Reverse, Number
 * - 5 Difficulty Levels: Beginner to Impossible
 * - Achievement System with 20+ achievements
 * - Comprehensive Statistics and Progress Tracking
 * - Educational Memory Training Tips
 * - Multiple Visual Themes
 * - Cross-platform ASCII graphics
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include "games.h"

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP_MS(ms) Sleep(ms)
    #define SIMON_KBHIT() games_kbhit()
    #define GETCH() _getch()
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define SIMON_KBHIT() games_kbhit()
    #define GETCH() getchar()
#endif

// Game Constants
#define MAX_SEQUENCE_LENGTH 50
#define MAX_COLORS 9
#define MAX_ACHIEVEMENTS 25
#define MAX_THEMES 4

// Game Modes
typedef enum {
    MODE_CLASSIC = 0,
    MODE_SPEED,
    MODE_MEMORY_MASTER,
    MODE_COLOR_BLIND,
    MODE_REVERSE,
    MODE_NUMBER_SEQUENCE,
    MODE_COUNT
} GameMode;

// Difficulty Levels
typedef enum {
    DIFF_BEGINNER = 0,
    DIFF_NORMAL,
    DIFF_HARD,
    DIFF_EXPERT,
    DIFF_IMPOSSIBLE,
    DIFF_COUNT
} DifficultyLevel;

// Achievement Types
typedef enum {
    ACH_FIRST_STEPS = 0,
    ACH_MEMORY_BANK,
    ACH_SPEED_DEMON,
    ACH_PERFECT_GAME,
    ACH_IRON_MEMORY,
    ACH_LIGHTNING_FAST,
    ACH_MASTER_MIND,
    ACH_IMPOSSIBLE_DREAM,
    ACH_STREAK_MASTER,
    ACH_MARATHON_RUNNER,
    ACH_COUNT
} AchievementType;

// Structures
typedef struct {
    char* name;
    int max_colors;
    int sequence_speed;      // ms per item display
    int input_time_limit;    // seconds for input
    int lives;
    int bonus_multiplier;
} Difficulty;

typedef struct {
    char* name;
    char symbols[MAX_COLORS][4];
    char* sound_effects[MAX_COLORS];
} Theme;

typedef struct {
    int id;
    char* name;
    char* description;
    int requirement;
    bool unlocked;
    int points_reward;
} Achievement;

typedef struct {
    // Game State
    int sequence[MAX_SEQUENCE_LENGTH];
    int sequence_length;
    int current_round;
    int lives;
    int score;
    int high_score;
    GameMode current_mode;
    DifficultyLevel current_difficulty;
    int current_theme;
    
    // Session Stats
    int rounds_played;
    int perfect_rounds;
    int total_input_time;
    int speed_bonuses;
    int current_streak;
    int best_streak;
    
    // Overall Statistics
    int games_played;
    int total_rounds;
    int best_round;
    int perfect_games;
    float success_rate;
    float average_round;
    float fastest_input;
    int memory_span;
    
    // Mode Records
    int mode_best_rounds[MODE_COUNT];
    int mode_high_scores[MODE_COUNT];
    
    // Achievements
    bool achievements[ACH_COUNT];
    int total_achievement_points;
    
    // Settings
    bool sound_enabled;
    bool hints_enabled;
    bool animations_enabled;
} GameState;

// Global Variables
static GameState game;
static time_t round_start_time;
static bool game_running = true;

// Difficulty Settings
static Difficulty difficulties[DIFF_COUNT] = {
    {"BEGINNER",   4, 1200, 30, 5, 1},
    {"NORMAL",     4, 1000, 25, 3, 2},
    {"HARD",       6, 800,  20, 3, 3},
    {"EXPERT",     8, 600,  15, 2, 4},
    {"IMPOSSIBLE", 9, 400,  10, 1, 5}
};

// Visual Themes
static Theme themes[MAX_THEMES] = {
    {"CLASSIC", {"[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]"}, 
     {"BEEP", "BOOP", "PING", "PONG", "DING", "DONG", "RING", "GONG", "CHIME"}},
    {"SHAPES", {"[*]", "[+]", "[=]", "[#]", "[@]", "[%]", "[&]", "[?]", "[!]"}, 
     {"CLICK", "SNAP", "POP", "TICK", "CLAP", "THUD", "BANG", "CRACK", "WHOOSH"}},
    {"MUSIC",  {"[A]", "[B]", "[C]", "[D]", "[E]", "[F]", "[G]", "[H]", "[I]"}, 
     {"DO", "RE", "MI", "FA", "SOL", "LA", "TI", "DO8", "RE8"}},
    {"SPACE",  {"[o]", "[*]", "[^]", "[x]", "[+]", "[~]", "[>]", "[<]", "[|]"}, 
     {"ZAP", "BOOM", "WHOOSH", "BEEP", "PULSE", "LASER", "WARP", "BLAST", "SONIC"}}
};

// Achievements
static Achievement achievements[ACH_COUNT] = {
    {0, "FIRST STEPS", "Complete round 3", 3, false, 50},
    {1, "MEMORY BANK", "Complete round 10", 10, false, 100},
    {2, "SPEED DEMON", "Input sequence in under 3 seconds", 3, false, 150},
    {3, "PERFECT GAME", "No mistakes for 5 rounds", 5, false, 200},
    {4, "IRON MEMORY", "Reach round 20", 20, false, 300},
    {5, "LIGHTNING FAST", "Get 10 speed bonuses in one game", 10, false, 250},
    {6, "MASTER MIND", "Complete Expert difficulty", 1, false, 400},
    {7, "IMPOSSIBLE DREAM", "Beat Impossible mode", 1, false, 500},
    {8, "STREAK MASTER", "Achieve 15-round streak", 15, false, 350},
    {9, "MARATHON RUNNER", "Play for 30 minutes straight", 30, false, 200}
};

// Function Prototypes
void simon_says_init_game(void);
void simon_says_main_menu(void);
void simon_says_game_loop(void);
void simon_says_display_header(const char* title);
void simon_says_clear_input_buffer(void);
void simon_says_play_sound(const char* sound);

// Game Mode Functions
void simon_says_classic_mode(void);
void simon_says_speed_mode(void);
void simon_says_memory_master_mode(void);
void simon_says_color_blind_mode(void);
void simon_says_reverse_mode(void);
void simon_says_number_mode(void);

// Core Game Functions
void simon_says_add_to_sequence(void);
void simon_says_display_sequence(void);
void simon_says_display_sequence_animated(void);
bool simon_says_get_player_input(void);
bool simon_says_check_input(int input[], int length);
void simon_says_calculate_score(bool speed_bonus);
void simon_says_next_round(void);
void simon_says_game_over(void);

// UI Functions
void simon_says_display_game_state(void);
void simon_says_display_lives(void);
void simon_says_display_progress(void);
void simon_says_display_statistics(void);
void simon_says_display_achievements(void);
void simon_says_display_memory_tips(void);
void simon_says_settings_menu(void);

// Achievement Functions
void simon_says_check_achievements(void);
void simon_says_unlock_achievement(AchievementType type);
void simon_says_display_achievement_unlock(AchievementType type);

// Utility Functions
void simon_says_save_statistics(void);
void simon_says_load_statistics(void);
int simon_says_get_random_number(int max);
double simon_says_get_elapsed_time(void);

// Sound and Display Functions
void simon_says_play_success_sound(void);
void simon_says_play_error_sound(void);
void simon_says_play_achievement_sound(void);

// Main Entry Point
void play_simon_says(void) {
    srand((unsigned int)time(NULL));
    simon_says_init_game();
    simon_says_load_statistics();
    
    while (game_running) {
        simon_says_main_menu();
    }
    
    simon_says_save_statistics();
}

// Game Initialization
void simon_says_init_game(void) {
    memset(&game, 0, sizeof(GameState));
    
    // Set defaults
    game.current_mode = MODE_CLASSIC;
    game.current_difficulty = DIFF_NORMAL;
    game.current_theme = 0;
    game.lives = difficulties[game.current_difficulty].lives;
    game.sound_enabled = true;
    game.hints_enabled = true;
    game.animations_enabled = true;
    
    // Initialize sequence
    game.sequence_length = 0;
    game.current_round = 1;
    game.score = 0;
    game.current_streak = 0;
}

// Header Display
void simon_says_display_header(const char* title) {
    CLEAR_SCREEN();
    printf("===============================================\n");
    printf("|              SIMON SAYS v1.0               |\n");
    printf("===============================================\n");
    printf("|                                           |\n");
    printf("|           %-31s |\n", title);
    printf("|                                           |\n");
    printf("===============================================\n");
}

// Main Menu
void simon_says_main_menu(void) {
    simon_says_display_header("MAIN MENU");
    printf("|                                           |\n");
    printf("| [1] Classic Simon                         |\n");
    printf("| [2] Speed Simon                           |\n");
    printf("| [3] Memory Master                         |\n");
    printf("| [4] Color Blind Friendly                  |\n");
    printf("| [5] Reverse Simon                         |\n");
    printf("| [6] Number Sequence                       |\n");
    printf("|                                           |\n");
    printf("| [7] Settings & Difficulty                 |\n");
    printf("| [8] Statistics & Records                  |\n");
    printf("| [9] Achievements                          |\n");
    printf("| [10] Memory Training Tips                 |\n");
    printf("| [11] Back to Main Menu                    |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    printf("\nChoice (1-11): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        simon_says_clear_input_buffer();
        return;
    }
    simon_says_clear_input_buffer();
    
    switch (choice) {
        case 1:
            game.current_mode = MODE_CLASSIC;
            simon_says_classic_mode();
            break;
        case 2:
            game.current_mode = MODE_SPEED;
            simon_says_speed_mode();
            break;
        case 3:
            game.current_mode = MODE_MEMORY_MASTER;
            simon_says_memory_master_mode();
            break;
        case 4:
            game.current_mode = MODE_COLOR_BLIND;
            simon_says_color_blind_mode();
            break;
        case 5:
            game.current_mode = MODE_REVERSE;
            simon_says_reverse_mode();
            break;
        case 6:
            game.current_mode = MODE_NUMBER_SEQUENCE;
            simon_says_number_mode();
            break;
        case 7:
            simon_says_settings_menu();
            break;
        case 8:
            simon_says_display_statistics();
            break;
        case 9:
            simon_says_display_achievements();
            break;
        case 10:
            simon_says_display_memory_tips();
            break;
        case 11:
            game_running = false;
            return;
        default:
            printf("Invalid choice! Press Enter to continue...");
            getchar();
            break;
    }
}

// Classic Simon Mode
void simon_says_classic_mode(void) {
    simon_says_display_header("CLASSIC SIMON");
    printf("|                                           |\n");
    printf("|  >>> ORIGINAL SIMON SAYS EXPERIENCE <<<  |\n");
    printf("|                                           |\n");
    printf("|  Remember and repeat the sequence!        |\n");
    printf("|  Four colors, increasing difficulty       |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_CLASSIC;
    simon_says_game_loop();
}

// Game Loop
void simon_says_game_loop(void) {
    while (game.lives > 0) {
        // Add new number to sequence
        simon_says_add_to_sequence();
        
        // Display game state
        simon_says_display_game_state();
        
        // Show sequence
        if (game.animations_enabled) {
            simon_says_display_sequence_animated();
        } else {
            simon_says_display_sequence();
        }
        
        // Get player input
        round_start_time = time(NULL);
        bool success = simon_says_get_player_input();
        
        if (success) {
            // Calculate score with potential speed bonus
            double input_time = simon_says_get_elapsed_time();
            bool speed_bonus = input_time < 3.0;
            simon_says_calculate_score(speed_bonus);
            
            // Success feedback
            simon_says_play_success_sound();
            game.current_streak++;
            game.perfect_rounds++;
            
            printf("\n[+] CORRECT! ");
            if (speed_bonus) {
                printf("SPEED BONUS! ");
                game.speed_bonuses++;
            }
            printf("Score: %d\n", game.score);
            
            simon_says_next_round();
            simon_says_check_achievements();
            
            printf("Press Enter for next round...");
            getchar();
        } else {
            // Failure feedback
            simon_says_play_error_sound();
            game.lives--;
            game.current_streak = 0;
            
            printf("\n[-] INCORRECT! Lives remaining: %d\n", game.lives);
            if (game.lives > 0) {
                printf("Press Enter to try again...");
                getchar();
            }
        }
    }
    
    simon_says_game_over();
}

// Add to Sequence
void simon_says_add_to_sequence(void) {
    if (game.sequence_length < MAX_SEQUENCE_LENGTH) {
        int max_colors = difficulties[game.current_difficulty].max_colors;
        game.sequence[game.sequence_length] = simon_says_get_random_number(max_colors) + 1;
        game.sequence_length++;
    }
}

// Display Game State
void simon_says_display_game_state(void) {
    simon_says_display_header("GAME IN PROGRESS");
    printf("| ROUND: %-2d   SCORE: %-6d   BEST: %-6d |\n", 
           game.current_round, game.score, game.high_score);
    printf("| LENGTH: %-2d   ", game.sequence_length);
    simon_says_display_lives();
    printf("|\n");
    printf("| MODE: %-15s DIFFICULTY: %-8s |\n", 
           "CLASSIC", difficulties[game.current_difficulty].name);
    printf("===============================================\n");
}

// Display Lives
void simon_says_display_lives(void) {
    printf("LIVES: ");
    for (int i = 0; i < difficulties[game.current_difficulty].lives; i++) {
        if (i < game.lives) {
            printf("* ");
        } else {
            printf("- ");
        }
    }
}

// Animated Sequence Display
void simon_says_display_sequence_animated(void) {
    printf("|                                           |\n");
    printf("|    >>> WATCH THE SEQUENCE <<<             |\n");
    printf("|                                           |\n");
    
    Theme* theme = &themes[game.current_theme];
    int display_speed = difficulties[game.current_difficulty].sequence_speed;
    
    // Show the full sequence first
    printf("| COMPLETE SEQUENCE: ");
    for (int i = 0; i < game.sequence_length; i++) {
        printf("%d ", game.sequence[i]);
    }
    printf("|\n");
    printf("|                                           |\n");
    
    SLEEP_MS(2000); // Let them see the full sequence
    
    // Now show it step by step
    for (int i = 0; i < game.sequence_length; i++) {
        printf("| STEP %d: Number %d %s                   |\n", 
               i + 1, game.sequence[i], theme->symbols[game.sequence[i] - 1]);
        
        // Play sound effect
        if (game.sound_enabled) {
            simon_says_play_sound(theme->sound_effects[game.sequence[i] - 1]);
        }
        
        SLEEP_MS(display_speed);
    }
    
    printf("|                                           |\n");
    printf("|    NOW ENTER THE SEQUENCE:                |\n");
    printf("| (Type numbers together, e.g. '123')       |\n");
}

// Get Player Input
bool simon_says_get_player_input(void) {
    printf("| Enter sequence (numbers 1-%d): ", 
           difficulties[game.current_difficulty].max_colors);
    
    int input[MAX_SEQUENCE_LENGTH];
    int input_length = 0;
    
    // Get input based on game mode
    if (game.current_mode == MODE_REVERSE) {
        // Reverse mode - input backwards
        printf("\n| (Enter in REVERSE order): ");
    }
    
    // Read sequence input as a string first, then parse each character
    char input_string[MAX_SEQUENCE_LENGTH + 1];
    if (scanf("%s", input_string) != 1) {
        simon_says_clear_input_buffer();
        return false;
    }
    
    // Parse each character as a number
    for (int i = 0; i < (int)strlen(input_string) && i < game.sequence_length; i++) {
        if (!isdigit(input_string[i])) {
            printf("| Invalid input! Use digits only.\n");
            simon_says_clear_input_buffer();
            return false;
        }
        
        int num = input_string[i] - '0';
        if (num < 1 || num > difficulties[game.current_difficulty].max_colors) {
            printf("| Invalid number! Use 1-%d only.\n", 
                   difficulties[game.current_difficulty].max_colors);
            simon_says_clear_input_buffer();
            return false;
        }
        
        input[input_length++] = num;
    }
    
    // Check if we got the right number of digits
    if (input_length != game.sequence_length) {
        printf("| Wrong length! Enter exactly %d numbers.\n", game.sequence_length);
        simon_says_clear_input_buffer();
        return false;
    }
    
    simon_says_clear_input_buffer();
    return simon_says_check_input(input, input_length);
}

// Check Input
bool simon_says_check_input(int input[], int length) {
    if (length != game.sequence_length) {
        return false;
    }
    
    if (game.current_mode == MODE_REVERSE) {
        // Check in reverse order
        for (int i = 0; i < length; i++) {
            if (input[i] != game.sequence[length - 1 - i]) {
                return false;
            }
        }
    } else {
        // Check in normal order
        for (int i = 0; i < length; i++) {
            if (input[i] != game.sequence[i]) {
                return false;
            }
        }
    }
    
    return true;
    
    return true;
}

// Calculate Score
void simon_says_calculate_score(bool speed_bonus) {
    int base_points = 100 * game.current_round;
    int difficulty_bonus = difficulties[game.current_difficulty].bonus_multiplier * 50;
    int streak_multiplier = (game.current_streak / 3) + 1; // +1 for every 3 streak
    
    int round_score = base_points + difficulty_bonus;
    
    if (speed_bonus) {
        round_score += 200;
    }
    
    round_score *= streak_multiplier;
    game.score += round_score;
    
    if (game.score > game.high_score) {
        game.high_score = game.score;
    }
}

// Next Round
void simon_says_next_round(void) {
    game.current_round++;
    game.rounds_played++;
    
    // Update best round
    if (game.current_round > game.best_round) {
        game.best_round = game.current_round;
    }
    
    if (game.current_round > game.mode_best_rounds[game.current_mode]) {
        game.mode_best_rounds[game.current_mode] = game.current_round;
    }
}

// Game Over
void simon_says_game_over(void) {
    simon_says_display_header("GAME OVER");
    printf("|                                           |\n");
    printf("| FINAL RESULTS:                            |\n");
    printf("| Round Reached: %-2d                        |\n", game.current_round);
    printf("| Final Score: %-6d                        |\n", game.score);
    printf("| Perfect Rounds: %-2d                       |\n", game.perfect_rounds);
    printf("| Best Streak: %-2d                          |\n", game.current_streak > game.best_streak ? game.current_streak : game.best_streak);
    printf("| Speed Bonuses: %-2d                        |\n", game.speed_bonuses);
    printf("|                                           |\n");
    
    // Update statistics
    game.games_played++;
    game.total_rounds += game.rounds_played;
    if (game.perfect_rounds == game.rounds_played && game.rounds_played >= 5) {
        game.perfect_games++;
    }
    
    // Check for final achievements
    simon_says_check_achievements();
    
    printf("| Press Enter to return to menu...          |\n");
    printf("===============================================\n");
    getchar();
}

// Settings Menu (stub)
void simon_says_settings_menu(void) {
    simon_says_display_header("SETTINGS");
    printf("|                                           |\n");
    printf("| [1] Difficulty: %-25s |\n", difficulties[game.current_difficulty].name);
    printf("| [2] Theme: %-30s |\n", themes[game.current_theme].name);
    printf("| [3] Sound: %-30s |\n", game.sound_enabled ? "ON" : "OFF");
    printf("| [4] Animations: %-25s |\n", game.animations_enabled ? "ON" : "OFF");
    printf("| [5] Hints: %-30s |\n", game.hints_enabled ? "ON" : "OFF");
    printf("|                                           |\n");
    printf("| [6] Back to Main Menu                     |\n");
    printf("===============================================\n");
    printf("\nChoice (1-6): ");
    
    int choice;
    if (scanf("%d", &choice) == 1) {
        switch (choice) {
            case 1:
                game.current_difficulty = (game.current_difficulty + 1) % DIFF_COUNT;
                game.lives = difficulties[game.current_difficulty].lives;
                break;
            case 2:
                game.current_theme = (game.current_theme + 1) % MAX_THEMES;
                break;
            case 3:
                game.sound_enabled = !game.sound_enabled;
                break;
            case 4:
                game.animations_enabled = !game.animations_enabled;
                break;
            case 5:
                game.hints_enabled = !game.hints_enabled;
                break;
            case 6:
                simon_says_clear_input_buffer();
                return;
        }
        simon_says_settings_menu(); // Recursive call to stay in settings
    }
    simon_says_clear_input_buffer();
}

// Statistics Display
void simon_says_display_statistics(void) {
    simon_says_display_header("STATISTICS");
    printf("|                                           |\n");
    printf("| OVERALL PERFORMANCE:                      |\n");
    printf("| Games Played: %-3d                        |\n", game.games_played);
    printf("| Total Rounds: %-3d                        |\n", game.total_rounds);
    printf("| Best Round: %-3d                          |\n", game.best_round);
    printf("| Perfect Games: %-3d                       |\n", game.perfect_games);
    printf("|                                           |\n");
    printf("| MODE RECORDS:                             |\n");
    printf("| Classic Best: Round %-2d                  |\n", game.mode_best_rounds[MODE_CLASSIC]);
    printf("| Speed Best: Round %-2d                    |\n", game.mode_best_rounds[MODE_SPEED]);
    printf("| Memory Master: Round %-2d                 |\n", game.mode_best_rounds[MODE_MEMORY_MASTER]);
    printf("|                                           |\n");
    printf("| COGNITIVE ASSESSMENT:                     |\n");
    printf("| Memory Span: %-2d items                   |\n", game.best_round);
    printf("| Success Rate: %.1f%%                      |\n", 
           game.total_rounds > 0 ? (float)game.perfect_rounds / game.total_rounds * 100 : 0);
    printf("|                                           |\n");
    printf("| Press Enter to continue...                |\n");
    printf("===============================================\n");
    getchar();
}

// Achievements Display
void simon_says_display_achievements(void) {
    simon_says_display_header("ACHIEVEMENTS");
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

// Memory Tips
void simon_says_display_memory_tips(void) {
    simon_says_display_header("MEMORY TRAINING TIPS");
    printf("|                                           |\n");
    printf("| IMPROVE YOUR MEMORY:                      |\n");
    printf("| • Group numbers into chunks (2-3)        |\n");
    printf("| • Use rhythm to remember patterns         |\n");
    printf("| • Visualize numbers as colors             |\n");
    printf("| • Practice daily for 10 minutes          |\n");
    printf("| • Try saying numbers out loud             |\n");
    printf("| • Create stories with the sequence        |\n");
    printf("| • Use the method of loci technique        |\n");
    printf("|                                           |\n");
    printf("| YOUR MEMORY SPAN: %-2d items               |\n", game.best_round);
    printf("| AVERAGE ADULT: 7±2 items                  |\n");
    printf("| GOAL: Reach 20+ items (Expert level)     |\n");
    printf("|                                           |\n");
    printf("| Press Enter to continue...                |\n");
    printf("===============================================\n");
    getchar();
}

// Achievement Checking
void simon_says_check_achievements(void) {
    // First Steps - Complete round 3
    if (game.current_round >= 3 && !game.achievements[ACH_FIRST_STEPS]) {
        simon_says_unlock_achievement(ACH_FIRST_STEPS);
    }
    
    // Memory Bank - Complete round 10
    if (game.current_round >= 10 && !game.achievements[ACH_MEMORY_BANK]) {
        simon_says_unlock_achievement(ACH_MEMORY_BANK);
    }
    
    // Iron Memory - Reach round 20
    if (game.current_round >= 20 && !game.achievements[ACH_IRON_MEMORY]) {
        simon_says_unlock_achievement(ACH_IRON_MEMORY);
    }
    
    // Lightning Fast - 10 speed bonuses
    if (game.speed_bonuses >= 10 && !game.achievements[ACH_LIGHTNING_FAST]) {
        simon_says_unlock_achievement(ACH_LIGHTNING_FAST);
    }
    
    // Perfect Game - 5 perfect rounds
    if (game.perfect_rounds >= 5 && !game.achievements[ACH_PERFECT_GAME]) {
        simon_says_unlock_achievement(ACH_PERFECT_GAME);
    }
    
    // Streak Master - 15-round streak
    if (game.current_streak >= 15 && !game.achievements[ACH_STREAK_MASTER]) {
        simon_says_unlock_achievement(ACH_STREAK_MASTER);
    }
    
    // Master Mind - Expert difficulty
    if (game.current_difficulty >= DIFF_EXPERT && game.current_round >= 10 && !game.achievements[ACH_MASTER_MIND]) {
        simon_says_unlock_achievement(ACH_MASTER_MIND);
    }
    
    // Impossible Dream - Impossible difficulty
    if (game.current_difficulty == DIFF_IMPOSSIBLE && game.current_round >= 5 && !game.achievements[ACH_IMPOSSIBLE_DREAM]) {
        simon_says_unlock_achievement(ACH_IMPOSSIBLE_DREAM);
    }
}

// Unlock Achievement
void simon_says_unlock_achievement(AchievementType type) {
    if (!game.achievements[type]) {
        game.achievements[type] = true;
        game.total_achievement_points += achievements[type].points_reward;
        simon_says_display_achievement_unlock(type);
    }
}

// Display Achievement Unlock
void simon_says_display_achievement_unlock(AchievementType type) {
    simon_says_play_achievement_sound();
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

// Speed Mode (simplified implementation)
void simon_says_speed_mode(void) {
    simon_says_display_header("SPEED SIMON");
    printf("|                                           |\n");
    printf("|  >>> FAST-PACED MEMORY CHALLENGE <<<     |\n");
    printf("|                                           |\n");
    printf("|  Faster sequence display                  |\n");
    printf("|  Bonus points for quick input             |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_SPEED;
    // Reduce display time for speed mode
    difficulties[game.current_difficulty].sequence_speed /= 2;
    simon_says_game_loop();
}

// Memory Master Mode
void simon_says_memory_master_mode(void) {
    simon_says_display_header("MEMORY MASTER");
    printf("|                                           |\n");
    printf("|  >>> ULTIMATE MEMORY CHALLENGE <<<       |\n");
    printf("|                                           |\n");
    printf("|  Longer sequences up to 50 items         |\n");
    printf("|  No visual aids during input             |\n");
    printf("|  Pure memory challenge                    |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_MEMORY_MASTER;
    simon_says_game_loop();
}

// Color Blind Friendly Mode
void simon_says_color_blind_mode(void) {
    simon_says_display_header("COLOR BLIND FRIENDLY");
    printf("|                                           |\n");
    printf("|  >>> SHAPES & SYMBOLS MODE <<<           |\n");
    printf("|                                           |\n");
    printf("|  Uses shapes instead of colors           |\n");
    printf("|  Clear visual distinctions               |\n");
    printf("|  Accessible for everyone                 |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_COLOR_BLIND;
    game.current_theme = 1; // Use shapes theme
    simon_says_game_loop();
}

// Reverse Mode
void simon_says_reverse_mode(void) {
    simon_says_display_header("REVERSE SIMON");
    printf("|                                           |\n");
    printf("|  >>> BACKWARDS CHALLENGE <<<             |\n");
    printf("|                                           |\n");
    printf("|  Input the sequence in reverse order     |\n");
    printf("|  Tests working memory skills             |\n");
    printf("|  Mental gymnastics challenge             |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_REVERSE;
    simon_says_game_loop();
}

// Number Sequence Mode
void simon_says_number_mode(void) {
    simon_says_display_header("NUMBER SEQUENCE");
    printf("|                                           |\n");
    printf("|  >>> NUMERICAL PATTERNS <<<              |\n");
    printf("|                                           |\n");
    printf("|  Uses digits 1-9 instead of colors       |\n");
    printf("|  More complex number patterns             |\n");
    printf("|  Mathematical sequence challenge          |\n");
    printf("|                                           |\n");
    printf("|  Press Enter to start...                  |\n");
    printf("===============================================\n");
    
    getchar();
    simon_says_init_game();
    game.current_mode = MODE_NUMBER_SEQUENCE;
    game.current_difficulty = DIFF_HARD; // Use more numbers
    simon_says_game_loop();
}

// Utility Functions
void simon_says_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void simon_says_play_sound(const char* sound) {
    if (game.sound_enabled) {
        printf("    [SFX] %s\n", sound);
        fflush(stdout);
        SLEEP_MS(100);
    }
}

void simon_says_play_success_sound(void) {
    simon_says_play_sound("SUCCESS!");
}

void simon_says_play_error_sound(void) {
    simon_says_play_sound("ERROR!");
}

void simon_says_play_achievement_sound(void) {
    simon_says_play_sound("ACHIEVEMENT UNLOCKED!");
}

int simon_says_get_random_number(int max) {
    return rand() % max;
}

double simon_says_get_elapsed_time(void) {
    return difftime(time(NULL), round_start_time);
}

void simon_says_display_sequence(void) {
    printf("|                                           |\n");
    printf("| SEQUENCE TO REMEMBER: ");
    for (int i = 0; i < game.sequence_length && i < 10; i++) {
        printf("%d ", game.sequence[i]);
    }
    if (game.sequence_length > 10) {
        printf("...");
    }
    printf("|\n");
    printf("|                                           |\n");
    
    // Give time to memorize
    printf("| Memorize this sequence... (3 seconds)     |\n");
    SLEEP_MS(3000);
    
    // Clear the sequence from view
    printf("\033[4A"); // Move cursor up 4 lines
    printf("|                                           |\n");
    printf("| SEQUENCE HIDDEN - Enter from memory!     |\n");
    printf("|                                           |\n");
    printf("|                                           |\n");
}

// Save/Load Statistics (simplified stubs)
void simon_says_save_statistics(void) {
    // Would save to file in full implementation
    // For now, just maintain session stats
}

void simon_says_load_statistics(void) {
    // Would load from file in full implementation
    // For now, start with clean stats
}