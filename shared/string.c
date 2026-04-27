#include "string.h"
#include "abi-types.h"
#include "string.h"

static const char digits_10[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
static const char digits_16[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

bool strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

bool strncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0')
            return false;
    }
    return true;
}

int strlen(const char *str) {
    const char *s = str;
    while (*s)
        s++;
    return s - str;
}

int strfind(char *str, char delim) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == delim)
            return i;
    }
    return -1;
}

char *strcpy(char *dest, char *src) {
    char *d = dest;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, int n) {
    char *d = dest;
    while (n-- > 0 && *src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

char *strcat(char *dest, char *src) {
    char *d = dest + strlen(dest);
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

char *strncat(char *dest, const char *src, int n) {
    char *d = dest + strlen(dest);
    while (n-- > 0 && *src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

char *strrev(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
    return str;
}

char *itos(char *result, int n) {
    int i = 0;
    bool negative = false;

    if (n < 0) {
        negative = true;
        n = -n;
    }

    do {
        result[i++] = '0' + (n % 10);
        n /= 10;
    } while (n > 0);

    if (negative)
        result[i++] = '-';

    result[i] = '\0';
    strrev(result);
    return result;
}

char *itohs(char *result, int n) {
    char hex[20];
    int i = 0;

    do {
        hex[i++] = digits_16[n % 16];
        n /= 16;
    } while (n > 0);

    hex[i] = '\0';
    strrev(hex);

    result[0] = '0';
    result[1] = 'X';
    strcpy(result + 2, hex);
    return result;
}

int stoi(char *str) {
    int result = 0;
    int i = 0;
    int sign = 1;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; str[i] >= '0' && str[i] <= '9'; i++)
        result = result * 10 + (str[i] - '0');

    return result * sign;
}

static char *string_token = NULL;

char *strtok(char *dest, char *str, char delim) {
    if (str)
        string_token = str;

    if (!string_token || *string_token == '\0') {
        dest[0] = '\0';
        return NULL;
    }

    while (*string_token == delim)
        string_token++;

    int i = 0;
    while (*string_token && *string_token != delim)
        dest[i++] = *string_token++;

    dest[i] = '\0';

    if (*string_token)
        string_token++;

    return (i > 0) ? dest : NULL;
}

char *ftos(char *result, float n, int precision) {
    int i = 0;
    bool negative = false;

    if (n < 0) {
        negative = true;
        n = -n;
    }

    int scale = 1;
    for (int p = 0; p < precision; p++)
        scale *= 10;

    int total = (int)(n * (float)scale + 0.5f);
    int int_part = total / scale;
    int frac_part = total % scale;

    do {
        result[i++] = '0' + (int_part % 10);
        int_part /= 10;
    } while (int_part > 0);

    if (negative)
        result[i++] = '-';

    result[i] = '\0';
    strrev(result);

    if (precision > 0) {
        result[i++] = '.';
        int divisor = scale / 10;
        for (int p = 0; p < precision; p++) {
            result[i++] = '0' + ((frac_part / divisor) % 10);
            divisor /= 10;
        }
        result[i] = '\0';
    }

    return result;
}
