#include "../pch.h"

bool strcmp(const char *str1, const char *str2) {
  int index = 0;
  while (str1[index] != '\0' && str2[index] != '\0') {
    if (str1[index] != str2[index])
      return false;
    index++;
  }

  if (str1[index] != '\0' || str2[index] != '\0')
    return false;

  return true;
}

bool strncmp(const char *str1, const char *str2, int n) {
  int index = 0;
  while (n > 0 && str1[index] != '\0' && str2[index] != '\0') {
    if (str1[index] != str2[index])
      return false;
    index++;
    n--;
  }

  if (n != 0)
    return false;
  return true;
}

int strlen(const char *str) {
  int index = 0;
  while (str[index] != '\0')
    index++;

  return index;
}

char *strcpy(char *destination, char *source) {
  int index = 0;

  while (source[index] != '\0') {
    destination[index] = source[index];
    index++;
  }

  destination[index] = '\0';
  return destination;
}

char *strcat(char *destination, char *source) {
  int index = 0;
  int destLength = strlen(destination);

  while (source[index] != '\0') {
    destination[destLength + index] = source[index];
    index++;
  }

  destination[index] = '\0';

  return destination;
}
