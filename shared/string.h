#ifndef STRING_H
#define STRING_H

#include "types.h"
extern char char_n_10[10]; 
extern char char_n_16[16];
extern void sys_write(int fd, char *buf, int len);

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
int strfind(char *c, char deliminator);
char *ftos(char *result, float n, int precision);
#endif
