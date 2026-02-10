#ifndef STRING_H
#define STRING_H

bool strcmp(const char *str1, const char *str2);
bool strncmp(const char *str1, const char *str2, int n);
int strlen(const char *str);
char *strcpy(char *dest, char *source);
char *strcat(char *dest, char *source);
char *strrev(char *str);
char *itos(char *result, int n);
char *itohs(char *result, int n);
int stoi(char *str);
char *strtok(char *destination, char *str, char delim);
char *strncpy(char* destination, const char* source, int n);
char *strncat(char* dest, const char* src, int n);

#endif
