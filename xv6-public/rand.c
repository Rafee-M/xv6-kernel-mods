// raf: Linear Congruential Generator (LCG)
// xv6 has no libc, so there is no rand()/srand() available in the kernel.
#include "rand.h"

static unsigned long next_rand = 1;

void
srand_k(unsigned long seed) // raf: can use timer here
{
  next_rand = seed;
}

int
rand_k(void)
{
  next_rand = next_rand * 1103515245 + 12345;
  return (unsigned int)(next_rand / 65536) % 32768;
}