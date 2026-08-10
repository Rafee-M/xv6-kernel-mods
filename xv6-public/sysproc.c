#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "rand.h" // raf: RNG

// raf: ticket exchange and transfer functions
int
sys_settickets(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  if(n < 1)
    return -1;
  myproc()->tickets = n;
  return 0;
}

int
sys_transfertickets(void)
{
  int target_pid, n;
  struct proc *p;
  struct proc *curproc = myproc();
  int found = 0;

  if(argint(0, &target_pid) < 0 || argint(1, &n) < 0)
    return -1;
  if(n <= 0)
    return -1;
  if(target_pid == curproc->pid)
    return -1;                        // reject self-transfer

  acquire(&ptable.lock);

  // Caller must retain at least 1 ticket after transferring.
  if(curproc->tickets <= n){
    release(&ptable.lock);
    return -1;
  }
  // Only one outstanding loan tracked at a time — reject stacking.
  if(curproc->lent_amount > 0){
    release(&ptable.lock);
    return -1;
  }

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == target_pid && p->state != UNUSED && p->state != ZOMBIE){
      found = 1;
      break;
    }
  }
  if(!found){
    release(&ptable.lock);
    return -1;
  }

  curproc->tickets -= n;
  p->tickets += n;
  curproc->lent_pid = target_pid;   // record the loan for later auto-revert
  curproc->lent_amount = n;

  release(&ptable.lock);
  return 0;
}

int
sys_exchangetickets(void)
{
  int target_pid, tmp;
  struct proc *p;
  struct proc *curproc = myproc();

  if(argint(0, &target_pid) < 0)
    return -1;
  if(target_pid == curproc->pid)
    return -1;                        // reject self-exchange

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == target_pid && p->state != UNUSED && p->state != ZOMBIE){
      tmp = curproc->tickets;
      curproc->tickets = p->tickets;
      p->tickets = tmp;
      release(&ptable.lock);
      return 0;                       // exchange does NOT create a loan —
                                        // it's a permanent swap, not reverted
    }
  }

  release(&ptable.lock);
  return -1;
}

// raf: Expose the kernel's RNG to user space
int
sys_getrandom(void)
{
  return rand_k();
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
