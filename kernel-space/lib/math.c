#include "../pch.h"

int pow(int n, int power) {
  int result = 1;

  if (power == 0)
    return result;

  for (int i = 0; i < power; i++) {
    result *= n;
  }

  return result;
}
