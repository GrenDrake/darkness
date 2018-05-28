#include "console.h"
#include "darkness.h"

int main() {
    con_init();
    rng_init_time();
    town_loop();
    con_end();
    return 0;
}
