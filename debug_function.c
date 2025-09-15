#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Test the generate_three_symbols function directly
void generate_three_symbols(int *reel1, int *reel2, int *reel3) {
    printf("Before generation: reel1=%d, reel2=%d, reel3=%d\n", *reel1, *reel2, *reel3);
    *reel1 = rand() % 8;
    *reel2 = rand() % 8;
    *reel3 = rand() % 8;
    printf("After generation: reel1=%d, reel2=%d, reel3=%d\n", *reel1, *reel2, *reel3);
}

int main() {
    srand(time(NULL));
    
    printf("Testing generate_three_symbols function:\n\n");
    
    for (int i = 0; i < 10; i++) {
        int reel1 = -1, reel2 = -1, reel3 = -1;
        printf("Test %d:\n", i+1);
        generate_three_symbols(&reel1, &reel2, &reel3);
        printf("Final values: %d %d %d", reel1, reel2, reel3);
        if (reel1 == reel2 && reel2 == reel3) {
            printf(" <- ALL SAME!");
        }
        printf("\n\n");
    }
    
    return 0;
}