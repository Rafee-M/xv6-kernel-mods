// raf: categorizes outputs into 10 blocks and increments them to show distribution over 1000 entries
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  int buckets[10];
  int i, r, b;

  for(i = 0; i < 10; i++)
    buckets[i] = 0;

  for(i = 0; i < 1000; i++){
    r = getrandom();
    b = r / 3277;         // 32768 / 10 ≈ 3277, splits range into 10 buckets
    if(b > 9) b = 9;       // guard the top edge
    buckets[b]++;
  }

  for(i = 0; i < 10; i++)
    printf(1, "bucket %d: %d\n", i, buckets[i]);

  exit();
}