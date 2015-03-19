#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <stdint.h>
#include <stdio.h>



void sxorshift32(uint32_t a);
void sxorshift64(uint64_t a);
void sxorshift128(uint32_t a,
				 uint32_t b,
				 uint32_t c,
				 uint32_t d);
uint32_t xorshift32();
uint64_t xorshift64();
uint32_t xorshift128();



#endif
