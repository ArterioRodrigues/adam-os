#ifndef STRING_H
#define STRING_H
#include "lib.h"

char char_n_10[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char char_n_16[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static int strlen(const char *s) {
    int i = 0;
    while (s[i])
        i++;
    return i;
}

static int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static int strncmp(const char *a, const char *b, int n) {
    int i = 0;
    while (i < n && a[i] && b[i] && a[i] == b[i])
        i++;
    return i == n;
}

static void strcpy(char *dst, const char *src) {
    int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void strcat(char *dst, const char *src) {
    int i = strlen(dst);
    int j = 0;
    while (src[j]) {
        dst[i++] = src[j++];
    }
    dst[i] = '\0';
}

char *strrev(char *str) {
    int str_length = strlen(str);
    for (int i = 0; i < str_length / 2; i++) {
        char temp = str[i];
        str[i] = str[str_length - i - 1];
        str[str_length - i - 1] = temp;
    }

    return str;
}

char *itos(char *result, int n) {
    int index = 0;

    while (n / 10 > 0) {
        result[index] = char_n_10[n % 10];
        index++;
        n = n / 10;
    }
    result[index] = char_n_10[n];
    result[index + 1] = '\0';
    
    strrev(result);
    return result;
}

char *itohs(char *result, int n) {
    int index = 0;
    char hex[20];
    while (n / 16 > 0) {
        hex[index] = char_n_16[n % 16];
        index++;
        n = n / 16;
    }
    hex[index] = char_n_16[n];
    hex[index + 1] = '\0';

    strrev(hex);
    result[0] = '0';
    result[1] = 'X';

    int i;
    for (i = 0; hex[i] != '\0'; i++) {
        result[2 + i] = hex[i];
    }
    result[2 + i] = '\0';
    return result;
}

static void print(const char *s) { sys_write(0, (char *)s, strlen(s)); }


int stoi(char *str) {
    int result = 0;
    int i = 0;

    int sign = 1;
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9')
            break;
        result = result * 10 + (str[i] - '0');
    }

    return result * sign;
}
#endif

