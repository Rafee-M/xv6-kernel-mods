// raf: Linear Congruential Generator (LCG)
// xv6 has no libc, so there is no rand()/srand() available in the kernel.

#include "types.h"
#include "rand.h"

static unsigned int next = 1;

int
rand_k(void)
{
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void
srand_k(unsigned int seed)
{
  next = seed;
}

// Returns a pseudo-random value in [0, max). Not perfectly uniform if
// max doesn't evenly divide 32768 (modulo bias), but the skew is
// negligible for lottery-scheduling purposes.
int
random_range(int max)
{
  if (max <= 0)
    return 0;
  return rand_k() % max;
}