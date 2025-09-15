/*
 * F1 Reaction Start Game
 * Simulates Formula 1 starting lights and reaction time testing
 * Features 6 game modes including authentic F1 scenarios
 * ASCII-only version for Windows compatibility
 * 
 * Author: CLI Games Pack
 * Version: 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Platform-specific includes and definitions
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP_MS(ms) Sleep(ms)
    #define GET_TIME(tv) QueryPerformanceCounter(&tv)
    typedef LARGE_INTEGER TimeValue;
#else
    #include <sys/time.h>
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define GET_TIME(tv) gettimeofday(&tv, NULL)
    typedef struct timeval TimeValue;
#endif

// Game constants
#define MAX_DRIVERS 20
#define MAX_RACES 10
#define MAX_NAME_LENGTH 50
#define CHAMPIONSHIP_RACES 10
#define SQ_ROUNDS 3

// Performance thresholds (in milliseconds)
#define EXCELLENT_TIME 180.0
#define GOOD_TIME 220.0
#define AVERAGE_TIME 300.0
#define POLE_POSITION_TIME 180.0

// Game structures
typedef struct {
    char name[MAX_NAME_LENGTH];
    double best_time;
    double average_time;
    int races_completed;
    int championship_points;
    int pole_positions;
    int false_starts;
    double total_time;
    int sq3_appearances;
    int restart_wins;
} F1Driver;

typedef struct {
    F1Driver player;
    double session_times[MAX_RACES];
    int session_count;
    bool game_active;
    int current_mode;
    double frequency;  // For Windows high-resolution timer
} F1Game;

// Global game state
static F1Game game = {0};

// Function prototypes
void f1_reaction_clear_input_buffer(void);
void f1_reaction_display_header(const char* title);
void f1_reaction_display_instructions(void);
void f1_reaction_display_statistics(void);
void f1_reaction_save_stats(void);
void f1_reaction_load_stats(void);
double f1_reaction_get_time_diff(TimeValue start, TimeValue end);
void f1_reaction_display_lights(int lights_on);
bool f1_reaction_wait_for_space(TimeValue* reaction_time);
double f1_reaction_single_start(void);
int f1_reaction_calculate_grid_position(double reaction_time);
const char* f1_reaction_get_performance_rating(double reaction_time);
void f1_reaction_display_result(double reaction_time, int grid_position);
void f1_reaction_quick_race_mode(void);
void f1_reaction_championship_mode(void);
void f1_reaction_training_mode(void);
void f1_reaction_multiplayer_mode(void);
void f1_reaction_safety_car_mode(void);
void f1_reaction_sprint_qualifying_mode(void);
void f1_reaction_game_loop(void);

// Sound effect functions
void f1_reaction_play_engine_sound(void);
void f1_reaction_play_tire_squeal(void);
void f1_reaction_play_crowd_cheer(void);
void f1_reaction_play_radio_beep(void);
void f1_reaction_play_formation_lap_sound(void);

// Sound effect implementations
void f1_reaction_play_engine_sound(void) {
    printf("\n    >>> VROOOOOOM! REV REV REV! <<<\n");
    fflush(stdout);
    SLEEP_MS(800);
}

void f1_reaction_play_tire_squeal(void) {
    printf("\n        SCREEEEEECH! SCREECH!\n");
    fflush(stdout);
    SLEEP_MS(600);
}

void f1_reaction_play_crowd_cheer(void) {
    printf("\n     ROAAAAAAR! *CROWD GOES WILD!*\n");
    fflush(stdout);
    SLEEP_MS(1000);
}

void f1_reaction_play_radio_beep(void) {
    printf("    [RADIO] *BEEP BEEP* Race Control...\n");
    fflush(stdout);
    SLEEP_MS(400);
}

void f1_reaction_play_formation_lap_sound(void) {
    printf("\n    [ENGINE] Warming up... RUMBLE RUMBLE...\n");
    printf("    [TIRES]  Getting heat into the tires...\n");
    printf("    [RADIO]  Formation lap complete, grid positions!\n");
    fflush(stdout);
    SLEEP_MS(1500);
}

// Utility function implementations
void f1_reaction_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void f1_reaction_display_header(const char* title) {
    CLEAR_SCREEN();
    printf("================================================\n");
    printf("|           F1 REACTION START v1.0           |\n");
    printf("================================================\n");
    printf("|                                            |\n");
    printf("|              %-29s |\n", title);
    printf("|                                            |\n");
    printf("================================================\n");
    printf("|                                            |\n");
}

void f1_reaction_display_instructions(void) {
    f1_reaction_display_header("GAME INSTRUCTIONS");
    printf("|  >>  FORMULA 1 REACTION START TRAINING <<  |\n");
    printf("|                                            |\n");
    printf("|  OBJECTIVE:                                |\n");
    printf("|  React instantly when all lights go out   |\n");
    printf("|  Just like real F1 drivers at race start  |\n");
    printf("|                                            |\n");
    printf("|  HOW TO PLAY:                              |\n");
    printf("|  1. Watch the 5 red lights turn on        |\n");
    printf("|  2. Wait for ALL lights to go out         |\n");
    printf("|  3. Press SPACE immediately when dark     |\n");
    printf("|  4. Your reaction time determines grid     |\n");
    printf("|                                            |\n");
    printf("|  GRID POSITIONS:                           |\n");
    printf("|  [1] P1-P3:  0.000-0.180s (Pole Zone)     |\n");
    printf("|  [2] P4-P10: 0.181-0.220s (Points Zone)   |\n");
    printf("|  [3] P11-P15: 0.221-0.300s (Midfield)     |\n");
    printf("|  [X] P16-P20: 0.301s+ (Elimination)       |\n");
    printf("|                                            |\n");
    printf("|  F1 DRIVER COMPARISON:                     |\n");
    printf("|  [*] Hamilton Level:  0.150-0.180s         |\n");
    printf("|  [*] Verstappen Level: 0.160-0.190s       |\n");
    printf("|  [*] Rookie Level:     0.200-0.300s       |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    printf("\nPress Enter to continue...");
    getchar();
}

void f1_reaction_display_statistics(void) {
    f1_reaction_display_header("DRIVER STATISTICS");
    printf("|  DRIVER: %-33s |\n", game.player.name[0] ? game.player.name : "Anonymous");
    printf("|                                            |\n");
    printf("|  CURRENT SESSION:                          |\n");
    if (game.session_count > 0) {
        double best_session = game.session_times[0];
        double total_session = 0;
        for (int i = 0; i < game.session_count; i++) {
            if (game.session_times[i] < best_session) {
                best_session = game.session_times[i];
            }
            total_session += game.session_times[i];
        }
        double avg_session = total_session / game.session_count;
        printf("|  - Session Best: %.3fs %-17s |\n", best_session, f1_reaction_get_performance_rating(best_session));
        printf("|  - Session Avg:  %.3fs                   |\n", avg_session);
        printf("|  - Attempts:     %-23d |\n", game.session_count);
    } else {
        printf("|  - No attempts this session               |\n");
    }
    printf("|                                            |\n");
    printf("|  CAREER STATISTICS:                        |\n");
    printf("|  - Best Time:    %.3fs %-17s |\n", 
           game.player.best_time > 0 ? game.player.best_time : 0.0,
           game.player.best_time > 0 ? f1_reaction_get_performance_rating(game.player.best_time) : "");
    printf("|  - Average Time: %.3fs                   |\n", game.player.average_time);
    printf("|  - Total Races:  %-23d |\n", game.player.races_completed);
    printf("|  - Pole Positions: %-21d |\n", game.player.pole_positions);
    printf("|  - Jump Starts: %-24d |\n", game.player.false_starts);
    printf("|  - SQ3 Appearances: %-20d |\n", game.player.sq3_appearances);
    printf("|  - Restart Wins: %-23d |\n", game.player.restart_wins);
    printf("|                                            |\n");
    printf("|  F1 DRIVER LEVEL:                          |\n");
    if (game.player.best_time <= 0.0) {
        printf("|  - [NEW] ROOKIE (No times set)            |\n");
    } else if (game.player.best_time <= 0.160) {
        printf("|  - [***] LEGENDARY (Hamilton Level!)      |\n");
    } else if (game.player.best_time <= 0.180) {
        printf("|  - [**] CHAMPION (F1 Driver Level!)       |\n");
    } else if (game.player.best_time <= 0.220) {
        printf("|  - [*] PROFESSIONAL (Racing Driver!)      |\n");
    } else if (game.player.best_time <= 0.300) {
        printf("|  - [-] AMATEUR (Keep Training!)           |\n");
    } else {
        printf("|  - [NEW] ROOKIE (Practice More!)          |\n");
    }
    printf("|                                            |\n");
    printf("================================================\n");
    printf("\nPress Enter to continue...");
    getchar();
}

// High-precision timing functions
double f1_reaction_get_time_diff(TimeValue start, TimeValue end) {
#ifdef _WIN32
    return ((double)(end.QuadPart - start.QuadPart)) / game.frequency * 1000.0;
#else
    return ((end.tv_sec - start.tv_sec) * 1000.0) + 
           ((end.tv_usec - start.tv_usec) / 1000.0);
#endif
}

// Game display functions
void f1_reaction_display_lights(int lights_on) {
    printf("\r|              RACE START LIGHTS            |\n");
    printf("|                                            |\n");
    printf("|        ");
    
    // Display red lights using [R] for red, [ ] for off
    for (int i = 0; i < 5; i++) {
        if (i < lights_on) {
            printf("[R]");
        } else {
            printf("[ ]");
        }
    }
    
    printf("  ");
    
    // Display second row
    for (int i = 0; i < 5; i++) {
        if (i < lights_on) {
            printf("[R]");
        } else {
            printf("[ ]");
        }
    }
    
    printf("         |\n");
    printf("|                                            |\n");
    
    if (lights_on == 0) {
        printf("|           >>> LIGHTS OUT - GO! <<<        |\n");
    } else if (lights_on == 5) {
        printf("|    Wait for all lights to go out...       |\n");
    } else {
        printf("|         Building up the lights...         |\n");
    }
    
    printf("|                                            |\n");
    printf("|      PRESS [SPACE] WHEN LIGHTS OUT!        |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    fflush(stdout);
}

bool f1_reaction_wait_for_space(TimeValue* reaction_time) {
#ifdef _WIN32
    while (!_kbhit()) {
        SLEEP_MS(1);
    }
    char key = _getch();
    GET_TIME(*reaction_time);
    return (key == ' ');
#else
    // Set terminal to non-blocking mode
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    char key;
    while (read(STDIN_FILENO, &key, 1) <= 0) {
        usleep(1000);
    }
    
    GET_TIME(*reaction_time);
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    fcntl(STDIN_FILENO, F_SETFL, flags);
    
    return (key == ' ');
#endif
}

// Core game logic
double f1_reaction_single_start(void) {
    TimeValue start_time, reaction_time;
    
    f1_reaction_display_header("F1 RACE START");
    
    // Formation lap complete - engines warming up
    f1_reaction_play_formation_lap_sound();
    
    // Initialize timing
#ifdef _WIN32
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    game.frequency = (double)freq.QuadPart;
#endif
    
    // Build up the lights (1 second intervals) with engine sounds
    for (int i = 1; i <= 5; i++) {
        f1_reaction_display_lights(i);
        
        // Add engine rev sounds for each light
        if (i == 1) {
            printf("    [ENGINE] RUMBLE... Getting ready...\n");
        } else if (i == 3) {
            printf("    [ENGINE] REV REV! Building power...\n");
        } else if (i == 5) {
            printf("    [ENGINE] VROOOOM! All lights on!\n");
            printf("    [CROWD]  *Anticipation building*\n");
        }
        
        fflush(stdout);
        SLEEP_MS(1000);
    }
    
    // Random hold time (1-5 seconds like real F1)
    int hold_time = (rand() % 4000) + 1000;  // 1000-5000ms
    SLEEP_MS(hold_time);
    
    // LIGHTS OUT!
    GET_TIME(start_time);
    f1_reaction_display_lights(0);
    
    // Add race start sound effect
    printf("    [ENGINE] VROOOOOOOOM! FULL POWER!\n");
    fflush(stdout);
    
    // Wait for player reaction
    if (!f1_reaction_wait_for_space(&reaction_time)) {
        printf("\n\n[X] Wrong key! Press SPACE only!\n");
        printf("Press Enter to try again...");
        getchar();
        return -1.0;
    }
    
    // Calculate reaction time
    double reaction_ms = f1_reaction_get_time_diff(start_time, reaction_time);
    
    // Check for false start (negative time means they pressed before lights out)
    if (reaction_ms < 0) {
        CLEAR_SCREEN();
        printf("================================================\n");
        printf("|                                            |\n");
        printf("|              [!] JUMP START! [!]          |\n");
        printf("|                                            |\n");
        printf("|     You reacted before lights went out    |\n");
        printf("|         Reaction: %.3fs (early)         |\n", reaction_ms / 1000.0);
        printf("|                                            |\n");
        printf("|           >>> DISQUALIFIED! <<<           |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        
        // Add jump start sound effects
        f1_reaction_play_tire_squeal();
        printf("    [STEWARDS] JUMP START DETECTED!\n");
        f1_reaction_play_radio_beep();
        printf("    [RACE CONTROL] Car disqualified!\n");
        
        game.player.false_starts++;
        printf("\nPress Enter to try again...");
        getchar();
        return -1.0;
    }
    
    return reaction_ms;
}

int f1_reaction_calculate_grid_position(double reaction_time) {
    if (reaction_time <= 180.0) return 1 + (rand() % 3);      // P1-P3
    else if (reaction_time <= 220.0) return 4 + (rand() % 7); // P4-P10
    else if (reaction_time <= 300.0) return 11 + (rand() % 5); // P11-P15
    else return 16 + (rand() % 5);                            // P16-P20
}

const char* f1_reaction_get_performance_rating(double reaction_time) {
    if (reaction_time <= 160.0) return "LEGENDARY!";
    else if (reaction_time <= 180.0) return "EXCELLENT!";
    else if (reaction_time <= 220.0) return "GOOD!";
    else if (reaction_time <= 300.0) return "AVERAGE";
    else return "NEEDS WORK";
}

void f1_reaction_display_result(double reaction_time, int grid_position) {
    CLEAR_SCREEN();
    printf("================================================\n");
    printf("|              >>> RACE RESULTS <<<          |\n");
    printf("================================================\n");
    printf("|                                            |\n");
    printf("|  Your Reaction: %.3fs                    |\n", reaction_time / 1000.0);
    
    if (grid_position <= 3) {
        printf("|  Grid Position: P%d [GOLD]                 |\n", grid_position);
        printf("|  Status: POLE POSITION ZONE!               |\n");
    } else if (grid_position <= 10) {
        printf("|  Grid Position: P%d [SILVER]               |\n", grid_position);
        printf("|  Status: POINTS SCORING POSITION!          |\n");
    } else if (grid_position <= 15) {
        printf("|  Grid Position: P%d [BRONZE]               |\n", grid_position);
        printf("|  Status: MIDFIELD BATTLE                   |\n");
    } else {
        printf("|  Grid Position: P%d [OUT]                  |\n", grid_position);
        printf("|  Status: ELIMINATION ZONE                  |\n");
    }
    
    printf("|  Performance: %-28s |\n", f1_reaction_get_performance_rating(reaction_time));
    printf("|                                            |\n");
    
    // F1 driver comparison
    printf("|  F1 DRIVER COMPARISON:                     |\n");
    if (reaction_time <= 160.0) {
        printf("|  [*] You're at Hamilton/Verstappen level! |\n");
    } else if (reaction_time <= 180.0) {
        printf("|  [*] Professional F1 driver level!        |\n");
    } else if (reaction_time <= 220.0) {
        printf("|  [-] Racing driver level - well done!     |\n");
    } else if (reaction_time <= 300.0) {
        printf("|  [-] Amateur level - keep practicing!     |\n");
    } else {
        printf("|  [NEW] Rookie level - more training!      |\n");
    }
    
    printf("|                                            |\n");
    printf("================================================\n");
    
    // Add sound effects based on performance
    if (reaction_time <= 160.0) {
        f1_reaction_play_crowd_cheer();
        printf("    [COMMENTATOR] INCREDIBLE! WORLD CLASS!\n");
    } else if (reaction_time <= 180.0) {
        f1_reaction_play_crowd_cheer();
        printf("    [COMMENTATOR] FANTASTIC START!\n");
    } else if (reaction_time <= 220.0) {
        printf("    [ENGINE] Good launch off the line!\n");
    } else if (reaction_time <= 300.0) {
        printf("    [ENGINE] Decent getaway...\n");
    } else {
        printf("    [ENGINE] Slow start, lost positions!\n");
    }
    
    fflush(stdout);
    
    // Update statistics
    double reaction_seconds = reaction_time / 1000.0;
    if (game.player.best_time <= 0.0 || reaction_seconds < game.player.best_time) {
        game.player.best_time = reaction_seconds;
        printf("\n[!] NEW PERSONAL BEST! [!]\n");
        f1_reaction_play_radio_beep();
        printf("    [TEAM RADIO] Brilliant! New personal best!\n");
    }
    
    if (grid_position <= 3) {
        game.player.pole_positions++;
        printf("[GOLD] POLE POSITION ACHIEVED! [GOLD]\n");
        f1_reaction_play_crowd_cheer();
        printf("    [COMMENTATOR] POLE POSITION! SUPERB!\n");
    }
    
    // Update averages
    game.player.total_time += reaction_seconds;
    game.player.races_completed++;
    game.player.average_time = game.player.total_time / game.player.races_completed;
    
    // Store session time
    if (game.session_count < MAX_RACES) {
        game.session_times[game.session_count] = reaction_seconds;
        game.session_count++;
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

// Game mode implementations
void f1_reaction_quick_race_mode(void) {
    f1_reaction_display_header("QUICK RACE START");
    printf("|              >>> QUICK START MODE <<<     |\n");
    printf("|                                            |\n");
    printf("|  Single F1 race start simulation          |\n");
    printf("|  Perfect for quick reaction training       |\n");
    printf("|                                            |\n");
    printf("|  Ready to start? Press Enter to begin...  |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter when ready...");
    getchar();
    
    double reaction_time = f1_reaction_single_start();
    if (reaction_time > 0) {
        int grid_position = f1_reaction_calculate_grid_position(reaction_time);
        f1_reaction_display_result(reaction_time, grid_position);
    }
}

void f1_reaction_championship_mode(void) {
    f1_reaction_display_header("CHAMPIONSHIP MODE");
    printf("|            [*] CHAMPIONSHIP SEASON [*]     |\n");
    printf("|                                            |\n");
    printf("|  10-race championship campaign             |\n");
    printf("|  Consistent performance wins titles        |\n");
    printf("|                                            |\n");
    printf("|  Points System:                            |\n");
    printf("|  P1-P3: 25-18 points                       |\n");
    printf("|  P4-P10: 12-1 points                       |\n");
    printf("|  P11+: 0 points                            |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to start championship...");
    getchar();
    
    int total_points = 0;
    const char* tracks[] = {
        "Bahrain GP", "Saudi Arabia GP", "Australian GP", "Japanese GP", "Chinese GP",
        "Miami GP", "Emilia Romagna GP", "Monaco GP", "Spanish GP", "Canadian GP"
    };
    
    for (int race = 0; race < CHAMPIONSHIP_RACES; race++) {
        f1_reaction_display_header("CHAMPIONSHIP MODE");
        printf("|  RACE %d/10: %-28s |\n", race + 1, tracks[race]);
        printf("|  Current Points: %-25d |\n", total_points);
        printf("|                                            |\n");
        printf("|  Press Enter for %-24s |\n", tracks[race]);
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\nPress Enter to start race...");
        getchar();
        
        double reaction_time = f1_reaction_single_start();
        if (reaction_time > 0) {
            int grid_position = f1_reaction_calculate_grid_position(reaction_time);
            
            // Calculate points
            int race_points = 0;
            if (grid_position == 1) race_points = 25;
            else if (grid_position == 2) race_points = 18;
            else if (grid_position == 3) race_points = 15;
            else if (grid_position <= 10) race_points = 13 - grid_position;
            
            total_points += race_points;
            
            CLEAR_SCREEN();
            printf("================================================\n");
            printf("|         >>> %s RESULTS <<<        |\n", tracks[race]);
            printf("================================================\n");
            printf("|                                            |\n");
            printf("|  Reaction Time: %.3fs                    |\n", reaction_time / 1000.0);
            printf("|  Grid Position: P%d                        |\n", grid_position);
            printf("|  Points Earned: %d                         |\n", race_points);
            printf("|  Total Points: %d                          |\n", total_points);
            printf("|                                            |\n");
            printf("|  Championship Standing: ");
            if (total_points >= 200) printf("[GOLD] CHAMPION!    |\n");
            else if (total_points >= 150) printf("[SILVER] TITLE!     |\n");
            else if (total_points >= 100) printf("[BRONZE] PODIUM!    |\n");
            else printf("[-] POINTS BATTLE! |\n");
            printf("|                                            |\n");
            printf("================================================\n");
            
            if (race < CHAMPIONSHIP_RACES - 1) {
                printf("\nPress Enter to continue to next race...");
                getchar();
            }
        }
    }
    
    // Final championship result
    f1_reaction_display_header("CHAMPIONSHIP FINAL");
    printf("|         [*] CHAMPIONSHIP RESULTS [*]       |\n");
    printf("|                                            |\n");
    printf("|  Final Points: %-27d |\n", total_points);
    printf("|  Races Won: %-29d |\n", game.player.pole_positions);
    printf("|                                            |\n");
    
    if (total_points >= 250) {
        printf("|  [GOLD] WORLD CHAMPION! LEGENDARY SEASON! |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        f1_reaction_play_crowd_cheer();
        printf("    [CROWD] CHAMPION! CHAMPION! CHAMPION!\n");
        printf("    [COMMENTATOR] ABSOLUTELY INCREDIBLE!\n");
        printf("    [TEAM RADIO] YOU ARE THE CHAMPION!\n");
    } else if (total_points >= 200) {
        printf("|  [SILVER] RUNNER-UP! EXCELLENT SEASON!    |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        printf("    [CROWD] What a season! Brilliant driving!\n");
    } else if (total_points >= 150) {
        printf("|  [BRONZE] PODIUM FINISH! STRONG SEASON!   |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        printf("    [COMMENTATOR] Solid championship result!\n");
    } else if (total_points >= 100) {
        printf("|  [-] POINTS FINISH! SOLID PERFORMANCE!    |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        printf("    [TEAM] Good points haul this season!\n");
    } else {
        printf("|  [NEW] LEARNING SEASON! KEEP IMPROVING!   |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        printf("    [COACH] Keep training, you'll improve!\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void f1_reaction_training_mode(void) {
    f1_reaction_display_header("REACTION TRAINING");
    printf("|            [-] TRAINING MODE [-]           |\n");
    printf("|                                            |\n");
    printf("|  Rapid-fire reaction training              |\n");
    printf("|  5 consecutive starts to build muscle     |\n");
    printf("|  memory and improve consistency            |\n");
    printf("|                                            |\n");
    printf("|  Goal: Consistent sub-0.200s reactions     |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to start training...");
    getchar();
    
    double training_times[5];
    int successful_starts = 0;
    
    for (int i = 0; i < 5; i++) {
        f1_reaction_display_header("REACTION TRAINING");
        printf("|  TRAINING SESSION: %d/5                   |\n", i + 1);
        printf("|  Successful Starts: %d                    |\n", successful_starts);
        printf("|                                            |\n");
        printf("|  Press Enter for next start...            |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\nPress Enter when ready...");
        getchar();
        
        double reaction_time = f1_reaction_single_start();
        if (reaction_time > 0) {
            training_times[successful_starts] = reaction_time / 1000.0;
            successful_starts++;
            
            printf("\nStart %d: %.3fs (%s)\n", successful_starts, 
                   reaction_time / 1000.0, 
                   f1_reaction_get_performance_rating(reaction_time));
            printf("Press Enter to continue...");
            getchar();
        }
    }
    
    // Training summary
    if (successful_starts > 0) {
        double total = 0, best = training_times[0], worst = training_times[0];
        
        for (int i = 0; i < successful_starts; i++) {
            total += training_times[i];
            if (training_times[i] < best) best = training_times[i];
            if (training_times[i] > worst) worst = training_times[i];
        }
        
        double average = total / successful_starts;
        
        f1_reaction_display_header("TRAINING SUMMARY");
        printf("|         [-] TRAINING RESULTS [-]           |\n");
        printf("|                                            |\n");
        printf("|  Successful Starts: %d/5                   |\n", successful_starts);
        printf("|  Best Time: %.3fs                         |\n", best);
        printf("|  Worst Time: %.3fs                        |\n", worst);
        printf("|  Average: %.3fs                           |\n", average);
        printf("|  Consistency: %.1f%%                       |\n", 
               ((best / worst) * 100.0));
        printf("|                                            |\n");
        
        if (average <= 0.200) {
            printf("|  [*] EXCELLENT! F1 DRIVER LEVEL!          |\n");
        } else if (average <= 0.250) {
            printf("|  [-] GOOD! RACING DRIVER LEVEL!           |\n");
        } else {
            printf("|  [NEW] KEEP TRAINING! IMPROVEMENT NEEDED! |\n");
        }
        
        printf("|                                            |\n");
        printf("================================================\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void f1_reaction_multiplayer_mode(void) {
    f1_reaction_display_header("MULTIPLAYER MODE");
    printf("|           [VS] MULTIPLAYER CHALLENGE [VS]  |\n");
    printf("|                                            |\n");
    printf("|  Head-to-head reaction battles             |\n");
    printf("|  Take turns on the same computer           |\n");
    printf("|  Best of 3 rounds wins the match           |\n");
    printf("|                                            |\n");
    printf("|  Enter player names:                       |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    char player1[50], player2[50];
    printf("\nPlayer 1 name: ");
    fgets(player1, sizeof(player1), stdin);
    player1[strcspn(player1, "\n")] = 0; // Remove newline
    
    printf("Player 2 name: ");
    fgets(player2, sizeof(player2), stdin);
    player2[strcspn(player2, "\n")] = 0; // Remove newline
    
    int p1_wins = 0, p2_wins = 0;
    double p1_times[3], p2_times[3];
    
    for (int round = 0; round < 3; round++) {
        // Player 1 turn
        f1_reaction_display_header("MULTIPLAYER MODE");
        printf("|  ROUND %d/3 - %s's TURN                  |\n", round + 1, player1);
        printf("|  Score: %s %d - %d %s                    |\n", 
               player1, p1_wins, p2_wins, player2);
        printf("|                                            |\n");
        printf("|  %s, get ready for your start!           |\n", player1);
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\n%s, press Enter when ready...", player1);
        getchar();
        
        double p1_time = f1_reaction_single_start();
        if (p1_time > 0) {
            p1_times[round] = p1_time / 1000.0;
            printf("\n%s: %.3fs\n", player1, p1_times[round]);
        } else {
            p1_times[round] = 999.0; // Penalty for false start
        }
        
        printf("Press Enter for %s's turn...", player2);
        getchar();
        
        // Player 2 turn
        f1_reaction_display_header("MULTIPLAYER MODE");
        printf("|  ROUND %d/3 - %s's TURN                  |\n", round + 1, player2);
        printf("|  Score: %s %d - %d %s                    |\n", 
               player1, p1_wins, p2_wins, player2);
        printf("|                                            |\n");
        printf("|  %s, get ready for your start!           |\n", player2);
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\n%s, press Enter when ready...", player2);
        getchar();
        
        double p2_time = f1_reaction_single_start();
        if (p2_time > 0) {
            p2_times[round] = p2_time / 1000.0;
            printf("\n%s: %.3fs\n", player2, p2_times[round]);
        } else {
            p2_times[round] = 999.0; // Penalty for false start
        }
        
        // Round result
        printf("\nROUND %d RESULTS:\n", round + 1);
        printf("%s: %.3fs\n", player1, p1_times[round]);
        printf("%s: %.3fs\n", player2, p2_times[round]);
        
        if (p1_times[round] < p2_times[round]) {
            printf("[*] %s wins round %d!\n", player1, round + 1);
            printf("    [COMMENTATOR] %s takes the round!\n", player1);
            p1_wins++;
        } else {
            printf("[*] %s wins round %d!\n", player2, round + 1);
            printf("    [COMMENTATOR] %s takes the round!\n", player2);
            p2_wins++;
        }
        
        // Add excitement sound for close races
        if (abs((int)((p1_times[round] - p2_times[round]) * 1000)) < 50) {
            printf("    [CROWD] INCREDIBLE! SO CLOSE!\n");
            fflush(stdout);
            SLEEP_MS(500);
        }
        
        printf("\nPress Enter to continue...");
        getchar();
    }
    
    // Final result
    f1_reaction_display_header("MULTIPLAYER FINAL");
    printf("|           [*] MATCH RESULTS [*]            |\n");
    printf("|                                            |\n");
    printf("|  Final Score: %s %d - %d %s               |\n", 
           player1, p1_wins, p2_wins, player2);
    printf("|                                            |\n");
    
    if (p1_wins > p2_wins) {
        printf("|  [GOLD] WINNER: %-25s |\n", player1);
    } else {
        printf("|  [GOLD] WINNER: %-25s |\n", player2);
    }
    
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

void f1_reaction_safety_car_mode(void) {
    f1_reaction_display_header("SAFETY CAR RESTART");
    printf("|         [SC] SAFETY CAR RESTART MODE [SC]  |\n");
    printf("|                                            |\n");
    printf("|  Simulate F1 safety car restart           |\n");
    printf("|  React when safety car pits and           |\n");
    printf("|  green flags are shown                     |\n");
    printf("|                                            |\n");
    printf("|  Longer hold times test concentration      |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to start safety car period...");
    getchar();
    
    f1_reaction_display_header("SAFETY CAR PERIOD");
    printf("|     [Y][Y][Y]  YELLOW FLAGS WAVING  [Y][Y][Y] |\n");
    printf("|                                            |\n");
    printf("|   [SC] Following safety car... [SC]       |\n");
    printf("|                                            |\n");
    printf("|        Reduced speed - stay alert!         |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    // Safety car period sounds
    printf("    [ENGINE] Reducing speed... RUMBLE rumble...\n");
    printf("    [RADIO] Yellow flags, yellow flags!\n");
    fflush(stdout);
    
    SLEEP_MS(3000);
    
    printf("\n[!] SAFETY CAR IN THIS LAP [!]\n");
    f1_reaction_play_radio_beep();
    printf("    [RACE CONTROL] Safety car in this lap!\n");
    SLEEP_MS(2000);
    
    printf("[>>] SAFETY CAR PITTING... [>>]\n");
    printf("    [ENGINE] Cars bunching up... preparing to race!\n");
    fflush(stdout);
    
    // Longer random delay for safety car restarts (3-8 seconds)
    int safety_delay = (rand() % 5000) + 3000;
    SLEEP_MS(safety_delay);
    
    TimeValue start_time, reaction_time;
    GET_TIME(start_time);
    
    f1_reaction_display_header("SAFETY CAR RESTART");
    printf("|        [G][G][G] GREEN FLAGS! GO! [G][G][G]    |\n");
    printf("|                                            |\n");
    printf("|           RESTART NOW! REACT!              |\n");
    printf("|                                            |\n");
    printf("|      PRESS [SPACE] IMMEDIATELY!            |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    // Green flag restart sound
    printf("    [ENGINE] VROOOOOM! GREEN FLAG RESTART!\n");
    fflush(stdout);
    
    if (!f1_reaction_wait_for_space(&reaction_time)) {
        printf("\n[X] Wrong key! Press SPACE only!\n");
        printf("Press Enter to try again...");
        getchar();
        return;
    }
    
    double reaction_ms = f1_reaction_get_time_diff(start_time, reaction_time);
    
    if (reaction_ms < 0) {
        printf("\n[!] JUMPED THE RESTART! DISQUALIFIED! [!]\n");
        game.player.false_starts++;
    } else {
        double reaction_seconds = reaction_ms / 1000.0;
        bool restart_win = reaction_seconds <= 0.200;
        
        if (restart_win) {
            game.player.restart_wins++;
        }
        
        CLEAR_SCREEN();
        printf("================================================\n");
        printf("|         [SC] RESTART RESULTS [SC]          |\n");
        printf("================================================\n");
        printf("|                                            |\n");
        printf("|  Restart Reaction: %.3fs                 |\n", reaction_seconds);
        printf("|  Result: ");
        
        if (restart_win) {
            printf("[*] PERFECT RESTART!           |\n");
            printf("|  You gained positions on restart!         |\n");
        } else {
            printf("[-] GOOD RESTART              |\n");
            printf("|  Maintained position in pack              |\n");
        }
        
        printf("|                                            |\n");
        printf("|  Safety Car Restarts Won: %-15d |\n", game.player.restart_wins);
        printf("|                                            |\n");
        printf("================================================\n");
        
        // Update session stats
        if (game.session_count < MAX_RACES) {
            game.session_times[game.session_count] = reaction_seconds;
            game.session_count++;
        }
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void f1_reaction_sprint_qualifying_mode(void) {
    f1_reaction_display_header("SPRINT QUALIFYING");
    printf("|         [-] SPRINT QUALIFYING MODE [-]     |\n");
    printf("|                                            |\n");
    printf("|  Three-round elimination format:           |\n");
    printf("|  SQ1: 20->15 drivers (eliminate 5)        |\n");
    printf("|  SQ2: 15->10 drivers (eliminate 5)        |\n");
    printf("|  SQ3: 10 drivers fight for pole           |\n");
    printf("|                                            |\n");
    printf("|  Must beat elimination time each round     |\n");
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to start Sprint Qualifying...");
    getchar();
    
    double elimination_times[] = {0.350, 0.280, 0.220}; // SQ1, SQ2, SQ3 cutoffs
    const char* session_names[] = {"SQ1", "SQ2", "SQ3"};
    const char* elimination_info[] = {
        "Bottom 5 eliminated (P16-P20)",
        "Bottom 5 eliminated (P11-P15)", 
        "Fight for pole position (P1-P10)"
    };
    
    bool qualified = true;
    int final_position = 20;
    
    for (int session = 0; session < 3 && qualified; session++) {
        f1_reaction_display_header("SPRINT QUALIFYING");
        printf("|  %s SESSION - %-26s |\n", session_names[session], elimination_info[session]);
        printf("|                                            |\n");
        printf("|  Elimination Time: %.3fs                 |\n", elimination_times[session]);
        printf("|  You must beat this time to advance!      |\n");
        printf("|                                            |\n");
        printf("|  Press Enter for your %s attempt...      |\n", session_names[session]);
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\nPress Enter when ready...");
        getchar();
        
        double reaction_time = f1_reaction_single_start();
        if (reaction_time > 0) {
            double reaction_seconds = reaction_time / 1000.0;
            
            CLEAR_SCREEN();
            printf("================================================\n");
            printf("|           %s RESULTS                      |\n", session_names[session]);
            printf("================================================\n");
            printf("|                                            |\n");
            printf("|  Your Time: %.3fs                         |\n", reaction_seconds);
            printf("|  Elimination: %.3fs                       |\n", elimination_times[session]);
            printf("|                                            |\n");
            
            if (reaction_seconds <= elimination_times[session]) {
                printf("|  [+] QUALIFIED! Advanced to next round    |\n");
                qualified = true;
                
                if (session == 0) final_position = 15; // Made it out of SQ1
                else if (session == 1) final_position = 10; // Made it out of SQ2
                else { // SQ3 - calculate final position
                    final_position = f1_reaction_calculate_grid_position(reaction_time);
                    if (final_position > 10) final_position = 10; // Cap at P10 for SQ3
                    game.player.sq3_appearances++;
                    
                    if (final_position <= 3) {
                        printf("|  [GOLD] FRONT ROW! POLE POSITION BATTLE!  |\n");
                        game.player.pole_positions++;
                    } else {
                        printf("|  [*] TOP 10! GREAT QUALIFYING RESULT!     |\n");
                    }
                }
            } else {
                printf("|  [X] ELIMINATED! Session ended             |\n");
                qualified = false;
                
                if (session == 0) final_position = 16 + (rand() % 5); // P16-P20
                else if (session == 1) final_position = 11 + (rand() % 5); // P11-P15
            }
            
            printf("|                                            |\n");
            printf("================================================\n");
            
            if (qualified && session < 2) {
                printf("\nPress Enter to continue to %s...", session_names[session + 1]);
                getchar();
            }
        } else {
            qualified = false;
            final_position = 20; // False start = last place
        }
    }
    
    // Final Sprint Qualifying result
    f1_reaction_display_header("SPRINT QUALIFYING");
    printf("|       >>> FINAL QUALIFYING RESULT <<<     |\n");
    printf("|                                            |\n");
    printf("|  Final Grid Position: P%-19d |\n", final_position);
    printf("|                                            |\n");
    
    if (final_position <= 3) {
        printf("|  [GOLD] FRONT ROW START! EXCELLENT!       |\n");
    } else if (final_position <= 10) {
        printf("|  [*] TOP 10 START! POINTS POSSIBLE!       |\n");
    } else if (final_position <= 15) {
        printf("|  [-] MIDFIELD START! GOOD EFFORT!         |\n");
    } else {
        printf("|  [NEW] BACK OF GRID! KEEP PRACTICING!     |\n");
    }
    
    printf("|                                            |\n");
    printf("|  SQ3 Appearances: %-24d |\n", game.player.sq3_appearances);
    printf("|                                            |\n");
    printf("================================================\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

// Main game loop
void f1_reaction_game_loop(void) {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Initialize player name if not set
    if (game.player.name[0] == 0) {
        strcpy(game.player.name, "Anonymous");
    }
    
    while (true) {
        f1_reaction_display_header("MAIN MENU");
        printf("|  [1] >>> Quick Race Start                  |\n");
        printf("|      - Single F1 start sequence           |\n");
        printf("|                                            |\n");
        printf("|  [2] [*] Championship Mode                 |\n");
        printf("|      - 10-race season campaign            |\n");
        printf("|                                            |\n");
        printf("|  [3] [-] Reaction Training                 |\n");
        printf("|      - Rapid-fire practice mode           |\n");
        printf("|                                            |\n");
        printf("|  [4] [VS] Multiplayer Challenge            |\n");
        printf("|      - Head-to-head battles               |\n");
        printf("|                                            |\n");
        printf("|  [5] [SC] Safety Car Restart              |\n");
        printf("|      - Restart simulation training        |\n");
        printf("|                                            |\n");
        printf("|  [6] [-] Sprint Qualifying                 |\n");
        printf("|      - Three-round elimination format     |\n");
        printf("|                                            |\n");
        printf("|  [7] [i] Statistics & Records             |\n");
        printf("|  [8] [?] Instructions                     |\n");
        printf("|  [9] [<] Back to Main Menu                |\n");
        printf("|                                            |\n");
        printf("================================================\n");
        
        printf("\nChoice (1-9): ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            f1_reaction_clear_input_buffer();
            continue;
        }
        f1_reaction_clear_input_buffer();
        
        switch (choice) {
            case 1:
                f1_reaction_quick_race_mode();
                break;
            case 2:
                f1_reaction_championship_mode();
                break;
            case 3:
                f1_reaction_training_mode();
                break;
            case 4:
                f1_reaction_multiplayer_mode();
                break;
            case 5:
                f1_reaction_safety_car_mode();
                break;
            case 6:
                f1_reaction_sprint_qualifying_mode();
                break;
            case 7:
                f1_reaction_display_statistics();
                break;
            case 8:
                f1_reaction_display_instructions();
                break;
            case 9:
                return;
            default:
                printf("Invalid choice! Press Enter to continue...");
                getchar();
                break;
        }
    }
}

// Main entry point
void play_f1_reaction(void) {
    f1_reaction_game_loop();
}