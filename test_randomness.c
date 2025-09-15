#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Copy the symbols and functions from slot_machine.c
typedef enum {
    CHERRY = 0,   // ^
    DIAMOND = 1,  // *
    BELL = 2,     // @
    BAR = 3,      // #
    SEVEN = 4,    // 7
    CROWN = 5,    // &
    COIN = 6,     // O
    STAR = 7      // +
} SlotSymbol;

const char* get_symbol_display(SlotSymbol symbol) {
    switch(symbol) {
        case CHERRY:  return "^";
        case DIAMOND: return "*";
        case BELL:    return "@";
        case BAR:     return "#";
        case SEVEN:   return "7";
        case CROWN:   return "&";
        case COIN:    return "O";
        case STAR:    return "+";
        default:      return "?";
    }
}

void generate_three_symbols(SlotSymbol* reel1, SlotSymbol* reel2, SlotSymbol* reel3) {
    *reel1 = (SlotSymbol)(rand() % 8);
    *reel2 = (SlotSymbol)(rand() % 8);
    *reel3 = (SlotSymbol)(rand() % 8);
}

int main() {
    srand(time(NULL));
    
    printf("Testing randomness - generating 20 sets of three symbols:\n\n");
    
    for (int i = 0; i < 20; i++) {
        SlotSymbol reel1, reel2, reel3;
        generate_three_symbols(&reel1, &reel2, &reel3);
        
        printf("Test %2d: %s %s %s", i+1, 
               get_symbol_display(reel1),
               get_symbol_display(reel2),
               get_symbol_display(reel3));
        
        // Check if all three are the same
        if (reel1 == reel2 && reel2 == reel3) {
            printf(" <- ALL SAME!");
        }
        printf("\n");
    }
    
    return 0;
}