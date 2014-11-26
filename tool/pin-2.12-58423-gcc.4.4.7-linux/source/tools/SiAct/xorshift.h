#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <stdint.h>
#include <stdio.h>

static uint32_t x32 = 314159265;
static uint64_t x64 = 88172645463325252ull;
static uint32_t x128 = 123456789;
static uint32_t y128 = 362436069;
static uint32_t z128 = 521288629;
static uint32_t w128 = 88675123;

void sxorshift32(uint32_t a){
		x32 = a == 0 ? x32 : a;
}

void sxorshift64(uint64_t a){
		x64 = a == 0 ? x64 : a;
}
void sxorshift128(uint32_t a,
				 uint32_t b,
				 uint32_t c,
				 uint32_t d){
		x128 = a == 0 ? x128 : a;
		y128 = b == 0 ? y128 : b;
		z128 = c == 0 ? z128 : c;
		w128 = d == 0 ? w128 : d;
}
uint32_t xorshift32() {
  x32 ^= x32 << 13;
  x32 ^= x32 >> 17;
  x32 ^= x32 << 5;
  return x32;
}

uint64_t xorshift64() {
  x64 ^= x64 << 13;
  x64 ^= x64 >> 7;
  x64 ^= x64 << 17;
  return x64;
}

uint32_t xorshift128() {
  uint32_t t = x128 ^ (x128 << 11);
  x128 = y128; y128 = z128; z128 = w128;
  w128 ^= (w128 >> 19) ^ t ^ (t >> 8);
 
  return w128;
}



#endif
