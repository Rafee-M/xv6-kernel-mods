#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(void)
{
  struct pstat ps;
  int p[2];
  pipe(p);
  int lpid = fork();

  if(lpid == 0){
    // ---- L: low-ticket process doing a "critical section" ----
    settickets(2);
    volatile long i;
    for(i = 0; i < 1500000000; i++)
      ;
    write(p[1], "x", 1);      // signal completion
    exit();
  } else {
    // ---- H: high-ticket process waiting on L ----
    settickets(90);
    getpinfo(&ps);
    printf(1, "Before transfer: H has 90, L has 2\n");

    transfertickets(lpid, 80);   // lend 80 tickets to L before blocking

    getpinfo(&ps);
    // find and print L's boosted balance here via the pid/tickets arrays

    char buf[1];
    int start = uptime();
    read(p[0], buf, 1);           // blocks (SLEEPING) — this is the wakeup1 path
    int elapsed = uptime() - start;
    printf(1, "L finished in %d ticks (boosted)\n", elapsed);

    getpinfo(&ps);
    // confirm H is back to 90 and L is back to its own baseline —
    // this is the auto-revert firing inside wakeup1()

    wait();
    exit();
  }
}
