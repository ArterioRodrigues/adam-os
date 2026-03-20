#include "math.h"
int pow(int n, int power) {
    int result = 1;

    if (power == 0)
        return result;

    for (int i = 0; i < power; i++) {
        result *= n;
    }

    return result;
}

int max(int x, int y) {
    if (x > y)
        return x;
    return y;
}

int min(int x, int y) {
    if(x < y) 
      return x;
    return y;
}
int ceil(int x, int y) { return (x + y - 1) / y; }

float abs(int x) {
    if (x < 0)
        return x * -1;

    return x;
}
