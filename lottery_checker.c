
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

const unsigned int MAX_PLAYERS = 10000000;
#define MAX_LINE_LEN 64

// Store each player's 5 numbers as a 90-bit bitmap
unsigned int *players;
int player_count = 0;

// Converts 5 numbers into a 90-bit bitmask (as 3 unsigned ints)
void line_to_bitmap(const int *nums, unsigned int *bitmap) {
    bitmap[0] = bitmap[1] = bitmap[2] = 0;
    for (int i = 0; i < 5; ++i) {
        int n = nums[i] - 1;
        bitmap[n / 32] |= (1U << (n % 32));
    }
}

// Count bits in a 32-bit int
// Time complexity = O(1)
int popcount(unsigned int x) {
    int count = 0;
    while (x) {
        x &= (x - 1);
        count++;
    }
    return count;
}

// Count total matching bits between two 90-bit bitmaps as number of matches
// Key optimization: replace nested for-loop compare by bit operations to improve performance.
// Time complexity = O(1)
int count_matches(unsigned int *a, unsigned int *b) {
    return popcount(a[0] & b[0]) + popcount(a[1] & b[1]) + popcount(a[2] & b[2]);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.txt\n", argv[0]);
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    players = malloc(sizeof(unsigned int) * 3 * MAX_PLAYERS);
    if (!players) {
        perror("malloc");
        return -1;
    }

    char line[MAX_LINE_LEN];
    int nums[5];

    // Read input file to players array
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d %d %d %d %d", &nums[0], &nums[1], &nums[2], &nums[3], &nums[4]) != 5)
            continue;
        line_to_bitmap(nums, &players[player_count * 3]);
        player_count++;
    }
    fclose(f);

    // Ready to accept queries
    printf("READY\n");
    fflush(stdout);

    // Read from stdin (lottery numbers)
    while (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%d %d %d %d %d", &nums[0], &nums[1], &nums[2], &nums[3], &nums[4]) != 5)
            continue;

        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        unsigned int lottery[3];
        line_to_bitmap(nums, lottery);

        int win2 = 0, win3 = 0, win4 = 0, win5 = 0;
        for (int i = 0; i < player_count; ++i) {
            int matches = count_matches(&players[i * 3], lottery);
            if (matches == 2) win2++;
            else if (matches == 3) win3++;
            else if (matches == 4) win4++;
            else if (matches == 5) win5++;
        }

        gettimeofday(&end, NULL);
        long seconds = end.tv_sec - start.tv_sec;
        long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
        printf("%d %d %d %d\n", win2, win3, win4, win5);
        fprintf(stderr, "Processed in %ld microseconds (%.2f ms)\n", micros, micros / 1000.0);
        //Tested that it generally takes less than 65ms to get winner results.
        //For further improve the calculation speed, we can consider split the for loop into groups 
        // and leverage parallel power as multiple thread and/or GPU power(GPGPU/CUDA) to reduce calculation time.
        fflush(stdout);
    }

    free(players);
    return 0;
}
