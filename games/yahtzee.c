/*
 * YAHTZEE - Classic Dice Game
 * CLI Games Pack v2.1
 * 
 * A complete implementation of the classic Yahtzee dice game featuring:
 * - Traditional 13-category scorecard
 * - 3 rolls per turn with dice keeping mechanics
 * - Upper section bonus calculation
 * - Yahtzee bonus scoring
 * - Strategy hints and optimal play suggestions
 * - Full ASCII dice graphics
 * - Cross-platform compatibility
 */

#include "games.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP_MS(ms) Sleep(ms)
    #define YAHTZEE_KBHIT() games_kbhit()
    #define GETCH() _getch()
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define YAHTZEE_KBHIT() games_kbhit()
    #define GETCH() getchar()
#endif

// Game Constants
#define NUM_DICE 5
#define NUM_CATEGORIES 13
#define NUM_ROUNDS 13
#define MAX_ROLLS 3
#define UPPER_BONUS_THRESHOLD 63
#define UPPER_BONUS_POINTS 35
#define YAHTZEE_BONUS 100

// Score Categories
typedef enum {
    ONES = 0, TWOS, THREES, FOURS, FIVES, SIXES,           // Upper section
    THREE_OF_KIND, FOUR_OF_KIND, FULL_HOUSE, SM_STRAIGHT,  // Lower section
    LG_STRAIGHT, YAHTZEE_CAT, CHANCE
} YahtzeeCategory;

// Game Structures
typedef struct {
    int values[NUM_DICE];
    bool keep[NUM_DICE];
} YahtzeeDice;

typedef struct {
    int scores[NUM_CATEGORIES];
    bool used[NUM_CATEGORIES];
    int upper_total;
    int upper_bonus;
    int lower_total;
    int yahtzee_bonuses;
    int grand_total;
} YahtzeeScorecard;

typedef struct {
    YahtzeeDice dice;
    YahtzeeScorecard scorecard;
    int current_round;
    int rolls_left;
    bool game_over;
} YahtzeeGame;

// Category names and scoring rules
const char* category_names[NUM_CATEGORIES] = {
    "Ones", "Twos", "Threes", "Fours", "Fives", "Sixes",
    "3 of a Kind", "4 of a Kind", "Full House", "Sm. Straight",
    "Lg. Straight", "YAHTZEE", "Chance"
};

const int category_points[NUM_CATEGORIES] = {
    1, 2, 3, 4, 5, 6,           // Upper section values
    0, 0, 25, 30, 40, 50, 0     // Lower section fixed points (0 = sum of dice)
};

// Global game instance
YahtzeeGame game;

// Function Declarations
void yahtzee_init_game(void);
void yahtzee_display_header(void);
void yahtzee_display_dice(void);
void yahtzee_display_scorecard(void);
void yahtzee_roll_dice(void);
void yahtzee_select_dice(void);
void yahtzee_score_turn(void);
int yahtzee_calculate_score(YahtzeeCategory category);
bool yahtzee_is_valid_category(YahtzeeCategory category);
void yahtzee_apply_score(YahtzeeCategory category, int score);
void yahtzee_calculate_totals(void);
void yahtzee_show_rules(void);
void yahtzee_show_strategy(void);
void yahtzee_final_results(void);
char* yahtzee_dice_art(int value);
void yahtzee_animate_roll(void);

// Utility functions for scoring
int count_dice(int target);
bool has_straight(int length);
bool has_of_kind(int count);
int sum_all_dice(void);
void yahtzee_analyze_dice_and_suggest(void);

// Initialize new game
void yahtzee_init_game(void) {
    // Reset dice
    for (int i = 0; i < NUM_DICE; i++) {
        game.dice.values[i] = 1;
        game.dice.keep[i] = false;
    }
    
    // Reset scorecard
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        game.scorecard.scores[i] = 0;
        game.scorecard.used[i] = false;
    }
    game.scorecard.upper_total = 0;
    game.scorecard.upper_bonus = 0;
    game.scorecard.lower_total = 0;
    game.scorecard.yahtzee_bonuses = 0;
    game.scorecard.grand_total = 0;
    
    // Reset game state
    game.current_round = 1;
    game.rolls_left = MAX_ROLLS;
    game.game_over = false;
}

// Display game header with round info
void yahtzee_display_header(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("||    Y   Y   AA   H   H  TTTTT  ZZZZ  EEEEE  EEEEE    ||   DICE MASTER   ||\n");
    printf("||     Y Y   A  A  H   H    T      Z   E      E        ||                 ||\n");
    printf("||      Y    AAAA  HHHHH    T     Z    EEEE   EEEE     ||  Round %2d / 13   ||\n", game.current_round);
    printf("||      Y    A  A  H   H    T    Z     E      E        ||  Rolls Left: %d  ||\n", game.rolls_left);
    printf("||      Y    A  A  H   H    T   ZZZZ  EEEEE  EEEEE    ||  Score: %4d    ||\n", game.scorecard.grand_total);
    printf("================================================================================\n");
    
    // Show progress bar
    printf("Progress: [");
    for (int i = 1; i <= 13; i++) {
        if (i <= game.current_round - 1) printf("#");
        else if (i == game.current_round) printf(">");
        else printf("-");
    }
    printf("] %d%%\n", (game.current_round - 1) * 100 / 13);
    printf("================================================================================\n");
}

// ASCII art for dice faces
char* yahtzee_dice_art(int value) {
    static char dice_faces[7][50];  // Increased size to prevent overflow
    
    strcpy(dice_faces[1], "+-----+\n|     |\n|  *  |\n|     |\n+-----+");
    strcpy(dice_faces[2], "+-----+\n| *   |\n|     |\n|   * |\n+-----+");
    strcpy(dice_faces[3], "+-----+\n| *   |\n|  *  |\n|   * |\n+-----+");
    strcpy(dice_faces[4], "+-----+\n| * * |\n|     |\n| * * |\n+-----+");
    strcpy(dice_faces[5], "+-----+\n| * * |\n|  *  |\n| * * |\n+-----+");
    strcpy(dice_faces[6], "+-----+\n| * * |\n| * * |\n| * * |\n+-----+");
    
    return dice_faces[value];
}

// Display current dice with enhanced visual formatting
void yahtzee_display_dice(void) {
    printf("\n+======================== CURRENT DICE ========================+\n");
    printf("|                                                          |\n");
    
    // Show dice values with ASCII art
    printf("|   ");
    for (int i = 0; i < NUM_DICE; i++) {
        if (game.dice.keep[i]) {
            printf("[%d]KEEP", game.dice.values[i]);
        } else {
            printf(" [%d]   ", game.dice.values[i]);
        }
        if (i < NUM_DICE - 1) printf("  ");
    }
    printf("   |\n");
    
    // Show visual indicators
    printf("|   ");
    for (int i = 0; i < NUM_DICE; i++) {
        if (game.dice.keep[i]) {
            printf(" ^^^^ ");
        } else {
            printf("  --  ");
        }
        if (i < NUM_DICE - 1) printf("  ");
    }
    printf("   |\n");
    
    // Show dice positions
    printf("|    1      2      3      4      5                        |\n");
    printf("|                                                          |\n");
    
    // Calculate and show total
    int total = 0;
    for (int i = 0; i < NUM_DICE; i++) {
        total += game.dice.values[i];
    }
    printf("|  Total Value: %2d    Kept: %d dice    Free: %d dice      |\n", 
           total, 
           game.dice.keep[0] + game.dice.keep[1] + game.dice.keep[2] + game.dice.keep[3] + game.dice.keep[4],
           5 - (game.dice.keep[0] + game.dice.keep[1] + game.dice.keep[2] + game.dice.keep[3] + game.dice.keep[4]));
    printf("+===========================================================+\n");
}

// Enhanced scorecard display with visual indicators
void yahtzee_display_scorecard(void) {
    printf("\n+========================= SCORECARD =========================+\n");
    printf("| UPPER SECTION                   | LOWER SECTION            |\n");
    printf("|=================================|==========================|\n");
    
    // Calculate upper section progress for bonus
    int upper_progress = 0;
    for (int i = 0; i <= SIXES; i++) {
        if (game.scorecard.used[i]) {
            upper_progress += game.scorecard.scores[i];
        }
    }
    
    // Display upper and lower sections side by side
    for (int i = 0; i < 6; i++) {
        // Upper section with visual indicators
        char status_char = game.scorecard.used[i] ? '*' : ' ';
        printf("| %c%d. %-11s ", status_char, i + 1, category_names[i]);
        if (game.scorecard.used[i]) {
            printf("%3d     |", game.scorecard.scores[i]);
        } else {
            printf("---     |");
        }
        
        // Lower section
        int lower_idx = i + 6;
        if (lower_idx < NUM_CATEGORIES) {
            char lower_status = game.scorecard.used[lower_idx] ? '*' : ' ';
            printf(" %c%d. %-11s ", lower_status, lower_idx + 1, category_names[lower_idx]);
            if (game.scorecard.used[lower_idx]) {
                printf("%3d  |\n", game.scorecard.scores[lower_idx]);
            } else {
                printf("---  |\n");
            }
        } else {
            printf("                          |\n");
        }
    }
    
    // Show 13th category (Chance) separately
    printf("|                                 |");
    char chance_status = game.scorecard.used[12] ? '*' : ' ';
    printf(" %c13. %-11s ", chance_status, category_names[12]);
    if (game.scorecard.used[12]) {
        printf("%3d  |\n", game.scorecard.scores[12]);
    } else {
        printf("---  |\n");
    }
    
    printf("|=================================|==========================|\n");
    
    // Enhanced totals section with progress indicators
    printf("| Upper Total:      %3d           | Lower Total:     %3d     |\n", 
           game.scorecard.upper_total, game.scorecard.lower_total);
    
    // Upper bonus progress
    if (game.scorecard.upper_bonus > 0) {
        printf("| Upper Bonus:      %3d  [EARNED] |", game.scorecard.upper_bonus);
    } else {
        int needed = UPPER_BONUS_THRESHOLD - upper_progress;
        if (needed <= 0) {
            printf("| Upper Bonus:       35  [READY!] |");
        } else {
            printf("| Upper Bonus:     (need %2d more) |", needed);
        }
    }
    
    printf(" Yahtzee Bonuses: %3d     |\n", game.scorecard.yahtzee_bonuses * YAHTZEE_BONUS);
    
    printf("|                                 |                          |\n");
    printf("|       GRAND TOTAL: %4d         |   [* = completed]        |\n", 
           game.scorecard.grand_total);
    printf("+===========================================================+\n");
    
    // Show completion progress
    int completed_categories = 0;
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        if (game.scorecard.used[i]) completed_categories++;
    }
    printf("Categories completed: %d/13 (%.0f%%)\n", 
           completed_categories, (completed_categories * 100.0) / 13);
}

// Animate dice rolling with enhanced visual feedback
void yahtzee_animate_roll(void) {
    const char* roll_frames[] = {
        "Rolling   [     ]",
        "Rolling   [*    ]", 
        "Rolling   [**   ]",
        "Rolling   [***  ]",
        "Rolling   [**** ]",
        "Rolling   [*****]",
        "Rolling   [****#]",
        "Rolling   [***##]",
        "Rolling   [**###]",
        "Rolling   [*####]",
        "Rolling   [#####]"
    };
    
    printf("\n");
    for (int frame = 0; frame < 11; frame++) {
        printf("\r%s", roll_frames[frame]);
        fflush(stdout);
        SLEEP_MS(120);
    }
    
    printf("\r>> DICE ROLLED! <<           \n");
    
    // Show some excitement based on what was rolled
    int total = 0;
    for (int i = 0; i < NUM_DICE; i++) {
        total += game.dice.values[i];
    }
    
    if (total >= 25) {
        printf("   *** EXCELLENT ROLL! ***\n");
    } else if (total >= 20) {
        printf("   ** Good roll! **\n");
    } else {
        printf("   * Roll complete *\n");
    }
}

// Enhanced dice rolling with sound effects and anticipation
void yahtzee_roll_dice(void) {
    if (game.rolls_left <= 0) {
        printf("\n*** NO ROLLS REMAINING! ***\n");
        printf("You must choose a scoring category to continue.\n");
        return;
    }
    
    // Count dice to be rolled
    int dice_to_roll = 0;
    for (int i = 0; i < NUM_DICE; i++) {
        if (!game.dice.keep[i]) dice_to_roll++;
    }
    
    if (dice_to_roll == 0) {
        printf("\n*** ALL DICE ARE KEPT! ***\n");
        printf("All your dice are marked for keeping. Choose a scoring category.\n");
        return;
    }
    
    printf("\n+=============== ROLLING %d DICE ===============+\n", dice_to_roll);
    printf("|  Get ready for the roll...                  |\n");
    printf("+=============================================+\n");
    
    // Sound effects simulation
    printf("\n*shake shake shake*\n");
    SLEEP_MS(300);
    
    yahtzee_animate_roll();
    
    // Roll the dice
    for (int i = 0; i < NUM_DICE; i++) {
        if (!game.dice.keep[i]) {
            game.dice.values[i] = (rand() % 6) + 1;
        }
    }
    
    game.rolls_left--;
    
    // Analyze the roll for excitement
    bool has_yahtzee = true;
    int first_val = game.dice.values[0];
    for (int i = 1; i < NUM_DICE; i++) {
        if (game.dice.values[i] != first_val) {
            has_yahtzee = false;
            break;
        }
    }
    
    bool has_four_kind = has_of_kind(4);
    bool has_full_house_potential = false;
    
    // Check for full house
    int counts[7] = {0};
    for (int i = 0; i < NUM_DICE; i++) {
        counts[game.dice.values[i]]++;
    }
    bool has_three = false, has_two = false;
    for (int i = 1; i <= 6; i++) {
        if (counts[i] == 3) has_three = true;
        if (counts[i] == 2) has_two = true;
    }
    has_full_house_potential = has_three && has_two;
    
    // Excitement based on roll quality
    if (has_yahtzee) {
        printf("\n\n*** Y A H T Z E E ! ! ! ***\n");
        printf("*** INCREDIBLE! ALL FIVE DICE MATCH! ***\n");
        printf("*clap clap clap* *cheering sounds*\n\n");
    } else if (has_four_kind) {
        printf("\n** FOUR OF A KIND! **\n");
        printf("*excited cheering*\n\n");
    } else if (has_full_house_potential) {
        printf("\n** FULL HOUSE! **\n");
        printf("*applause*\n\n");
    } else if (has_straight(5)) {
        printf("\n** LARGE STRAIGHT! **\n");
        printf("*whistling sounds*\n\n");
    } else if (has_straight(4)) {
        printf("\n* Small Straight! *\n");
        printf("*nice roll sounds*\n\n");
    }
    
    printf("Rolls remaining: %d\n", game.rolls_left);
    
    if (game.rolls_left == 0) {
        printf("\n>>> FINAL ROLL! Time to score these dice. <<<\n");
    }
}

// Enhanced dice selection with better UX
void yahtzee_select_dice(void) {
    if (game.rolls_left == 0) {
        printf("\n*** NO ROLLS LEFT! You must choose a scoring category. ***\n");
        return;
    }
    
    printf("\n+================== DICE SELECTION ==================+\n");
    printf("|  Select dice to KEEP for your next roll:            |\n");
    printf("|                                                      |\n");
    printf("|  Examples:                                           |\n");
    printf("|   '13'    - Keep dice 1 and 3                       |\n");
    printf("|   '245'   - Keep dice 2, 4, and 5                   |\n");
    printf("|   'all'   - Keep all dice (end turn)                |\n");
    printf("|   'none'  - Keep no dice (reroll all)               |\n");
    printf("|   'c'     - Cancel and continue with current keeps  |\n");
    printf("|                                                      |\n");
    printf("+======================================================+\n");
    printf("\nYour choice: ");
    
    char input[20];
    fgets(input, sizeof(input), stdin);
    
    // Remove newline
    for (int i = 0; input[i]; i++) {
        if (input[i] == '\n') input[i] = '\0';
    }
    
    // Handle special commands
    if (strcmp(input, "all") == 0 || strcmp(input, "ALL") == 0) {
        for (int i = 0; i < NUM_DICE; i++) {
            game.dice.keep[i] = true;
        }
        printf(">> All dice marked for keeping!\n");
        return;
    }
    
    if (strcmp(input, "none") == 0 || strcmp(input, "NONE") == 0 || strcmp(input, "0") == 0) {
        for (int i = 0; i < NUM_DICE; i++) {
            game.dice.keep[i] = false;
        }
        printf(">> All dice will be rerolled!\n");
        return;
    }
    
    if (strcmp(input, "c") == 0 || strcmp(input, "C") == 0 || strcmp(input, "cancel") == 0) {
        printf(">> Selection cancelled. Current keeps unchanged.\n");
        return;
    }
    
    // Reset all keep flags first
    for (int i = 0; i < NUM_DICE; i++) {
        game.dice.keep[i] = false;
    }
    
    // Process numeric input
    int kept_count = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] >= '1' && input[i] <= '5') {
            int dice_num = input[i] - '1';
            if (!game.dice.keep[dice_num]) {
                game.dice.keep[dice_num] = true;
                kept_count++;
            }
        }
    }
    
    if (kept_count == 0) {
        printf(">> No valid dice selected. All dice will be rerolled.\n");
    } else {
        printf(">> %d dice marked for keeping!\n", kept_count);
        printf("   Kept dice: ");
        for (int i = 0; i < NUM_DICE; i++) {
            if (game.dice.keep[i]) {
                printf("[%d] ", game.dice.values[i]);
            }
        }
        printf("\n");
    }
}

// Count occurrences of a specific dice value
int count_dice(int target) {
    int count = 0;
    for (int i = 0; i < NUM_DICE; i++) {
        if (game.dice.values[i] == target) {
            count++;
        }
    }
    return count;
}

// Check for straight of specified length
bool has_straight(int length) {
    bool present[7] = {false}; // Index 1-6 for dice values
    
    // Mark which values are present
    for (int i = 0; i < NUM_DICE; i++) {
        present[game.dice.values[i]] = true;
    }
    
    // Check for consecutive sequence
    int consecutive = 0;
    for (int i = 1; i <= 6; i++) {
        if (present[i]) {
            consecutive++;
            if (consecutive >= length) {
                return true;
            }
        } else {
            consecutive = 0;
        }
    }
    
    return false;
}

// Check for N of a kind
bool has_of_kind(int count) {
    for (int value = 1; value <= 6; value++) {
        if (count_dice(value) >= count) {
            return true;
        }
    }
    return false;
}

// Sum all dice values
int sum_all_dice(void) {
    int sum = 0;
    for (int i = 0; i < NUM_DICE; i++) {
        sum += game.dice.values[i];
    }
    return sum;
}

// Intelligent dice analysis and suggestions
void yahtzee_analyze_dice_and_suggest(void) {
    printf("\n+================ DICE ANALYSIS ================+\n");
    
    // Count each value
    int counts[7] = {0};
    for (int i = 0; i < NUM_DICE; i++) {
        counts[game.dice.values[i]]++;
    }
    
    // Analyze what we have
    bool has_pair = false, has_three = false, has_four = false;
    int pair_value = 0, three_value = 0, four_value = 0;
    
    for (int i = 1; i <= 6; i++) {
        if (counts[i] == 2) { has_pair = true; pair_value = i; }
        if (counts[i] == 3) { has_three = true; three_value = i; }
        if (counts[i] >= 4) { has_four = true; four_value = i; }
    }
    
    printf("|  Current dice combination analysis:            |\n");
    printf("|                                                |\n");
    
    if (has_four) {
        printf("|  >>> FOUR OF A KIND! Keep all %ds! <<<        |\n", four_value);
        printf("|  This is an excellent result!                 |\n");
    } else if (has_three && has_pair) {
        printf("|  >>> FULL HOUSE! Keep all dice! <<<           |\n");
        printf("|  This scores 25 points guaranteed!            |\n");
    } else if (has_three) {
        printf("|  >> Three %ds - consider keeping them        |\n", three_value);
        printf("|  Good chance for four-of-a-kind or full house |\n");
    } else if (has_straight(4)) {
        printf("|  >> Four in a row - you have a straight!      |\n");
        printf("|  Keep the straight dice for guaranteed points |\n");
    } else if (has_pair) {
        printf("|  > Pair of %ds found                          |\n", pair_value);
        printf("|  Consider keeping for potential full house    |\n");
    } else {
        printf("|  No obvious patterns - aim for high values    |\n");
        printf("|  or keep dice that could form straights       |\n");
    }
    
    // Suggest optimal keeping strategy
    printf("|                                                |\n");
    printf("|  SMART KEEPING SUGGESTION:                     |\n");
    
    if (has_four) {
        printf("|  Keep all %ds (positions: ", four_value);
        for (int i = 0; i < NUM_DICE; i++) {
            if (game.dice.values[i] == four_value) printf("%d ", i+1);
        }
        printf(")              |\n");
    } else if (has_three && has_pair) {
        printf("|  Keep ALL dice - you have a full house!       |\n");
    } else if (has_three) {
        printf("|  Keep the three %ds (positions: ", three_value);
        for (int i = 0; i < NUM_DICE; i++) {
            if (game.dice.values[i] == three_value) printf("%d ", i+1);
        }
        printf(")            |\n");
    } else {
        // Find highest values to suggest
        printf("|  Consider keeping highest values: ");
        int high_count = 0;
        for (int val = 6; val >= 1 && high_count < 3; val--) {
            for (int i = 0; i < NUM_DICE; i++) {
                if (game.dice.values[i] == val && high_count < 3) {
                    printf("%d ", i+1);
                    high_count++;
                }
            }
        }
        printf("         |\n");
    }
    
    printf("+================================================+\n");
}

// Calculate score for a category
int yahtzee_calculate_score(YahtzeeCategory category) {
    switch (category) {
        case ONES: case TWOS: case THREES: case FOURS: case FIVES: case SIXES:
            return count_dice(category + 1) * (category + 1);
            
        case THREE_OF_KIND:
            return has_of_kind(3) ? sum_all_dice() : 0;
            
        case FOUR_OF_KIND:
            return has_of_kind(4) ? sum_all_dice() : 0;
            
        case FULL_HOUSE: {
            // Check for 3 of one kind and 2 of another
            int counts[7] = {0};
            for (int i = 0; i < NUM_DICE; i++) {
                counts[game.dice.values[i]]++;
            }
            
            bool has_three = false, has_two = false;
            for (int i = 1; i <= 6; i++) {
                if (counts[i] == 3) has_three = true;
                if (counts[i] == 2) has_two = true;
            }
            
            return (has_three && has_two) ? 25 : 0;
        }
        
        case SM_STRAIGHT:
            return has_straight(4) ? 30 : 0;
            
        case LG_STRAIGHT:
            return has_straight(5) ? 40 : 0;
            
        case YAHTZEE_CAT: {
            // All five dice the same
            int first_value = game.dice.values[0];
            for (int i = 1; i < NUM_DICE; i++) {
                if (game.dice.values[i] != first_value) {
                    return 0;
                }
            }
            return 50;
        }
        
        case CHANCE:
            return sum_all_dice();
    }
    
    return 0;
}

// Check if category is available for scoring
bool yahtzee_is_valid_category(YahtzeeCategory category) {
    return !game.scorecard.used[category];
}

// Apply score to category
void yahtzee_apply_score(YahtzeeCategory category, int score) {
    game.scorecard.scores[category] = score;
    game.scorecard.used[category] = true;
    
    // Check for Yahtzee bonus
    if (category == YAHTZEE_CAT && score == 50) {
        // First Yahtzee
    } else if (score > 0 && yahtzee_calculate_score(YAHTZEE_CAT) == 50 && game.scorecard.used[YAHTZEE_CAT] && game.scorecard.scores[YAHTZEE_CAT] > 0) {
        // Subsequent Yahtzees
        game.scorecard.yahtzee_bonuses++;
        printf(">> YAHTZEE BONUS! +%d points!\n", YAHTZEE_BONUS);
    }
}

// Calculate totals and bonuses
void yahtzee_calculate_totals(void) {
    // Upper section total
    game.scorecard.upper_total = 0;
    for (int i = ONES; i <= SIXES; i++) {
        if (game.scorecard.used[i]) {
            game.scorecard.upper_total += game.scorecard.scores[i];
        }
    }
    
    // Upper section bonus
    game.scorecard.upper_bonus = (game.scorecard.upper_total >= UPPER_BONUS_THRESHOLD) ? UPPER_BONUS_POINTS : 0;
    
    // Lower section total
    game.scorecard.lower_total = 0;
    for (int i = THREE_OF_KIND; i <= CHANCE; i++) {
        if (game.scorecard.used[i]) {
            game.scorecard.lower_total += game.scorecard.scores[i];
        }
    }
    
    // Grand total
    game.scorecard.grand_total = game.scorecard.upper_total + game.scorecard.upper_bonus + 
                                 game.scorecard.lower_total + (game.scorecard.yahtzee_bonuses * YAHTZEE_BONUS);
}

// Enhanced turn scoring with smart suggestions
void yahtzee_score_turn(void) {
    printf("\n+==================== SCORING MENU ====================+\n");
    printf("|  Choose how to score your dice:                       |\n");
    printf("|                                                        |\n");
    printf("|  Commands:                                             |\n");
    printf("|   1-13  - Select scoring category                      |\n");
    printf("|   'p'   - Preview all potential scores                |\n");
    printf("|   'b'   - Show best scoring recommendations            |\n");
    printf("|   'h'   - Show help and strategy                      |\n");
    printf("|                                                        |\n");
    printf("+========================================================+\n");
    printf("\nYour choice: ");
    
    char input[10];
    fgets(input, sizeof(input), stdin);
    
    // Handle special commands
    if (input[0] == 'p' || input[0] == 'P') {
        printf("\n+=== POTENTIAL SCORES PREVIEW ===========================+\n");
        int best_score = 0;
        int best_category = -1;
        
        for (int i = 0; i < NUM_CATEGORIES; i++) {
            if (yahtzee_is_valid_category(i)) {
                int potential = yahtzee_calculate_score(i);
                char indicator = ' ';
                if (potential > best_score) {
                    best_score = potential;
                    best_category = i;
                    indicator = '*';
                }
                printf("| %c %2d. %-15s : %3d points %-10s |\n", 
                       indicator, i + 1, category_names[i], potential, 
                       (potential == 0) ? "(miss)" : "");
            }
        }
        printf("+======================================================+\n");
        if (best_category >= 0) {
            printf(">> BEST OPTION: %s (%d points) - marked with *\n", 
                   category_names[best_category], best_score);
        }
        printf("\nPress Enter to continue...");
        getchar();
        yahtzee_score_turn(); // Ask again
        return;
    }
    
    if (input[0] == 'b' || input[0] == 'B') {
        // Show intelligent recommendations
        printf("\n+=== SMART RECOMMENDATIONS ===========================+\n");
        int recommendations[3][2]; // [rank][category, score]
        int rec_count = 0;
        
        // Find top 3 scoring options
        for (int rank = 0; rank < 3; rank++) {
            int best_score = -1;
            int best_cat = -1;
            
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                if (yahtzee_is_valid_category(i)) {
                    int score = yahtzee_calculate_score(i);
                    bool already_recommended = false;
                    
                    for (int j = 0; j < rec_count; j++) {
                        if (recommendations[j][0] == i) {
                            already_recommended = true;
                            break;
                        }
                    }
                    
                    if (!already_recommended && score > best_score) {
                        best_score = score;
                        best_cat = i;
                    }
                }
            }
            
            if (best_cat >= 0) {
                recommendations[rec_count][0] = best_cat;
                recommendations[rec_count][1] = best_score;
                rec_count++;
            }
        }
        
        for (int i = 0; i < rec_count; i++) {
            printf("| %d. %-15s : %3d points - %s\n", 
                   i + 1, 
                   category_names[recommendations[i][0]], 
                   recommendations[i][1],
                   (i == 0) ? "BEST CHOICE" : 
                   (i == 1) ? "Good backup" : "Alternative");
        }
        printf("+===================================================+\n");
        printf("\nPress Enter to continue...");
        getchar();
        yahtzee_score_turn(); // Ask again
        return;
    }
    
    if (input[0] == 'h' || input[0] == 'H') {
        yahtzee_show_strategy();
        yahtzee_score_turn(); // Ask again
        return;
    }
    
    int choice = atoi(input);
    
    if (choice < 1 || choice > NUM_CATEGORIES) {
        printf("\n*** Invalid choice! Please select 1-13 or use commands. ***\n");
        SLEEP_MS(1500);
        yahtzee_score_turn();
        return;
    }
    
    YahtzeeCategory category = choice - 1;
    
    if (!yahtzee_is_valid_category(category)) {
        printf("\n*** Category '%s' already used! Choose another. ***\n", 
               category_names[category]);
        SLEEP_MS(1500);
        yahtzee_score_turn();
        return;
    }
    
    int score = yahtzee_calculate_score(category);
    yahtzee_apply_score(category, score);
    
    printf("\n+======== SCORING COMPLETE ========+\n");
    printf("| Category: %-15s      |\n", category_names[category]);
    printf("| Points:   %3d                 |\n", score);
    if (score == 0) {
        printf("| Result:   MISS                |\n");
    } else if (score >= 25) {
        printf("| Result:   EXCELLENT!          |\n");
    } else {
        printf("| Result:   SCORED              |\n");
    }
    printf("+===================================+\n");
}

// Show game rules
void yahtzee_show_rules(void) {
    CLEAR_SCREEN();
    printf("+==============================================================================+\n");
    printf("|                              ** YAHTZEE RULES **                            |\n");
    printf("+==============================================================================+\n");
    printf("\n>> OBJECTIVE:\n");
    printf("   Score the highest total by rolling five dice to make certain combinations.\n");
    printf("\n>> GAMEPLAY:\n");
    printf("   * Each turn: Roll up to 3 times, keeping dice between rolls\n");
    printf("   * After rolling, choose a scoring category (must use each category once)\n");
    printf("   * Game lasts 13 rounds (one for each category)\n");
    printf("\n>> SCORING CATEGORIES:\n");
    printf("   UPPER SECTION (sum of matching dice):\n");
    printf("   * Ones, Twos, Threes, Fours, Fives, Sixes\n");
    printf("   * Bonus: +35 points if upper section total >= 63\n");
    printf("\n   LOWER SECTION:\n");
    printf("   * 3 of a Kind: 3+ same dice (sum all dice)\n");
    printf("   * 4 of a Kind: 4+ same dice (sum all dice)\n");
    printf("   * Full House: 3 of one + 2 of another (25 points)\n");
    printf("   * Small Straight: 4 consecutive dice (30 points)\n");
    printf("   * Large Straight: 5 consecutive dice (40 points)\n");
    printf("   * YAHTZEE: All 5 dice same (50 points + bonuses)\n");
    printf("   * Chance: Any combination (sum all dice)\n");
    printf("\n>> YAHTZEE BONUSES:\n");
    printf("   * Additional Yahtzees after the first: +100 points each\n");
    printf("\nPress any key to return to game...");
    GETCH();
}

// Show strategy hints
void yahtzee_show_strategy(void) {
    CLEAR_SCREEN();
    printf("+==============================================================================+\n");
    printf("|                            ** STRATEGY HINTS **                             |\n");
    printf("+==============================================================================+\n");
    printf("\n>> GENERAL STRATEGY:\n");
    printf("   * Go for upper section bonus early (average 3+ of each number)\n");
    printf("   * Keep pairs when going for 3/4 of a kind or full house\n");
    printf("   * Save Chance category for high-scoring hands that don't fit elsewhere\n");
    printf("   * Large Straight is rare - take Small Straight when you can\n");
    printf("\n>> DICE KEEPING TIPS:\n");
    printf("   * Keep any 3+ of a kind immediately\n");
    printf("   * Keep pairs when you need them for full house\n");
    printf("   * For straights, keep any consecutive sequence\n");
    printf("   * When in doubt, keep the highest dice for Chance\n");
    printf("\n>> PROBABILITY TIPS:\n");
    printf("   * Yahtzee odds: ~0.08%% per roll (very rare!)\n");
    printf("   * Large Straight: ~3.1%% (1-2-3-4-5 or 2-3-4-5-6)\n");
    printf("   * Small Straight: ~12.3%% (any 4 consecutive)\n");
    printf("   * Full House: ~3.9%% per roll\n");
    printf("\n>> SCORING PRIORITIES:\n");
    printf("   1. Take Yahtzee if you get it (50 + potential bonuses)\n");
    printf("   2. Secure upper section bonus (focus on 4s, 5s, 6s)\n");
    printf("   3. Take 4 of a kind with high values\n");
    printf("   4. Use lower categories strategically\n");
    printf("\nPress any key to return to game...");
    GETCH();
}

// Display final results
void yahtzee_final_results(void) {
    CLEAR_SCREEN();
    yahtzee_calculate_totals();
    
    printf("+==============================================================================+\n");
    printf("|                            ** FINAL RESULTS **                              |\n");
    printf("+==============================================================================+\n");
    
    yahtzee_display_scorecard();
    
    printf("\n>> GAME SUMMARY:\n");
    printf("   * Upper Section: %d points", game.scorecard.upper_total);
    if (game.scorecard.upper_bonus > 0) {
        printf(" + %d bonus = %d", game.scorecard.upper_bonus, game.scorecard.upper_total + game.scorecard.upper_bonus);
    }
    printf("\n   * Lower Section: %d points", game.scorecard.lower_total);
    if (game.scorecard.yahtzee_bonuses > 0) {
        printf("\n   * Yahtzee Bonuses: %d x %d = %d points", 
               game.scorecard.yahtzee_bonuses, YAHTZEE_BONUS, game.scorecard.yahtzee_bonuses * YAHTZEE_BONUS);
    }
    printf("\n\n>> FINAL SCORE: %d POINTS\n", game.scorecard.grand_total);
    
    // Score evaluation
    if (game.scorecard.grand_total >= 400) {
        printf("** EXCELLENT! You're a Yahtzee master!\n");
    } else if (game.scorecard.grand_total >= 300) {
        printf("** GREAT JOB! Very solid gameplay!\n");
    } else if (game.scorecard.grand_total >= 200) {
        printf("** GOOD SCORE! Keep practicing!\n");
    } else {
        printf("** Nice try! Yahtzee takes practice to master.\n");
    }
    
    printf("\nPress any key to return to main menu...");
    GETCH();
}

// Main Yahtzee game function
void yahtzee_game(void) {
    srand(time(NULL));
    
    CLEAR_SCREEN();
    printf("+==============================================================================+\n");
    printf("|                       ** Welcome to YAHTZEE! **                             |\n");
    printf("+==============================================================================+\n");
    printf("\n>> Ready to play the classic dice game?\n");
    printf("\n[R] Play Game  [H] Rules  [S] Strategy  [Q] Quit\n");
    printf("Choose: ");
    
    char choice = GETCH();
    choice = tolower(choice);
    
    switch (choice) {
        case 'h':
            yahtzee_show_rules();
            yahtzee_game();
            return;
        case 's':
            yahtzee_show_strategy();
            yahtzee_game();
            return;
        case 'q':
            return;
        case 'r':
            break;
        default:
            yahtzee_game();
            return;
    }
    
    // Initialize and start game
    yahtzee_init_game();
    
    // Main game loop
    while (!game.game_over) {
        CLEAR_SCREEN();
        yahtzee_display_header();
        yahtzee_display_scorecard();
        yahtzee_display_dice();
        
        if (game.rolls_left > 0) {
            // Show intelligent analysis
            yahtzee_analyze_dice_and_suggest();
            
            printf("\n+============= TURN ACTIONS =============+\n");
            printf("|                                        |\n");
            printf("|  [R] Roll dice     [K] Keep/select     |\n");
            printf("|  [P] Preview scores [S] Strategy       |\n");
            printf("|  [H] Help & Rules   [Q] Quit game      |\n");
            printf("|                                        |\n");
            printf("+========================================+\n");
            printf("\nWhat would you like to do? ");
            
            char action = GETCH();
            action = tolower(action);
            
            printf("\n"); // Add line after input
            
            switch (action) {
                case 'r':
                    yahtzee_roll_dice();
                    if (game.rolls_left > 0) {
                        printf("\nPress any key to continue...");
                        GETCH();
                    }
                    break;
                case 'k':
                    yahtzee_select_dice();
                    printf("\nPress any key to continue...");
                    GETCH();
                    break;
                case 'p':
                    // Quick preview without full menu
                    printf("\n=== QUICK SCORE PREVIEW ===\n");
                    for (int i = 0; i < NUM_CATEGORIES && i < 6; i++) {
                        if (yahtzee_is_valid_category(i)) {
                            printf("%d. %-12s: %3d pts\n", i+1, category_names[i], yahtzee_calculate_score(i));
                        }
                    }
                    printf("... (use scoring menu for full list)\n");
                    printf("\nPress any key to continue...");
                    GETCH();
                    break;
                case 's':
                    yahtzee_show_strategy();
                    break;
                case 'h':
                    yahtzee_show_rules();
                    break;
                case 'q':
                    printf("\n>>> Thanks for playing Yahtzee! <<<\n");
                    printf("Your final score would have been: %d points\n", game.scorecard.grand_total);
                    return;
                default:
                    printf("Invalid choice '%c'! Try again.\n", action);
                    SLEEP_MS(1000);
                    break;
            }
        } else {
            // Must score this turn
            printf("\n*** SCORING REQUIRED ***\n");
            printf("No rolls left - you must choose a scoring category!\n");
            yahtzee_score_turn();
            yahtzee_calculate_totals();
            
            // Reset for next round
            game.current_round++;
            game.rolls_left = MAX_ROLLS;
            for (int i = 0; i < NUM_DICE; i++) {
                game.dice.keep[i] = false;
            }
            
            // Check if game is over
            if (game.current_round > NUM_ROUNDS) {
                game.game_over = true;
            } else {
                printf("\n+========================================+\n");
                printf("| Round %2d complete! Moving to round %2d  |\n", 
                       game.current_round - 1, game.current_round);
                printf("| Current total: %4d points            |\n", 
                       game.scorecard.grand_total);
                printf("+========================================+\n");
                printf("\nPress any key for next round...");
                GETCH();
            }
        }
    }
    
    // Show final results
    yahtzee_final_results();
}