#include "../pch.h"

char char_n_10[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char char_n_16[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

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
  int dest_length = strlen(destination);
  char *result;

  while (source[index] != '\0') {
    destination[dest_length + index] = source[index];
    index++;
  }

  destination[dest_length + index] = '\0';
  return destination;
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

char *int_to_string(char *result, int n) {

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

char *int_to_hex_string(char *result, int n) {
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

  for (int i = 0; hex[i] != '\0'; i++) {
    result[2 + i] = hex[i];
  }
  return result;
}

int string_to_int(char *str) {
  int result = 0;
  int str_length = strlen(str);

  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] >= '0' && str[i] <= '9')
      result += (str[i] - '0') * pow(10, str_length - i - 1);
  }

  return result;
}
static char *string_token = NULL;

char *strtok(char *str, char delim) {
  if (str != NULL) {
    string_token = str;
  }

  if (string_token == NULL || *string_token == '\0') {
    return NULL;
  }

  while (*string_token == delim) {
    string_token++;
  }

  char *token_start = string_token;

  while (*string_token && *string_token != delim) {
    string_token++;
  }

  if (*string_token) {
    *string_token = '\0';
    string_token++;
  }

  return (*token_start != '\0') ? token_start : NULL;
}
