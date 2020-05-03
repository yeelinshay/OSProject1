#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/timer.h>

asmlinkage void sys_mytime(long long *sec , long long *nsec)
{
	struct timespec t;
	getnstimeofday(&t);
	*sec = t.tv_sec;
	*nsec = t.tv_nsec;
}
