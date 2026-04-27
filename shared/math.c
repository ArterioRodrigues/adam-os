#include "math.h"
double pow(double n, double power) {
    double result = 1;

    if (power == 0)
        return result;

    for (double i = 0; i < power; i++) {
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
    if (x < y)
        return x;
    return y;
}
int ceil(int x, int y) { return (x + y - 1) / y; }

int abs(int x) {
    if (x < 0)
        return x * -1;

    return x;
}

double factorial(double n) {
    double result = 1;
    for (int i = 1; i <= n; i++)
        result *= i;

    return result;
}

float sin_approx(float x) {
    while (x >  PI) x -= 2.0f * PI;
    while (x < -PI) x += 2.0f * PI;

    float result = x;
    for (int i = 1; i < 10; i++) {
        int power = 2 * i + 1;
        if (i % 2 == 0)
            result += pow(x, power) / factorial(power);
        else
            result -= pow(x, power) / factorial(power);
    }
    return result;
}

float cos_approx(float x) {
    while (x >  PI) x -= 2.0f * PI;
    while (x < -PI) x += 2.0f * PI;

    float result = 1.0f;
    for (int i = 1; i < 7; i++) {
        int power = 2 * i;
        if (i % 2 == 0)
            result += pow(x, power) / factorial(power);
        else
            result -= pow(x, power) / factorial(power);
    }
    return result;
}
