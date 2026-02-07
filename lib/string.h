#ifndef STRING_H
#define STRING_H

bool strcmp(const char *str1, const char *str2);
bool strncmp(const char *str1, const char *str2, int n);
int strlen(const char *str);
char *strcpy(char *dest, char *source);
char *strcat(char *dest, char *source);

char *strrev(char *str);

char *int_to_string(char *result, int n);
char *int_to_hex_string(char *result, int n);

int string_to_int(char *str);

#endif
