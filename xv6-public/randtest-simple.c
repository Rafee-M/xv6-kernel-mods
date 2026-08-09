#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  int i;

  printf(1, "Testing kernel RNG (rand_k) via getrandom() syscall:\n");
  for(i = 0; i < 10; i++){
    int r = getrandom();
    printf(1, "call %d: %d\n", i, r);
  }

  exit();
}