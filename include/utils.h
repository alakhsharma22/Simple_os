#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int atoi(char *str);
void putint(int n);
int strcmp(const char *s1, const char *s2);
void puthex(uint64_t val);
void gets(char *buf, int max);
void power_off(void);

#endif
