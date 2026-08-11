#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

void print_tickets(struct pstat *ps, int pid, char *label){
  int i;
  for(i = 0; i < NPROC; i++)
    if(ps->inuse[i] && ps->pid[i] == pid)
      printf(1, "%s (pid %d): %d tickets\n", label, pid, ps->tickets[i]);
}

int
main(void)
{
  struct pstat ps;
  int mypid = getpid();
  int pid = fork();

  if(pid == 0){
    settickets(5);
    sleep(2);
    exit();
  } else {
    settickets(50);
    sleep(1);
    getpinfo(&ps);
    print_tickets(&ps, mypid, "BEFORE parent");
    print_tickets(&ps, pid,   "BEFORE child ");

    exchangetickets(pid);

    getpinfo(&ps);
    print_tickets(&ps, mypid, "AFTER  parent");
    print_tickets(&ps, pid,   "AFTER  child ");

    wait();
    exit();
  }
}
