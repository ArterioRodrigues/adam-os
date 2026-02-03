#ifndef STRING_H
#define STRING_H

struct string {
  const char *data;
  int length;
};

bool cmpString(struct string s1, struct string s2);
int getStringLength(struct string str);
struct string createString(const char *c);

#endif
