#include "../lib/helpers.h" 

void wait() {
    int i = 0;
    int j = 0;
    while (i < 10000) {
        while (j < 10000)
            j++;
        i++;
        j = 0;
    }
}
