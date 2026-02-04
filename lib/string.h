#ifndef STRING_H
#define STRING_H

bool strcmp(const char *str1, const char *str2);
bool strncmp(const char *str1, const char *str2, int n);
int strlen(const char *str);
char *strcpy(char *dest, char *source);
char *strcat(char *dest, char *source);

#endif
