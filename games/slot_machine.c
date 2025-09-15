#include "games.h"
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define SLEEP(ms) Sleep(ms)
    #define CLEAR_SCREEN() system("cls")
#else
    #include <unistd.h>
    #define SLEEP(ms) usleep(ms * 1000)
    #define CLEAR_SCREEN() system("clear")
#endif

// Slot machine constants
#define MAX_CREDITS 999999
#define MIN_BET 1
#define MAX_BET 25
#define STARTING_CREDITS 100
#define STARTING_JACKPOT 1000
#define JACKPOT_CONTRIBUTION 0.01  // 1% of each bet goes to jackpot
#define REEL_POSITIONS 8

// Symbol constants
#define SYMBOL_CHERRY 0     // @@@
#define SYMBOL_LEMON 1      // ^^^
#define SYMBOL_ORANGE 2     // OOO
#define SYMBOL_STAR 3       // ***
#define SYMBOL_SEVEN 4      // 777
#define SYMBOL_BELL 5       // [B]
#define SYMBOL_DIAMOND 6    // <#>
#define SYMBOL_WILD 7       // ???

// Symbol probabilities (out of 100)
static int symbol_weights[] = {25, 20, 20, 15, 10, 7, 2, 1};

// Game state structure
typedef struct {
    int credits;
    int current_bet;
    int jackpot_amount;
    int total_spins;
    int total_bet;
    int total_won;
    int biggest_win;
    int win_streak;
    int best_streak;
    int jackpots_hit;
    int reel1, reel2, reel3;
    int last_win;
} SlotMachine;

static SlotMachine slot_game;

// Function prototypes
void init_slot_machine(void);
void display_slot_rules(void);
void display_payout_table(void);
void display_slot_interface(void);
void spin_reels(void);
void animate_spinning(void);
int check_winning_combinations(void);
void calculate_payout(int win_type);
void display_statistics(void);
void handle_jackpot_win(void);
void display_win_celebration(int amount);
char* get_symbol_display(int symbol);
char* get_symbol_name(int symbol);
int get_random_symbol(void);
void auto_play_mode(void);

// Generate three completely independent symbols
void generate_three_symbols(int *reel1, int *reel2, int *reel3) {
    *reel1 = rand() % 8;
    *reel2 = rand() % 8;
    *reel3 = rand() % 8;
}
char* get_symbol_display(int symbol) {
    static char display[4];
    switch (symbol) {
        case SYMBOL_CHERRY:  strcpy(display, "@@@"); break;
        case SYMBOL_LEMON:   strcpy(display, "^^^"); break;
        case SYMBOL_ORANGE:  strcpy(display, "OOO"); break;
        case SYMBOL_STAR:    strcpy(display, "***"); break;
        case SYMBOL_SEVEN:   strcpy(display, "777"); break;
        case SYMBOL_BELL:    strcpy(display, "[B]"); break;
        case SYMBOL_DIAMOND: strcpy(display, "<#>"); break;
        case SYMBOL_WILD:    strcpy(display, "???"); break;
        default:             strcpy(display, "   "); break;
    }
    return display;
}

// Get symbol name for messages
char* get_symbol_name(int symbol) {
    static char name[10];
    switch (symbol) {
        case SYMBOL_CHERRY:  strcpy(name, "Cherry"); break;
        case SYMBOL_LEMON:   strcpy(name, "Lemon"); break;
        case SYMBOL_ORANGE:  strcpy(name, "Orange"); break;
        case SYMBOL_STAR:    strcpy(name, "Star"); break;
        case SYMBOL_SEVEN:   strcpy(name, "Seven"); break;
        case SYMBOL_BELL:    strcpy(name, "Bell"); break;
        case SYMBOL_DIAMOND: strcpy(name, "Diamond"); break;
        case SYMBOL_WILD:    strcpy(name, "Wild"); break;
        default:             strcpy(name, "Unknown"); break;
    }
    return name;
}

// Generate random symbol based on weights
int get_random_symbol(void) {
    int random_num = rand() % 100;
    int cumulative = 0;
    
    for (int i = 0; i < 8; i++) {
        cumulative += symbol_weights[i];
        if (random_num < cumulative) {
            return i;
        }
    }
    return SYMBOL_CHERRY; // Fallback
}

// Initialize game state
void init_slot_machine(void) {
    slot_game.credits = STARTING_CREDITS;
    slot_game.current_bet = MIN_BET;
    slot_game.jackpot_amount = STARTING_JACKPOT;
    slot_game.total_spins = 0;
    slot_game.total_bet = 0;
    slot_game.total_won = 0;
    slot_game.biggest_win = 0;
    slot_game.win_streak = 0;
    slot_game.best_streak = 0;
    slot_game.jackpots_hit = 0;
    slot_game.reel1 = SYMBOL_CHERRY;
    slot_game.reel2 = SYMBOL_LEMON;
    slot_game.reel3 = SYMBOL_ORANGE;
    slot_game.last_win = 0;
    
    srand(time(NULL));
}

// Display game rules
void display_slot_rules(void) {
    printf("\n+==========================================+\n");
    printf("|            ASCII SLOT MACHINE           |\n");
    printf("+==========================================+\n");
    printf("| SYMBOL REFERENCE:                        |\n");
    printf("| Cherry: @@@  Lemon:  ^^^  Orange: OOO   |\n");
    printf("| Star:   ***  Seven:  777  Bell:   [B]   |\n");
    printf("| Diamond:<#>  Wild:   ???                 |\n");
    printf("|                                          |\n");
    printf("| HOW TO PLAY:                             |\n");
    printf("| * Choose your bet amount (1-25 credits) |\n");
    printf("| * Spin the reels and match symbols      |\n");
    printf("| * Win credits based on combinations     |\n");
    printf("| * Hit 3 Wilds on max bet for JACKPOT!   |\n");
    printf("|                                          |\n");
    printf("| CONTROLS:                                |\n");
    printf("| [S] Spin Reels     [B] Change Bet       |\n");
    printf("| [T] Payout Table   [A] Auto-Play        |\n");
    printf("| [Stats] Statistics [Q] Quit              |\n");
    printf("+==========================================+\n");
    printf("\nPress any key to start playing...");
    getchar();
}

// Display payout table
void display_payout_table(void) {
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|              PAYOUT TABLE                |\n");
    printf("+==========================================+\n");
    printf("| THREE OF A KIND:                         |\n");
    printf("| Three Cherries:     50x bet              |\n");
    printf("| Three Sevens:       100x bet             |\n");
    printf("| Three Diamonds:     200x bet             |\n");
    printf("| Three Stars:        500x bet             |\n");
    printf("| Three Wilds:        1000x bet            |\n");
    printf("|                                          |\n");
    printf("| TWO OF A KIND:                           |\n");
    printf("| Two Cherries:       5x bet               |\n");
    printf("| Two Sevens:         10x bet              |\n");
    printf("| Cherry + Wild:      25x bet              |\n");
    printf("| Any Two + Wild:     15x bet              |\n");
    printf("|                                          |\n");
    printf("| SINGLE SYMBOLS:                          |\n");
    printf("| One Cherry:         2x bet               |\n");
    printf("| One Wild:           3x bet               |\n");
    printf("|                                          |\n");
    printf("| JACKPOT: 3 Wilds on Max Bet (25)        |\n");
    printf("| Current Jackpot: %d credits              |\n", slot_game.jackpot_amount);
    printf("+==========================================+\n");
    printf("\nPress any key to continue...");
    getchar();
}

// Display main game interface
void display_slot_interface(void) {
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|            ASCII SLOT MACHINE           |\n");
    printf("+==========================================+\n");
    printf("| Credits: %-6d  Bet: %-2d  Last Win: %-4d |\n", 
           slot_game.credits, slot_game.current_bet, slot_game.last_win);
    printf("| Jackpot: %-6d  Spins: %-4d            |\n", 
           slot_game.jackpot_amount, slot_game.total_spins);
    printf("+==========================================+\n");
    printf("\n");
    printf("    REEL 1    REEL 2    REEL 3\n");
    printf("   +------+  +------+  +------+\n");
    printf("   |  %s  |  |  %s  |  |  %s  |\n", 
           get_symbol_display(slot_game.reel1),
           get_symbol_display(slot_game.reel2),
           get_symbol_display(slot_game.reel3));
    printf("   +------+  +------+  +------+\n");
    printf("\n");
    
    // Show win message if there was a win
    if (slot_game.last_win > 0) {
        if (slot_game.last_win >= 1000) {
            printf("         *** JACKPOT! JACKPOT! ***\n");
            printf("         You won %d credits!\n", slot_game.last_win);
        } else if (slot_game.last_win >= 100) {
            printf("         *** BIG WIN! ***\n");
            printf("         You won %d credits!\n", slot_game.last_win);
        } else if (slot_game.last_win >= 20) {
            printf("         ** Nice Win! **\n");
            printf("         You won %d credits!\n", slot_game.last_win);
        } else {
            printf("         * Winner! *\n");
            printf("         You won %d credits!\n", slot_game.last_win);
        }
    } else {
        printf("         Good luck on your next spin!\n");
    }
    
    printf("\n");
    printf("[S]pin (%d credits) | [B]et Amount | [T]able\n", slot_game.current_bet);
    printf("[A]uto-Play | [R]eport Stats | [Q]uit\n");
    printf("\nChoice: ");
}

// Animate spinning reels
void animate_spinning(void) {
    printf("\n    REEL 1    REEL 2    REEL 3\n");
    
    for (int frame = 0; frame < 15; frame++) {
        printf("   +------+  +------+  +------+\n");
        
        // Generate random symbols for animation
        int temp1 = rand() % 8;
        int temp2 = rand() % 8;
        int temp3 = rand() % 8;
        
        printf("   |  %s  |  |  %s  |  |  %s  |\n", 
               get_symbol_display(temp1),
               get_symbol_display(temp2),
               get_symbol_display(temp3));
        printf("   +------+  +------+  +------+\n");
        
        SLEEP(100); // 100ms delay
        
        // Move cursor back up to overwrite
        if (frame < 14) {
            printf("\033[3A"); // Move up 3 lines
        }
    }
    
    SLEEP(500); // Pause before showing final result
}

// Spin the reels
void spin_reels(void) {
    if (slot_game.credits < slot_game.current_bet) {
        printf("\nInsufficient credits! You need %d credits to spin.\n", slot_game.current_bet);
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Deduct bet and update jackpot
    slot_game.credits -= slot_game.current_bet;
    slot_game.total_bet += slot_game.current_bet;
    slot_game.total_spins++;
    slot_game.jackpot_amount += (int)(slot_game.current_bet * JACKPOT_CONTRIBUTION);
    
    printf("\nSpinning the reels...\n");
    animate_spinning();
    
    // Generate final reel results using simplified independent generation
    generate_three_symbols(&slot_game.reel1, &slot_game.reel2, &slot_game.reel3);
    
    // Check for wins
    int win_type = check_winning_combinations();
    if (win_type > 0) {
        calculate_payout(win_type);
        slot_game.win_streak++;
        if (slot_game.win_streak > slot_game.best_streak) {
            slot_game.best_streak = slot_game.win_streak;
        }
    } else {
        slot_game.last_win = 0;
        slot_game.win_streak = 0;
    }
    
    printf("\nPress any key to continue...");
    getchar();
}

// Check winning combinations
int check_winning_combinations(void) {
    int r1 = slot_game.reel1;
    int r2 = slot_game.reel2;
    int r3 = slot_game.reel3;
    
    // Count wilds for substitution
    int wild_count = 0;
    if (r1 == SYMBOL_WILD) wild_count++;
    if (r2 == SYMBOL_WILD) wild_count++;
    if (r3 == SYMBOL_WILD) wild_count++;
    
    // Check for three wilds (jackpot)
    if (r1 == SYMBOL_WILD && r2 == SYMBOL_WILD && r3 == SYMBOL_WILD) {
        if (slot_game.current_bet == MAX_BET) {
            return 100; // Jackpot
        } else {
            return 11; // Three wilds but not max bet
        }
    }
    
    // Check for three of a kind
    if ((r1 == r2 && r2 == r3) || 
        (wild_count >= 1 && ((r1 == r2) || (r1 == r3) || (r2 == r3)))) {
        if (r1 == SYMBOL_CHERRY || r2 == SYMBOL_CHERRY || r3 == SYMBOL_CHERRY) return 1;
        if (r1 == SYMBOL_SEVEN || r2 == SYMBOL_SEVEN || r3 == SYMBOL_SEVEN) return 2;
        if (r1 == SYMBOL_DIAMOND || r2 == SYMBOL_DIAMOND || r3 == SYMBOL_DIAMOND) return 3;
        if (r1 == SYMBOL_STAR || r2 == SYMBOL_STAR || r3 == SYMBOL_STAR) return 4;
        return 5; // Other three of a kind
    }
    
    // Check for two of a kind
    if (r1 == r2 || r1 == r3 || r2 == r3 || wild_count >= 1) {
        if ((r1 == SYMBOL_CHERRY || r2 == SYMBOL_CHERRY || r3 == SYMBOL_CHERRY) && wild_count >= 1) return 8;
        if (r1 == SYMBOL_CHERRY && r2 == SYMBOL_CHERRY) return 6;
        if (r1 == SYMBOL_SEVEN && r2 == SYMBOL_SEVEN) return 7;
        if (wild_count >= 1) return 9; // Any two + wild
    }
    
    // Check for single special symbols
    if (r1 == SYMBOL_CHERRY || r2 == SYMBOL_CHERRY || r3 == SYMBOL_CHERRY) return 10;
    if (r1 == SYMBOL_WILD || r2 == SYMBOL_WILD || r3 == SYMBOL_WILD) return 12;
    
    return 0; // No win
}

// Calculate payout based on win type
void calculate_payout(int win_type) {
    int payout = 0;
    
    switch (win_type) {
        case 1:  payout = slot_game.current_bet * 50; break;  // Three cherries
        case 2:  payout = slot_game.current_bet * 100; break; // Three sevens
        case 3:  payout = slot_game.current_bet * 200; break; // Three diamonds
        case 4:  payout = slot_game.current_bet * 500; break; // Three stars
        case 5:  payout = slot_game.current_bet * 25; break;  // Other three of a kind
        case 6:  payout = slot_game.current_bet * 5; break;   // Two cherries
        case 7:  payout = slot_game.current_bet * 10; break;  // Two sevens
        case 8:  payout = slot_game.current_bet * 25; break;  // Cherry + wild
        case 9:  payout = slot_game.current_bet * 15; break;  // Any two + wild
        case 10: payout = slot_game.current_bet * 2; break;   // One cherry
        case 11: payout = slot_game.current_bet * 1000; break; // Three wilds (no jackpot)
        case 12: payout = slot_game.current_bet * 3; break;   // One wild
        case 100: // Jackpot
            payout = slot_game.jackpot_amount;
            slot_game.jackpots_hit++;
            slot_game.jackpot_amount = STARTING_JACKPOT; // Reset jackpot
            break;
        default: payout = 0; break;
    }
    
    slot_game.last_win = payout;
    slot_game.credits += payout;
    slot_game.total_won += payout;
    
    if (payout > slot_game.biggest_win) {
        slot_game.biggest_win = payout;
    }
}

// Display statistics
void display_statistics(void) {
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|              SLOT STATISTICS             |\n");
    printf("+==========================================+\n");
    printf("| Total Spins: %-4d                       |\n", slot_game.total_spins);
    printf("| Total Bet: %-6d credits                |\n", slot_game.total_bet);
    printf("| Total Won: %-6d credits                |\n", slot_game.total_won);
    
    int net_profit = slot_game.total_won - slot_game.total_bet;
    printf("| Net Profit: %-6d credits               |\n", net_profit);
    printf("|                                          |\n");
    printf("| Biggest Win: %-6d credits              |\n", slot_game.biggest_win);
    
    if (slot_game.total_spins > 0) {
        float win_percentage = (float)(slot_game.total_won) / slot_game.total_bet * 100;
        printf("| Win Percentage: %.1f%%                    |\n", win_percentage);
    }
    
    printf("| Current Streak: %-3d                    |\n", slot_game.win_streak);
    printf("| Best Streak: %-3d                       |\n", slot_game.best_streak);
    printf("|                                          |\n");
    printf("| Jackpots Hit: %-2d                       |\n", slot_game.jackpots_hit);
    printf("| Current Jackpot: %-6d                  |\n", slot_game.jackpot_amount);
    printf("+==========================================+\n");
    printf("\nPress any key to continue...");
    getchar();
}

// Auto-play mode with real-time animations
void auto_play_mode(void) {
    int auto_spins;
    printf("\nAUTO-PLAY MODE\n");
    printf("==============\n");
    printf("How many automatic spins? (1-100): ");
    if (scanf("%d", &auto_spins) != 1 || auto_spins < 1 || auto_spins > 100) {
        printf("Invalid number! Using 10 spins.\n");
        auto_spins = 10;
    }
    clear_input_buffer();
    
    printf("\nStarting Auto-Play with %d spins...\n", auto_spins);
    printf("Press Enter to stop early or wait for completion.\n");
    SLEEP(1500); // Brief pause before starting
    
    for (int i = 0; i < auto_spins && slot_game.credits >= slot_game.current_bet; i++) {
        CLEAR_SCREEN();
        printf("\n+==========================================+\n");
        printf("|            AUTO-PLAY MODE                |\n");
        printf("+==========================================+\n");
        printf("| Spin: %d/%-2d                             |\n", i + 1, auto_spins);
        printf("| Credits: %-6d  Bet: %-2d               |\n", slot_game.credits, slot_game.current_bet);
        printf("| Jackpot: %-6d                          |\n", slot_game.jackpot_amount);
        printf("+==========================================+\n");
        
        // Show spinning message
        printf("\n*** SPINNING REELS ***\n\n");
        
        // Enhanced spinning animation (simplified)
        for (int frame = 0; frame < 15; frame++) {
            printf("    REEL 1    REEL 2    REEL 3\n");
            printf("   +------+  +------+  +------+\n");
            
            // Generate animation symbols using simple randomness
            int temp1 = rand() % 8;
            int temp2 = rand() % 8;
            int temp3 = rand() % 8;
            
            printf("   |  %s  |  |  %s  |  |  %s  |\n", 
                   get_symbol_display(temp1),
                   get_symbol_display(temp2),
                   get_symbol_display(temp3));
            printf("   +------+  +------+  +------+\n");
            
            printf("\nSpinning... (%d/%d)\n", i + 1, auto_spins);
            
            SLEEP(120); // Realistic spinning speed
            
            // Clear and redraw
            #ifdef _WIN32
            system("cls");
            printf("\n+==========================================+\n");
            printf("|            AUTO-PLAY MODE                |\n");
            printf("+==========================================+\n");
            printf("| Spin: %d/%-2d                             |\n", i + 1, auto_spins);
            printf("| Credits: %-6d  Bet: %-2d               |\n", slot_game.credits, slot_game.current_bet);
            printf("| Jackpot: %-6d                          |\n", slot_game.jackpot_amount);
            printf("+==========================================+\n\n");
            printf("*** SPINNING REELS ***\n\n");
            #endif
        }
        
        // Deduct bet and update counters
        slot_game.credits -= slot_game.current_bet;
        slot_game.total_bet += slot_game.current_bet;
        slot_game.total_spins++;
        slot_game.jackpot_amount += (int)(slot_game.current_bet * JACKPOT_CONTRIBUTION);
        
        // Generate final results using simplified independent generation
        generate_three_symbols(&slot_game.reel1, &slot_game.reel2, &slot_game.reel3);
        
        // Show final result
        CLEAR_SCREEN();
        printf("\n+==========================================+\n");
        printf("|            AUTO-PLAY RESULT             |\n");
        printf("+==========================================+\n");
        printf("| Spin: %d/%-2d                             |\n", i + 1, auto_spins);
        printf("| Credits: %-6d  Bet: %-2d               |\n", slot_game.credits, slot_game.current_bet);
        printf("| Jackpot: %-6d                          |\n", slot_game.jackpot_amount);
        printf("+==========================================+\n\n");
        
        printf("    REEL 1    REEL 2    REEL 3\n");
        printf("   +------+  +------+  +------+\n");
        printf("   |  %s  |  |  %s  |  |  %s  |\n", 
               get_symbol_display(slot_game.reel1),
               get_symbol_display(slot_game.reel2),
               get_symbol_display(slot_game.reel3));
        printf("   +------+  +------+  +------+\n\n");
        
        // Check for wins and display results
        int win_type = check_winning_combinations();
        if (win_type > 0) {
            calculate_payout(win_type);
            slot_game.win_streak++;
            if (slot_game.win_streak > slot_game.best_streak) {
                slot_game.best_streak = slot_game.win_streak;
            }
            
            // Clean win celebration
            if (slot_game.last_win >= 1000) {
                printf("*** JACKPOT! JACKPOT! JACKPOT! ***\n");
                printf("*** WON %d CREDITS! ***\n", slot_game.last_win);
                printf("*** AMAZING WIN! ***\n");
            } else if (slot_game.last_win >= 100) {
                printf("*** BIG WIN! ***\n");
                printf("WON %d CREDITS!\n", slot_game.last_win);
            } else if (slot_game.last_win >= 20) {
                printf("*** NICE WIN! ***\n");
                printf("WON %d CREDITS!\n", slot_game.last_win);
            } else {
                printf("*** WINNER! ***\n");
                printf("Won %d credits!\n", slot_game.last_win);
            }
            
            // Show symbol combination
            printf("Combination: %s %s %s\n", 
                   get_symbol_display(slot_game.reel1),
                   get_symbol_display(slot_game.reel2),
                   get_symbol_display(slot_game.reel3));
                   
        } else {
            slot_game.last_win = 0;
            slot_game.win_streak = 0;
            printf("No win this time...\n");
            printf("Better luck on the next spin!\n");
        }
        
        // Show current session stats
        printf("\n--- CURRENT SESSION ---\n");
        printf("Total Spins: %d\n", slot_game.total_spins);
        printf("Total Bet: %d credits\n", slot_game.total_bet);
        printf("Total Won: %d credits\n", slot_game.total_won);
        printf("Net: %s%d credits\n", 
               (slot_game.total_won - slot_game.total_bet >= 0) ? "+" : "",
               slot_game.total_won - slot_game.total_bet);
        
        if (slot_game.win_streak > 0) {
            printf("Win Streak: %d\n", slot_game.win_streak);
        }
        
        // Pause between spins
        if (slot_game.last_win > 0) {
            SLEEP(2500); // 2.5 seconds for wins
        } else {
            SLEEP(1800); // 1.8 seconds for losses
        }
        
        // Check if out of credits
        if (slot_game.credits < slot_game.current_bet) {
            printf("\n*** OUT OF CREDITS! ***\n");
            printf("Auto-play ended at spin %d/%d\n", i + 1, auto_spins);
            break;
        }
    }
    
    // Final auto-play summary
    CLEAR_SCREEN();
    printf("\n+==========================================+\n");
    printf("|           AUTO-PLAY COMPLETE!           |\n");
    printf("+==========================================+\n");
    printf("| Spins Completed: %-3d                    |\n", slot_game.total_spins);
    printf("| Final Credits: %-6d                   |\n", slot_game.credits);
    printf("| Session Net: %s%-6d credits            |\n",
           (slot_game.total_won - slot_game.total_bet >= 0) ? "+" : "",
           slot_game.total_won - slot_game.total_bet);
    printf("| Best Win: %-6d credits                |\n", slot_game.biggest_win);
    printf("| Best Streak: %-3d wins                  |\n", slot_game.best_streak);
    printf("+==========================================+\n");
    
    printf("\nPress any key to return to manual mode...");
    getchar();
}

// Main slot machine game function
void play_slot_machine(void) {
    printf("\n+==========================================+\n");
    printf("|        [SLOTS] SLOT MACHINE [LUCK]       |\n");
    printf("+==========================================+\n");
    
    display_slot_rules();
    init_slot_machine();
    
    char choice;
    
    while (slot_game.credits > 0) {
        display_slot_interface();
        
        choice = getchar();
        clear_input_buffer();
        
        switch (choice) {
            case 's':
            case 'S':
                spin_reels();
                break;
                
            case 'b':
            case 'B':
                printf("\nCurrent bet: %d\n", slot_game.current_bet);
                printf("Enter new bet amount (1-25): ");
                int new_bet;
                if (scanf("%d", &new_bet) == 1 && new_bet >= MIN_BET && new_bet <= MAX_BET) {
                    slot_game.current_bet = new_bet;
                    printf("Bet set to %d credits.\n", new_bet);
                } else {
                    printf("Invalid bet! Keeping current bet of %d.\n", slot_game.current_bet);
                }
                clear_input_buffer();
                printf("Press any key to continue...");
                getchar();
                break;
                
            case 't':
            case 'T':
                display_payout_table();
                break;
                
            case 'a':
            case 'A':
                auto_play_mode();
                break;
                
            case 'r':
            case 'R':
                display_statistics();
                break;
                
            case 'q':
            case 'Q':
                printf("\nThanks for playing! Final credits: %d\n", slot_game.credits);
                return;
                
            default:
                printf("\nInvalid choice! Press any key to continue...");
                getchar();
                break;
        }
    }
    
    printf("\nGame Over! You're out of credits.\n");
    printf("Better luck next time!\n");
}