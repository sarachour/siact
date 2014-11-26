#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <stdint.h>

uint32_t xorshift32() {
  static uint32_t x = 314159265;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

uint64_t xorshift64() {
  static uint64_t x = 88172645463325252ull;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return x;
}

uint32_t xorshift128() {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
 
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  w ^= (w >> 19) ^ t ^ (t >> 8);
 
  return w;
}



#endif
