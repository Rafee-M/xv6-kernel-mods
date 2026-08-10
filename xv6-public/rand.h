// raf: simple pseudo-random number generator for kernel use
void srand_k(unsigned int seed);
int  rand_k(void);
int  random_range(int max);    //// returns value in [0, max) i.e totalTickets