#include <stdlib.h>
#include <time.h>

#include "darkness.h"


/*
    Initialize the random number generator.
*/
void rng_init(long seed) {
    srand(seed);
}

/*
    Initialize the random number generator with the current timestamp.
*/
void rng_init_time() {
    srand(time(NULL));
}

/*
    Returns a number in the range [0..max).
*/
int rng_max(int max) {
    return rand() % max;
}

/*
    Returns a number in the range [min..max).
*/
int rng_range(int min, int max) {
    int range = max - min;
    int result = min + rand() % range;
    return result;
}

/*
    Rolls a number of dice with the given number of sides.
*/
int rng_roll(int count, int sides) {
    int result = 0;
    for (int i = 0; i < count; ++i) {
        result += rng_range(0, sides);
    }
    return result;
}
