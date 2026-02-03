#include "../pch.h"

bool cmpString(struct string s1, struct string s2) {
  if (s1.length != s2.length) {
    return false;
  }

  for (int i = 0; i < s1.length; i++) {
    if (s1.data[i] != s2.data[i])
      return false;
  }

  return true;
}

int getStringLength(struct string str) {

  int index = 0;
  while (str.data[index] != '\0') {
    index++;
  }

  return index;
}

struct string createString(const char *c) {
  struct string str;
  str.data = c;
  str.length = getStringLength(str);
  return str;
}
