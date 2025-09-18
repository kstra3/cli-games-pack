#include "games.h"

#define CHAMBER_COUNT 6
#define MAX_PLAYERS 6

typedef struct {
    int chambers[CHAMBER_COUNT];  // 0 = empty, 1 = bullet
    int current_chamber;
    int bullet_count;
} Revolver;

typedef struct {
    char name[32];
    int is_alive;
    int shots_survived;
} Player;

void display_revolver_ascii(void) {
    printf("\n");
    printf("        ╔═══════════════════════════════════╗\n");
    printf("        ║        RUSSIAN ROULETTE           ║\n");
    printf("        ║                                   ║\n");
    printf("        ║           .-.   .-.               ║\n");
    printf("        ║          /   \\ /   \\              ║\n");
    printf("        ║         |  O  |  O  |             ║\n");
    printf("        ║          \\   / \\   /              ║\n");
    printf("        ║           '-'   '-'               ║\n");
    printf("        ║              |                    ║\n");
    printf("        ║         ═════════════             ║\n");
    printf("        ║        ║ ● ● ● ● ● ● ║            ║\n");
    printf("        ║         ═════════════             ║\n");
    printf("        ║              |||||                ║\n");
    printf("        ║          ▄▄▄▄▄▄▄▄▄▄▄             ║\n");
    printf("        ║         ████████████              ║\n");
    printf("        ╚═══════════════════════════════════╝\n");
    printf("\n");
}

void display_chamber_status(Revolver* revolver) {
    printf("\n┌─────────────────────────────────────┐\n");
    printf("│            CYLINDER STATUS          │\n");
    printf("│                                     │\n");
    printf("│    Chamber:  1  2  3  4  5  6       │\n");
    printf("│    Status:  ");
    
    for (int i = 0; i < CHAMBER_COUNT; i++) {
        if (i == revolver->current_chamber) {
            printf(" ► ");
        } else {
            printf(" ? ");
        }
    }
    printf("      │\n");
    printf("│                                     │\n");
    printf("│    Bullets loaded: %d/6              │\n", revolver->bullet_count);
    printf("└─────────────────────────────────────┘\n");
}

void initialize_revolver(Revolver* revolver, int bullet_count) {
    // Clear all chambers
    for (int i = 0; i < CHAMBER_COUNT; i++) {
        revolver->chambers[i] = 0;
    }
    
    revolver->bullet_count = bullet_count;
    revolver->current_chamber = 0;
    
    // Randomly place bullets in chambers
    for (int i = 0; i < bullet_count; i++) {
        int chamber;
        do {
            chamber = rand() % CHAMBER_COUNT;
        } while (revolver->chambers[chamber] == 1);
        
        revolver->chambers[chamber] = 1;
    }
}

void spin_cylinder(Revolver* revolver) {
    printf("\n🔄 *SPINNING CYLINDER* 🔄\n");
    printf("   Click... click... click...\n");
    
    // Simulate spinning animation
    for (int i = 0; i < 10; i++) {
        printf(".");
        fflush(stdout);
        // Small delay simulation
        for (volatile int j = 0; j < 50000000; j++);
    }
    
    revolver->current_chamber = rand() % CHAMBER_COUNT;
    printf("\n\n✨ Cylinder spun! Starting position set. ✨\n");
}

int pull_trigger(Revolver* revolver) {
    int result = revolver->chambers[revolver->current_chamber];
    
    printf("\n🔫 *CLICK*\n");
    
    if (result == 1) {
        printf("\n💥 BANG! 💥\n");
        printf("   ████████████████████████████\n");
        printf("   █ ☠️  FATAL SHOT!  ☠️ █\n");
        printf("   ████████████████████████████\n");
    } else {
        printf("\n😅 *Click* - Empty chamber!\n");
        printf("   You live to see another day...\n");
    }
    
    // Move to next chamber
    revolver->current_chamber = (revolver->current_chamber + 1) % CHAMBER_COUNT;
    
    return result;
}

void display_game_rules(void) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    RUSSIAN ROULETTE RULES                    ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║                                                           ║\n");
    printf("║  • A revolver with 6 chambers is loaded with bullets      ║\n");
    printf("║  • Players take turns pulling the trigger                 ║\n");
    printf("║  • The cylinder spins before the first shot               ║\n");
    printf("║  • If you get an empty chamber, you survive the round     ║\n");
    printf("║  • If you get a bullet... GAME OVER for that player!      ║\n");
    printf("║  • Last player standing wins!                             ║\n");
    printf("║                                                           ║\n");
    printf("║  ⚠️  WARNING: This is a simulation game for entertainment  ║\n");
    printf("║      only. Never attempt this in real life!              ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
}

void play_single_player_mode(void) {
    Revolver revolver;
    int bullets, round = 1;
    int shots_survived = 0;
    
    printf("\n🎯 SINGLE PLAYER SURVIVAL MODE 🎯\n");
    printf("How many bullets do you want to load? (1-5): ");
    
    if (scanf("%d", &bullets) != 1 || bullets < 1 || bullets > 5) {
        printf("Invalid input! Loading 1 bullet...\n");
        bullets = 1;
    }
    
    initialize_revolver(&revolver, bullets);
    spin_cylinder(&revolver);
    
    printf("\n🎲 Starting with %d bullet(s) loaded...\n", bullets);
    
    while (1) {
        printf("\n═══════════════════════════════════════\n");
        printf("              ROUND %d\n", round);
        printf("═══════════════════════════════════════\n");
        
        display_chamber_status(&revolver);
        
        printf("\nPress Enter to pull the trigger...");
        getchar();
        
        if (pull_trigger(&revolver)) {
            // Player died
            printf("\n💀 GAME OVER! 💀\n");
            printf("You survived %d shot(s) before meeting your fate.\n", shots_survived);
            break;
        } else {
            // Player survived
            shots_survived++;
            printf("\n🎉 You survived shot #%d!\n", shots_survived);
            
            // Check if all bullets are used up
            int remaining_bullets = 0;
            for (int i = 0; i < CHAMBER_COUNT; i++) {
                remaining_bullets += revolver.chambers[i];
            }
            
            if (remaining_bullets == 0) {
                printf("\n🏆 INCREDIBLE! You survived all bullets!\n");
                printf("🌟 ULTIMATE SURVIVOR - %d shots survived! 🌟\n", shots_survived);
                break;
            }
        }
        
        round++;
    }
}

void play_multiplayer_mode(void) {
    Revolver revolver;
    Player players[MAX_PLAYERS];
    int num_players, bullets;
    int current_player = 0;
    int alive_count;
    int round = 1;
    
    printf("\n👥 MULTIPLAYER MODE 👥\n");
    printf("Number of players (2-6): ");
    
    if (scanf("%d", &num_players) != 1 || num_players < 2 || num_players > MAX_PLAYERS) {
        printf("Invalid input! Setting to 2 players...\n");
        num_players = 2;
    }
    
    printf("How many bullets to load? (1-%d): ", num_players - 1);
    
    if (scanf("%d", &bullets) != 1 || bullets < 1 || bullets >= num_players) {
        printf("Invalid input! Loading 1 bullet...\n");
        bullets = 1;
    }
    
    // Initialize players
    for (int i = 0; i < num_players; i++) {
        printf("Enter name for Player %d: ", i + 1);
        scanf("%31s", players[i].name);
        players[i].is_alive = 1;
        players[i].shots_survived = 0;
    }
    
    initialize_revolver(&revolver, bullets);
    spin_cylinder(&revolver);
    
    printf("\n🎲 Game starting with %d bullet(s) loaded...\n", bullets);
    
    while (1) {
        // Count alive players
        alive_count = 0;
        for (int i = 0; i < num_players; i++) {
            if (players[i].is_alive) alive_count++;
        }
        
        if (alive_count <= 1) {
            // Game over - find winner
            for (int i = 0; i < num_players; i++) {
                if (players[i].is_alive) {
                    printf("\n🏆 WINNER: %s! 🏆\n", players[i].name);
                    printf("Survived %d shot(s) to claim victory!\n", players[i].shots_survived);
                    break;
                }
            }
            break;
        }
        
        // Find next alive player
        while (!players[current_player].is_alive) {
            current_player = (current_player + 1) % num_players;
        }
        
        printf("\n═══════════════════════════════════════\n");
        printf("              ROUND %d\n", round);
        printf("═══════════════════════════════════════\n");
        printf("🎯 %s's turn!\n", players[current_player].name);
        
        display_chamber_status(&revolver);
        
        printf("\n%s, press Enter to pull the trigger...", players[current_player].name);
        getchar();
        
        if (pull_trigger(&revolver)) {
            // Player died
            printf("\n💀 %s has been eliminated! 💀\n", players[current_player].name);
            players[current_player].is_alive = 0;
        } else {
            // Player survived
            players[current_player].shots_survived++;
            printf("\n😌 %s survived! (Total: %d shots)\n", 
                   players[current_player].name, players[current_player].shots_survived);
        }
        
        current_player = (current_player + 1) % num_players;
        round++;
    }
}

void play_russian_roulette(void) {
    int choice;
    
    display_revolver_ascii();
    display_game_rules();
    
    printf("\nSelect game mode:\n");
    printf("1. Single Player (Survival)\n");
    printf("2. Multiplayer (Last One Standing)\n");
    printf("3. Return to Main Menu\n");
    printf("\nChoice (1-3): ");
    
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input!\n");
        return;
    }
    
    // Clear input buffer
    while (getchar() != '\n');
    
    switch (choice) {
        case 1:
            play_single_player_mode();
            break;
        case 2:
            play_multiplayer_mode();
            break;
        case 3:
            return;
        default:
            printf("Invalid choice!\n");
            break;
    }
}