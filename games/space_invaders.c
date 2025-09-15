/*
 * Space Invaders Classic
 * ASCII-only retro arcade game with multiple custom modes
 * Defend Earth from descending alien waves!
 * 
 * Author: CLI Games Pack
 * Version: 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <sys/time.h>
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

// Game constants
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define ALIEN_ROWS 5
#define ALIEN_COLS 11
#define MAX_BULLETS 20
#define NUM_BARRIERS 4
#define BARRIER_WIDTH 7
#define BARRIER_HEIGHT 4
#define MAX_NAME_LENGTH 50

// Game modes
typedef enum {
    MODE_CLASSIC,
    MODE_SPEED_RUN,
    MODE_FORTRESS,
    MODE_MARKSMAN,
    MODE_ENDLESS,
    MODE_CUSTOM
} GameMode;

// Game states
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_WAVE_CLEAR,
    STATE_GAME_OVER,
    STATE_HIGH_SCORE
} GameState;

// Alien structure
typedef struct {
    int x, y;
    int type;           // 0=bottom (30pts), 1=middle (20pts), 2=top (10pts)
    bool alive;
    int animation_frame; // 0 or 1
} Alien;

// Player structure
typedef struct {
    int x, y;
    int lives;
    bool alive;
    int shoot_cooldown;
    int weapon_type;    // 0=normal, 1=rapid, 2=spread, 3=laser
} Player;

// Bullet structure
typedef struct {
    int x, y;
    int direction;      // 1=up (player), -1=down (alien)
    bool active;
    int type;          // 0=normal, 1=piercing, 2=explosive, 3=spread, 4=zigzag
    int speed;         // Bullet speed (pixels per frame)
    int animation_frame; // For animated projectiles
    int zigzag_offset;  // For zigzag movement pattern
} Bullet;

// Barrier structure
typedef struct {
    int x, y;
    char shape[BARRIER_HEIGHT][BARRIER_WIDTH + 1];
    bool damaged[BARRIER_HEIGHT][BARRIER_WIDTH];
} Barrier;

// UFO structure
typedef struct {
    int x, y;
    bool active;
    int direction;
    int points;
    int spawn_timer;
} UFO;

// Explosion structure for visual effects
typedef struct {
    int x, y;
    bool active;
    int frame;          // Animation frame (0-3)
    int duration;       // How long the explosion lasts
    int type;          // 0=small, 1=medium, 2=large
} Explosion;

#define MAX_EXPLOSIONS 10

// Game statistics
typedef struct {
    char player_name[MAX_NAME_LENGTH];
    int high_score;
    int games_played;
    int total_aliens_killed;
    int accuracy_shots_fired;
    int accuracy_shots_hit;
    int waves_completed;
    int perfect_waves;
    
    // Mode-specific records
    int classic_high_score;
    int speed_run_best_time;
    int fortress_waves_survived;
    int marksman_best_accuracy;
    int endless_wave_record;
    int custom_challenges_created;
} GameStats;

// Game state structure
typedef struct {
    GameMode current_mode;
    GameState state;
    Player player;
    Alien aliens[ALIEN_ROWS][ALIEN_COLS];
    Bullet bullets[MAX_BULLETS];
    Barrier barriers[NUM_BARRIERS];
    UFO ufo;
    Explosion explosions[MAX_EXPLOSIONS];
    
    int score;
    int wave;
    int aliens_remaining;
    int alien_direction;    // 1=right, -1=left
    int alien_move_timer;
    int alien_drop_timer;
    int alien_shoot_timer;
    
    // Mode-specific variables
    int speed_run_timer;
    int fortress_city_health;
    int marksman_ammo;
    int marksman_targets_hit;
    int endless_difficulty;
    
    // Custom mode settings
    int custom_alien_rows;
    int custom_alien_cols;
    int custom_alien_speed;
    bool custom_no_barriers;
    bool custom_infinite_ammo;
    bool custom_boss_mode;
    
    GameStats stats;
    bool game_running;
} SpaceInvadersGame;

// Global game instance
static SpaceInvadersGame game = {0};

// Function prototypes
void space_invaders_clear_input_buffer(void);
void space_invaders_display_header(const char* title);
void space_invaders_init_game(void);
void space_invaders_init_aliens(void);
void space_invaders_init_barriers(void);
void space_invaders_init_player(void);
void space_invaders_handle_input(void);
void space_invaders_update_game(void);
void space_invaders_update_aliens(void);
void space_invaders_update_bullets(void);
void space_invaders_update_collisions(void);
void space_invaders_update_ufo(void);
void space_invaders_draw_screen(void);
void space_invaders_draw_hud(void);
void space_invaders_draw_aliens(void);
void space_invaders_draw_player(void);
void space_invaders_draw_bullets(void);
void space_invaders_draw_barriers(void);
void space_invaders_draw_ufo(void);
void space_invaders_draw_explosions(void);
void space_invaders_create_explosion(int x, int y, int type);
void space_invaders_update_explosions(void);
void space_invaders_play_sound(const char* sound);
void space_invaders_main_menu(void);
void space_invaders_mode_selection(void);
void space_invaders_classic_mode(void);
void space_invaders_speed_run_mode(void);
void space_invaders_fortress_mode(void);
void space_invaders_marksman_mode(void);
void space_invaders_endless_mode(void);
void space_invaders_custom_mode(void);
void space_invaders_show_statistics(void);
void space_invaders_game_loop(void);

// Sound effect functions
void space_invaders_play_sound(const char* sound) {
    printf("    [SFX] %s\n", sound);
    fflush(stdout);
    SLEEP_MS(200);
}

// Explosion system functions
void space_invaders_create_explosion(int x, int y, int type) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!game.explosions[i].active) {
            game.explosions[i].active = true;
            game.explosions[i].x = x;
            game.explosions[i].y = y;
            game.explosions[i].type = type;
            game.explosions[i].frame = 0;
            game.explosions[i].duration = (type == 0) ? 8 : 12; // Different durations for different types
            break;
        }
    }
}

void space_invaders_update_explosions(void) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (game.explosions[i].active) {
            game.explosions[i].frame++;
            if (game.explosions[i].frame >= game.explosions[i].duration) {
                game.explosions[i].active = false;
            }
        }
    }
}

void space_invaders_draw_explosions(void) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (game.explosions[i].active) {
            int frame_cycle = game.explosions[i].frame % 4;
            char explosion_char;
            
            // Animate explosion with different characters
            switch (frame_cycle) {
                case 0: explosion_char = '*'; break;
                case 1: explosion_char = '+'; break;
                case 2: explosion_char = 'X'; break;
                case 3: explosion_char = '%'; break;
                default: explosion_char = '*'; break;
            }
            
            // Position cursor and draw explosion
            if (game.explosions[i].x >= 0 && game.explosions[i].x < SCREEN_WIDTH &&
                game.explosions[i].y >= 0 && game.explosions[i].y < SCREEN_HEIGHT) {
                printf("\033[%d;%dH%c", game.explosions[i].y, game.explosions[i].x, explosion_char);
            }
        }
    }
}

// Utility functions
void space_invaders_clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void space_invaders_display_header(const char* title) {
    CLEAR_SCREEN();
    printf("===============================================\n");
    printf("|          SPACE INVADERS 1978              |\n");
    printf("===============================================\n");
    printf("|                                           |\n");
    printf("|           %-31s |\n", title);
    printf("|                                           |\n");
    printf("===============================================\n");
}

// Game initialization
void space_invaders_init_game(void) {
    game.state = STATE_MENU;
    game.current_mode = MODE_CLASSIC;
    game.score = 0;
    game.wave = 1;
    game.alien_direction = 1;
    game.alien_move_timer = 0;
    game.alien_drop_timer = 0;
    game.alien_shoot_timer = 0;
    game.speed_run_timer = 0;
    game.fortress_city_health = 100;
    game.marksman_ammo = 10;
    game.marksman_targets_hit = 0;
    game.endless_difficulty = 1;
    
    // Custom mode defaults
    game.custom_alien_rows = 5;
    game.custom_alien_cols = 11;
    game.custom_alien_speed = 50;
    game.custom_no_barriers = false;
    game.custom_infinite_ammo = false;
    game.custom_boss_mode = false;
    
    space_invaders_init_player();
    space_invaders_init_aliens();
    space_invaders_init_barriers();
    
    // Initialize UFO
    game.ufo.active = false;
    game.ufo.spawn_timer = rand() % 300 + 200; // Random spawn time
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        game.bullets[i].active = false;
    }
    
    // Initialize explosions
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        game.explosions[i].active = false;
    }
    
    game.game_running = true;
}

void space_invaders_init_player(void) {
    game.player.x = SCREEN_WIDTH / 2;
    game.player.y = SCREEN_HEIGHT - 3;
    game.player.lives = 3;
    game.player.alive = true;
    game.player.shoot_cooldown = 0;
    game.player.weapon_type = 0;
}

void space_invaders_init_aliens(void) {
    int start_x = 10;
    int start_y = 5;
    game.aliens_remaining = 0;
    
    for (int row = 0; row < ALIEN_ROWS; row++) {
        for (int col = 0; col < ALIEN_COLS; col++) {
            game.aliens[row][col].x = start_x + col * 6;
            game.aliens[row][col].y = start_y + row * 2;
            game.aliens[row][col].alive = true;
            game.aliens[row][col].animation_frame = 0;
            
            // Set alien types (point values)
            if (row >= 4) game.aliens[row][col].type = 0; // Bottom: 30 points
            else if (row >= 2) game.aliens[row][col].type = 1; // Middle: 20 points
            else game.aliens[row][col].type = 2; // Top: 10 points
            
            game.aliens_remaining++;
        }
    }
}

void space_invaders_init_barriers(void) {
    const char barrier_template[BARRIER_HEIGHT][BARRIER_WIDTH + 1] = {
        "  ###  ",
        " ##### ",
        "#######",
        "##   ##"
    };
    
    int barrier_spacing = (SCREEN_WIDTH - (NUM_BARRIERS * BARRIER_WIDTH)) / (NUM_BARRIERS + 1);
    
    for (int i = 0; i < NUM_BARRIERS; i++) {
        game.barriers[i].x = barrier_spacing + i * (BARRIER_WIDTH + barrier_spacing);
        game.barriers[i].y = SCREEN_HEIGHT - 8;
        
        for (int row = 0; row < BARRIER_HEIGHT; row++) {
            strcpy(game.barriers[i].shape[row], barrier_template[row]);
            for (int col = 0; col < BARRIER_WIDTH; col++) {
                game.barriers[i].damaged[row][col] = false;
            }
        }
    }
}

// Input handling
void space_invaders_handle_input(void) {
    if (game.player.shoot_cooldown > 0) {
        game.player.shoot_cooldown--;
    }
    
#ifdef _WIN32
    if (_kbhit()) {
        char key = _getch();
        
        switch (key) {
            case 'a':
            case 'A':
                if (game.player.x > 2) game.player.x -= 2;
                break;
            case 'd':
            case 'D':
                if (game.player.x < SCREEN_WIDTH - 3) game.player.x += 2;
                break;
            case ' ':
                if (game.player.shoot_cooldown == 0) {
                    // Find empty bullet slot
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!game.bullets[i].active) {
                            game.bullets[i].x = game.player.x;
                            game.bullets[i].y = game.player.y - 1;
                            game.bullets[i].direction = 1; // Up
                            game.bullets[i].active = true;
                            game.bullets[i].type = 0; // Normal
                            game.bullets[i].speed = 1; // Normal speed
                            game.bullets[i].animation_frame = 0;
                            game.bullets[i].zigzag_offset = 0; // Straight shot
                            game.player.shoot_cooldown = 10;
                            space_invaders_play_sound("PEW!");
                            break;
                        }
                    }
                }
                break;
            case 'p':
            case 'P':
                if (game.state == STATE_PLAYING) {
                    game.state = STATE_PAUSED;
                } else if (game.state == STATE_PAUSED) {
                    game.state = STATE_PLAYING;
                }
                break;
            case 27: // Escape
                game.game_running = false;
                break;
        }
    }
#else
    // Linux input handling would go here
    // For now, simplified version
    char input;
    if (read(STDIN_FILENO, &input, 1) > 0) {
        // Handle input similar to Windows version
    }
#endif
}

// Game update functions
void space_invaders_update_game(void) {
    if (game.state != STATE_PLAYING) return;
    
    space_invaders_update_aliens();
    space_invaders_update_bullets();
    space_invaders_update_explosions();
    space_invaders_update_collisions();
    space_invaders_update_ufo();
    
    // Check win/lose conditions
    if (game.aliens_remaining == 0) {
        game.state = STATE_WAVE_CLEAR;
        game.wave++;
        space_invaders_play_sound("WAVE COMPLETE!");
    }
    
    if (game.player.lives <= 0) {
        game.state = STATE_GAME_OVER;
        space_invaders_play_sound("GAME OVER!");
    }
}

void space_invaders_update_aliens(void) {
    game.alien_move_timer++;
    
    // Alien movement speed based on remaining aliens and wave
    int move_speed = 50 - (game.wave * 2) + game.aliens_remaining;
    if (move_speed < 5) move_speed = 5;
    
    if (game.alien_move_timer >= move_speed) {
        game.alien_move_timer = 0;
        
        // Check if aliens need to drop down
        bool should_drop = false;
        for (int row = 0; row < ALIEN_ROWS; row++) {
            for (int col = 0; col < ALIEN_COLS; col++) {
                if (game.aliens[row][col].alive) {
                    if ((game.alien_direction == 1 && game.aliens[row][col].x >= SCREEN_WIDTH - 5) ||
                        (game.alien_direction == -1 && game.aliens[row][col].x <= 2)) {
                        should_drop = true;
                        break;
                    }
                }
            }
            if (should_drop) break;
        }
        
        if (should_drop) {
            // Drop down and change direction
            for (int row = 0; row < ALIEN_ROWS; row++) {
                for (int col = 0; col < ALIEN_COLS; col++) {
                    if (game.aliens[row][col].alive) {
                        game.aliens[row][col].y++;
                    }
                }
            }
            game.alien_direction *= -1;
            space_invaders_play_sound("ALIEN DROP!");
        } else {
            // Move horizontally
            for (int row = 0; row < ALIEN_ROWS; row++) {
                for (int col = 0; col < ALIEN_COLS; col++) {
                    if (game.aliens[row][col].alive) {
                        game.aliens[row][col].x += game.alien_direction;
                        game.aliens[row][col].animation_frame = 1 - game.aliens[row][col].animation_frame;
                    }
                }
            }
        }
    }
    
    // Alien shooting
    game.alien_shoot_timer++;
    if (game.alien_shoot_timer >= (100 - game.wave * 5)) {
        game.alien_shoot_timer = 0;
        
        // Find a random alive alien to shoot
        if (rand() % 10 == 0) { // 10% chance per update
            int attempts = 0;
            while (attempts < 50) {
                int row = rand() % ALIEN_ROWS;
                int col = rand() % ALIEN_COLS;
                
                if (game.aliens[row][col].alive) {
                    // Find empty bullet slot
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!game.bullets[i].active) {
                            game.bullets[i].x = game.aliens[row][col].x;
                            game.bullets[i].y = game.aliens[row][col].y + 1;
                            game.bullets[i].direction = -1; // Down
                            game.bullets[i].active = true;
                            game.bullets[i].type = 1; // Enemy bullet
                            
                            // Vary bullet characteristics based on alien type and wave
                            int bullet_variant = rand() % 4;
                            switch (bullet_variant) {
                                case 0: // Normal straight shot
                                    game.bullets[i].speed = 1;
                                    game.bullets[i].zigzag_offset = 0;
                                    break;
                                case 1: // Fast straight shot
                                    game.bullets[i].speed = 2;
                                    game.bullets[i].zigzag_offset = 0;
                                    break;
                                case 2: // Zigzag shot
                                    game.bullets[i].speed = 1;
                                    game.bullets[i].zigzag_offset = 1;
                                    break;
                                case 3: // Slow zigzag shot
                                    game.bullets[i].speed = 1;
                                    game.bullets[i].zigzag_offset = 2;
                                    break;
                            }
                            
                            game.bullets[i].animation_frame = 0;
                            space_invaders_play_sound("ZAP!");
                            break;
                        }
                    }
                    break;
                }
                attempts++;
            }
        }
    }
}

void space_invaders_update_bullets(void) {
    // Update all bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullets[i].active) {
            // Update animation frame for visual effects
            game.bullets[i].animation_frame++;
            
            // Move bullet based on speed and direction
            game.bullets[i].y -= game.bullets[i].direction * game.bullets[i].speed;
            
            // Add zigzag effect for enhanced bullets
            if (game.bullets[i].zigzag_offset > 0 && (game.bullets[i].animation_frame % 6) == 0) {
                int zigzag_dir = (game.bullets[i].animation_frame / 6) % 2;
                game.bullets[i].x += zigzag_dir ? 1 : -1;
            }
            
            // Remove bullets that go off screen
            if (game.bullets[i].y < 1 || game.bullets[i].y >= SCREEN_HEIGHT - 1 ||
                game.bullets[i].x < 1 || game.bullets[i].x >= SCREEN_WIDTH - 1) {
                game.bullets[i].active = false;
            }
        }
    }
}

void space_invaders_update_collisions(void) {
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!game.bullets[b].active) continue;
        
        // Bullet vs Aliens
        if (game.bullets[b].direction == 1) { // Player bullet
            for (int row = 0; row < ALIEN_ROWS; row++) {
                for (int col = 0; col < ALIEN_COLS; col++) {
                    if (game.aliens[row][col].alive &&
                        game.bullets[b].x >= game.aliens[row][col].x &&
                        game.bullets[b].x <= game.aliens[row][col].x + 2 &&
                        game.bullets[b].y == game.aliens[row][col].y) {
                        
                        // Hit!
                        game.aliens[row][col].alive = false;
                        game.aliens_remaining--;
                        game.bullets[b].active = false;
                        
                        // Create explosion effect
                        space_invaders_create_explosion(game.aliens[row][col].x, game.aliens[row][col].y, 0);
                        
                        // Score points
                        int points[] = {30, 20, 10};
                        game.score += points[game.aliens[row][col].type];
                        
                        space_invaders_play_sound("ALIEN DESTROYED!");
                        break;
                    }
                }
            }
        } else { // Alien bullet
            // Bullet vs Player
            if (game.bullets[b].x >= game.player.x - 1 &&
                game.bullets[b].x <= game.player.x + 1 &&
                game.bullets[b].y >= game.player.y - 1 &&
                game.bullets[b].y <= game.player.y + 1) {
                
                game.bullets[b].active = false;
                game.player.lives--;
                
                // Create player hit explosion
                space_invaders_create_explosion(game.player.x, game.player.y, 1);
                
                space_invaders_play_sound("PLAYER HIT!");
                
                if (game.player.lives <= 0) {
                    game.player.alive = false;
                }
            }
        }
        
        // Bullet vs Barriers
        for (int bar = 0; bar < NUM_BARRIERS; bar++) {
            if (game.bullets[b].x >= game.barriers[bar].x &&
                game.bullets[b].x < game.barriers[bar].x + BARRIER_WIDTH &&
                game.bullets[b].y >= game.barriers[bar].y &&
                game.bullets[b].y < game.barriers[bar].y + BARRIER_HEIGHT) {
                
                int rel_x = game.bullets[b].x - game.barriers[bar].x;
                int rel_y = game.bullets[b].y - game.barriers[bar].y;
                
                if (game.barriers[bar].shape[rel_y][rel_x] == '#' &&
                    !game.barriers[bar].damaged[rel_y][rel_x]) {
                    
                    game.barriers[bar].damaged[rel_y][rel_x] = true;
                    game.barriers[bar].shape[rel_y][rel_x] = ' ';
                    game.bullets[b].active = false;
                    space_invaders_play_sound("BARRIER HIT!");
                }
            }
        }
    }
}

void space_invaders_update_ufo(void) {
    if (!game.ufo.active) {
        game.ufo.spawn_timer--;
        if (game.ufo.spawn_timer <= 0) {
            game.ufo.active = true;
            game.ufo.x = (rand() % 2) ? 0 : SCREEN_WIDTH - 1;
            game.ufo.y = 2;
            game.ufo.direction = (game.ufo.x == 0) ? 1 : -1;
            game.ufo.points = 50 + (rand() % 4) * 50; // 50, 100, 150, or 200
            space_invaders_play_sound("UFO APPEARS!");
        }
    } else {
        game.ufo.x += game.ufo.direction;
        
        if (game.ufo.x < 0 || game.ufo.x >= SCREEN_WIDTH) {
            game.ufo.active = false;
            game.ufo.spawn_timer = rand() % 300 + 200;
        }
        
        // Check UFO collision with player bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (game.bullets[i].active && game.bullets[i].direction == 1 &&
                game.bullets[i].x >= game.ufo.x - 1 &&
                game.bullets[i].x <= game.ufo.x + 3 &&
                game.bullets[i].y == game.ufo.y) {
                
                game.bullets[i].active = false;
                game.score += game.ufo.points;
                game.ufo.active = false;
                game.ufo.spawn_timer = rand() % 300 + 200;
                space_invaders_play_sound("UFO DESTROYED! BONUS!");
                break;
            }
        }
    }
}

// Drawing functions
void space_invaders_draw_screen(void) {
    CLEAR_SCREEN();
    space_invaders_draw_hud();
    space_invaders_draw_aliens();
    space_invaders_draw_barriers();
    space_invaders_draw_bullets();
    space_invaders_draw_player();
    space_invaders_draw_ufo();
    space_invaders_draw_explosions();
}

void space_invaders_draw_hud(void) {
    printf("===============================================\n");
    printf("| SCORE: %06d  HI: %06d  LIVES: ", game.score, game.stats.high_score);
    for (int i = 0; i < game.player.lives; i++) {
        printf("^");
    }
    for (int i = game.player.lives; i < 3; i++) {
        printf(" ");
    }
    printf(" |\n");
    printf("| WAVE: %02d       MODE: ", game.wave);
    
    switch (game.current_mode) {
        case MODE_CLASSIC: printf("CLASSIC      |\n"); break;
        case MODE_SPEED_RUN: printf("SPEED RUN    |\n"); break;
        case MODE_FORTRESS: printf("FORTRESS     |\n"); break;
        case MODE_MARKSMAN: printf("MARKSMAN     |\n"); break;
        case MODE_ENDLESS: printf("ENDLESS      |\n"); break;
        case MODE_CUSTOM: printf("CUSTOM       |\n"); break;
    }
    printf("===============================================\n");
}

void space_invaders_draw_aliens(void) {
    for (int row = 0; row < ALIEN_ROWS; row++) {
        for (int col = 0; col < ALIEN_COLS; col++) {
            if (game.aliens[row][col].alive) {
                // Move cursor to alien position
                printf("\033[%d;%dH", game.aliens[row][col].y, game.aliens[row][col].x);
                
                // Draw alien based on type and animation frame
                if (game.aliens[row][col].type == 0) { // Bottom row - 30 points
                    printf(game.aliens[row][col].animation_frame ? "@@@" : "(@)");
                } else if (game.aliens[row][col].type == 1) { // Middle rows - 20 points
                    printf(game.aliens[row][col].animation_frame ? "###" : "|#|");
                } else { // Top rows - 10 points
                    printf(game.aliens[row][col].animation_frame ? "VVV" : "\\V/");
                }
            }
        }
    }
}

void space_invaders_draw_player(void) {
    if (game.player.alive) {
        printf("\033[%d;%dH", game.player.y, game.player.x);
        printf("^^^");
    }
}

void space_invaders_draw_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullets[i].active) {
            printf("\033[%d;%dH", game.bullets[i].y, game.bullets[i].x);
            
            char bullet_char;
            if (game.bullets[i].direction == 1) { // Player bullets
                if (game.bullets[i].speed > 1) {
                    bullet_char = '^'; // Fast bullet
                } else {
                    bullet_char = '|'; // Normal bullet
                }
            } else { // Enemy bullets
                if (game.bullets[i].zigzag_offset > 0) {
                    // Animated zigzag bullets
                    int anim_frame = (game.bullets[i].animation_frame / 4) % 3;
                    switch (anim_frame) {
                        case 0: bullet_char = '\\'; break;
                        case 1: bullet_char = '!'; break;
                        case 2: bullet_char = '/'; break;
                        default: bullet_char = '!'; break;
                    }
                } else if (game.bullets[i].speed > 1) {
                    bullet_char = 'v'; // Fast enemy bullet
                } else {
                    bullet_char = '!'; // Normal enemy bullet
                }
            }
            
            printf("%c", bullet_char);
        }
    }
}

void space_invaders_draw_barriers(void) {
    for (int i = 0; i < NUM_BARRIERS; i++) {
        for (int row = 0; row < BARRIER_HEIGHT; row++) {
            printf("\033[%d;%dH", game.barriers[i].y + row, game.barriers[i].x);
            printf("%s", game.barriers[i].shape[row]);
        }
    }
}

void space_invaders_draw_ufo(void) {
    if (game.ufo.active) {
        printf("\033[%d;%dH", game.ufo.y, game.ufo.x);
        printf("<UFO>");
    }
}

// Game mode functions
void space_invaders_main_menu(void) {
    space_invaders_display_header("MAIN MENU");
    printf("|                                           |\n");
    printf("|  [1] Classic Arcade Mode                  |\n");
    printf("|      - Original 1978 experience           |\n");
    printf("|                                           |\n");
    printf("|  [2] Speed Run Mode                       |\n");
    printf("|      - Fast-paced alien elimination       |\n");
    printf("|                                           |\n");
    printf("|  [3] Fortress Defense Mode                |\n");
    printf("|      - Protect the last city              |\n");
    printf("|                                           |\n");
    printf("|  [4] Marksman Challenge                   |\n");
    printf("|      - Precision shooting contest         |\n");
    printf("|                                           |\n");
    printf("|  [5] Endless Wave Mode                    |\n");
    printf("|      - Survive infinite alien waves       |\n");
    printf("|                                           |\n");
    printf("|  [6] Custom Setup Mode                    |\n");
    printf("|      - Design your own challenge          |\n");
    printf("|                                           |\n");
    printf("|  [7] Statistics & Records                 |\n");
    printf("|  [8] Back to Main Menu                    |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    
    printf("\nChoice (1-8): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        space_invaders_clear_input_buffer();
        return;
    }
    space_invaders_clear_input_buffer();
    
    switch (choice) {
        case 1:
            game.current_mode = MODE_CLASSIC;
            space_invaders_classic_mode();
            break;
        case 2:
            game.current_mode = MODE_SPEED_RUN;
            space_invaders_speed_run_mode();
            break;
        case 3:
            game.current_mode = MODE_FORTRESS;
            space_invaders_fortress_mode();
            break;
        case 4:
            game.current_mode = MODE_MARKSMAN;
            space_invaders_marksman_mode();
            break;
        case 5:
            game.current_mode = MODE_ENDLESS;
            space_invaders_endless_mode();
            break;
        case 6:
            game.current_mode = MODE_CUSTOM;
            space_invaders_custom_mode();
            break;
        case 7:
            space_invaders_show_statistics();
            break;
        case 8:
            game.game_running = false;
            return;
        default:
            printf("Invalid choice! Press Enter to continue...");
            getchar();
            break;
    }
}

void space_invaders_classic_mode(void) {
    space_invaders_display_header("CLASSIC ARCADE MODE");
    printf("|                                           |\n");
    printf("|  >>> ORIGINAL 1978 SPACE INVADERS <<<    |\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Defend Earth from alien invasion!       |\n");
    printf("|  Clear all waves to save humanity!       |\n");
    printf("|                                           |\n");
    printf("|  CONTROLS:                                |\n");
    printf("|  A/D - Move left/right                    |\n");
    printf("|  SPACE - Shoot                            |\n");
    printf("|  P - Pause game                           |\n");
    printf("|  ESC - Exit to menu                       |\n");
    printf("|                                           |\n");
    printf("|  SCORING:                                 |\n");
    printf("|  Top row aliens: 10 points                |\n");
    printf("|  Middle aliens: 20 points                 |\n");
    printf("|  Bottom aliens: 30 points                 |\n");
    printf("|  UFO bonus: 50-200 points                 |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    
    printf("\nPress Enter to start invasion...");
    getchar();
    
    space_invaders_init_game();
    game.state = STATE_PLAYING;
    space_invaders_game_loop();
}

void space_invaders_speed_run_mode(void) {
    space_invaders_display_header("SPEED RUN MODE");
    printf("|                                           |\n");
    printf("|  >>> RACE AGAINST TIME <<<               |\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Clear 5 waves as fast as possible!      |\n");
    printf("|  Target time: Under 2 minutes            |\n");
    printf("|                                           |\n");
    printf("|  SPECIAL FEATURES:                        |\n");
    printf("|  - Faster alien movement                  |\n");
    printf("|  - Time multiplier bonuses                |\n");
    printf("|  - Rapid fire power-ups                   |\n");
    printf("|  - Speed completion leaderboard           |\n");
    printf("|                                           |\n");
    printf("|  Current record: ");
    if (game.stats.speed_run_best_time > 0) {
        printf("%02d:%02d.%02d                |\n", 
               game.stats.speed_run_best_time / 6000,
               (game.stats.speed_run_best_time % 6000) / 100,
               game.stats.speed_run_best_time % 100);
    } else {
        printf("--:--.--                |\n");
    }
    printf("|                                           |\n");
    printf("===============================================\n");
    
    printf("\nPress Enter to start speed run...");
    getchar();
    
    space_invaders_init_game();
    game.speed_run_timer = 0;
    game.state = STATE_PLAYING;
    space_invaders_game_loop();
}

void space_invaders_fortress_mode(void) {
    space_invaders_display_header("FORTRESS DEFENSE");
    printf("|                                           |\n");
    printf("|  >>> PROTECT THE LAST CITY <<<           |\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Defend the city from alien bombers!     |\n");
    printf("|  City health decreases with each hit     |\n");
    printf("|                                           |\n");
    printf("|  SPECIAL FEATURES:                        |\n");
    printf("|  - City health bar                        |\n");
    printf("|  - Multiple weapon types                  |\n");
    printf("|  - Defensive turrets                      |\n");
    printf("|  - Boss alien motherships                 |\n");
    printf("|                                           |\n");
    printf("|  WEAPONS:                                 |\n");
    printf("|  1 - Rapid fire cannon                    |\n");
    printf("|  2 - Spread shot                          |\n");
    printf("|  3 - Laser beam                           |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    
    printf("\nPress Enter to defend the city...");
    getchar();
    
    space_invaders_init_game();
    game.fortress_city_health = 100;
    game.state = STATE_PLAYING;
    space_invaders_game_loop();
}

void space_invaders_marksman_mode(void) {
    space_invaders_display_header("MARKSMAN CHALLENGE");
    printf("|                                           |\n");
    printf("|  >>> PRECISION SHOOTING CONTEST <<<      |\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Achieve 90%% accuracy with limited ammo  |\n");
    printf("|  Hit special golden alien targets         |\n");
    printf("|                                           |\n");
    printf("|  CHALLENGE RULES:                         |\n");
    printf("|  - Only 10 shots available                |\n");
    printf("|  - Must hit 9/10 targets to pass         |\n");
    printf("|  - Bonus points for golden aliens         |\n");
    printf("|  - No wasted shots allowed!               |\n");
    printf("|                                           |\n");
    printf("|  SCORING:                                 |\n");
    printf("|  Normal alien: 50 points                  |\n");
    printf("|  Golden alien: 200 points                 |\n");
    printf("|  Perfect accuracy: 500 bonus              |\n");
    printf("|                                           |\n");
    printf("===============================================\n");
    
    printf("\nPress Enter to start marksman test...");
    getchar();
    
    space_invaders_init_game();
    game.marksman_ammo = 10;
    game.marksman_targets_hit = 0;
    game.state = STATE_PLAYING;
    space_invaders_game_loop();
}

void space_invaders_endless_mode(void) {
    space_invaders_display_header("ENDLESS WAVE MODE");
    printf("|                                           |\n");
    printf("|  >>> SURVIVE THE INFINITE INVASION <<<   |\n");
    printf("|                                           |\n");
    printf("|  OBJECTIVE:                               |\n");
    printf("|  Survive as many waves as possible!      |\n");
    printf("|  Difficulty increases exponentially       |\n");
    printf("|                                           |\n");
    printf("|  PROGRESSION SYSTEM:                      |\n");
    printf("|  - Power-ups unlock every 5 waves        |\n");
    printf("|  - New alien types appear                 |\n");
    printf("|  - Speed and aggression increase          |\n");
    printf("|  - Elite golden aliens spawn              |\n");
    printf("|                                           |\n");
    printf("|  POWER-UPS:                               |\n");
    printf("|  Wave 5: Rapid Fire                       |\n");
    printf("|  Wave 10: Shield Generator                |\n");
    printf("|  Wave 15: Triple Shot                     |\n");
    printf("|  Wave 20: Laser Cannon                    |\n");
    printf("|                                           |\n");
    printf("|  Current record: Wave %-2d                 |\n", game.stats.endless_wave_record);
    printf("===============================================\n");
    
    printf("\nPress Enter to start endless survival...");
    getchar();
    
    space_invaders_init_game();
    game.endless_difficulty = 1;
    game.state = STATE_PLAYING;
    space_invaders_game_loop();
}

void space_invaders_custom_mode(void) {
    space_invaders_display_header("CUSTOM SETUP MODE");
    printf("|                                           |\n");
    printf("|  >>> DESIGN YOUR CHALLENGE <<<           |\n");
    printf("|                                           |\n");
    printf("|  ALIEN FORMATION:                         |\n");
    printf("|  Rows: [%d]  Columns: [%d]                |\n", game.custom_alien_rows, game.custom_alien_cols);
    printf("|  Speed: [%s]                           |\n", 
           game.custom_alien_speed < 30 ? "FAST" : 
           game.custom_alien_speed < 60 ? "NORMAL" : "SLOW");
    printf("|                                           |\n");
    printf("|  SPECIAL RULES:                           |\n");
    printf("|  No Barriers: [%s]                        |\n", game.custom_no_barriers ? "YES" : "NO");
    printf("|  Infinite Ammo: [%s]                      |\n", game.custom_infinite_ammo ? "YES" : "NO");
    printf("|  Boss Mode: [%s]                          |\n", game.custom_boss_mode ? "YES" : "NO");
    printf("|                                           |\n");
    printf("|  [1] Adjust alien rows (3-8)              |\n");
    printf("|  [2] Adjust alien columns (5-15)          |\n");
    printf("|  [3] Change alien speed                    |\n");
    printf("|  [4] Toggle barriers                      |\n");
    printf("|  [5] Toggle infinite ammo                 |\n");
    printf("|  [6] Toggle boss mode                     |\n");
    printf("|  [7] START CUSTOM GAME                    |\n");
    printf("|  [8] Back to menu                         |\n");
    printf("===============================================\n");
    
    printf("\nChoice (1-8): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        space_invaders_clear_input_buffer();
        return;
    }
    space_invaders_clear_input_buffer();
    
    switch (choice) {
        case 1:
            printf("Enter alien rows (3-8): ");
            scanf("%d", &game.custom_alien_rows);
            if (game.custom_alien_rows < 3) game.custom_alien_rows = 3;
            if (game.custom_alien_rows > 8) game.custom_alien_rows = 8;
            space_invaders_clear_input_buffer();
            space_invaders_custom_mode();
            break;
        case 2:
            printf("Enter alien columns (5-15): ");
            scanf("%d", &game.custom_alien_cols);
            if (game.custom_alien_cols < 5) game.custom_alien_cols = 5;
            if (game.custom_alien_cols > 15) game.custom_alien_cols = 15;
            space_invaders_clear_input_buffer();
            space_invaders_custom_mode();
            break;
        case 3:
            game.custom_alien_speed = (game.custom_alien_speed == 20) ? 50 : 
                                     (game.custom_alien_speed == 50) ? 80 : 20;
            space_invaders_custom_mode();
            break;
        case 4:
            game.custom_no_barriers = !game.custom_no_barriers;
            space_invaders_custom_mode();
            break;
        case 5:
            game.custom_infinite_ammo = !game.custom_infinite_ammo;
            space_invaders_custom_mode();
            break;
        case 6:
            game.custom_boss_mode = !game.custom_boss_mode;
            space_invaders_custom_mode();
            break;
        case 7:
            space_invaders_init_game();
            game.state = STATE_PLAYING;
            space_invaders_game_loop();
            break;
        case 8:
            return;
        default:
            printf("Invalid choice! Press Enter to continue...");
            getchar();
            space_invaders_custom_mode();
            break;
    }
}

void space_invaders_show_statistics(void) {
    space_invaders_display_header("STATISTICS & RECORDS");
    printf("|                                           |\n");
    printf("|  OVERALL STATISTICS:                      |\n");
    printf("|  Games Played: %-26d |\n", game.stats.games_played);
    printf("|  Total Aliens Killed: %-18d |\n", game.stats.total_aliens_killed);
    printf("|  Waves Completed: %-23d |\n", game.stats.waves_completed);
    printf("|  Perfect Waves: %-25d |\n", game.stats.perfect_waves);
    printf("|                                           |\n");
    printf("|  ACCURACY:                                |\n");
    if (game.stats.accuracy_shots_fired > 0) {
        int accuracy = (game.stats.accuracy_shots_hit * 100) / game.stats.accuracy_shots_fired;
        printf("|  Shots Fired: %-27d |\n", game.stats.accuracy_shots_fired);
        printf("|  Shots Hit: %-29d |\n", game.stats.accuracy_shots_hit);
        printf("|  Accuracy: %d%%                           |\n", accuracy);
    } else {
        printf("|  No shooting data available               |\n");
    }
    printf("|                                           |\n");
    printf("|  MODE RECORDS:                            |\n");
    printf("|  Classic High Score: %-20d |\n", game.stats.classic_high_score);
    if (game.stats.speed_run_best_time > 0) {
        printf("|  Speed Run Best: %02d:%02d.%02d                 |\n",
               game.stats.speed_run_best_time / 6000,
               (game.stats.speed_run_best_time % 6000) / 100,
               game.stats.speed_run_best_time % 100);
    } else {
        printf("|  Speed Run Best: --:--.--                 |\n");
    }
    printf("|  Fortress Waves: %-24d |\n", game.stats.fortress_waves_survived);
    printf("|  Marksman Best: %d%% accuracy                |\n", game.stats.marksman_best_accuracy);
    printf("|  Endless Record: Wave %-19d |\n", game.stats.endless_wave_record);
    printf("|  Custom Challenges: %-21d |\n", game.stats.custom_challenges_created);
    printf("===============================================\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

// Main game loop
void space_invaders_game_loop(void) {
    while (game.game_running && game.state == STATE_PLAYING) {
        space_invaders_handle_input();
        space_invaders_update_game();
        space_invaders_draw_screen();
        
        SLEEP_MS(33); // ~30 FPS for smoother gameplay
    }
    
    if (game.state == STATE_GAME_OVER) {
        space_invaders_display_header("GAME OVER");
        printf("|                                           |\n");
        printf("|  >>> INVASION COMPLETE <<<               |\n");
        printf("|                                           |\n");
        printf("|  Final Score: %-27d |\n", game.score);
        printf("|  Waves Cleared: %-25d |\n", game.wave - 1);
        printf("|  Aliens Defeated: %-22d |\n", ALIEN_ROWS * ALIEN_COLS * (game.wave - 1) + (ALIEN_ROWS * ALIEN_COLS - game.aliens_remaining));
        printf("|                                           |\n");
        
        if (game.score > game.stats.high_score) {
            printf("|  [!] NEW HIGH SCORE! [!]                  |\n");
            game.stats.high_score = game.score;
            space_invaders_play_sound("NEW HIGH SCORE!");
        }
        
        printf("|                                           |\n");
        printf("|  Thank you for defending Earth!           |\n");
        printf("|                                           |\n");
        printf("===============================================\n");
        
        // Update statistics
        game.stats.games_played++;
        
        printf("\nPress Enter to return to menu...");
        getchar();
    }
}

// Main entry point
void play_space_invaders(void) {
    srand((unsigned int)time(NULL));
    
    // Initialize game statistics
    strcpy(game.stats.player_name, "Player");
    game.stats.high_score = 0;
    game.stats.games_played = 0;
    game.stats.total_aliens_killed = 0;
    game.stats.accuracy_shots_fired = 0;
    game.stats.accuracy_shots_hit = 0;
    game.stats.waves_completed = 0;
    game.stats.perfect_waves = 0;
    game.stats.classic_high_score = 0;
    game.stats.speed_run_best_time = 0;
    game.stats.fortress_waves_survived = 0;
    game.stats.marksman_best_accuracy = 0;
    game.stats.endless_wave_record = 0;
    game.stats.custom_challenges_created = 0;
    
    game.game_running = true;
    
    while (game.game_running) {
        space_invaders_main_menu();
    }
}