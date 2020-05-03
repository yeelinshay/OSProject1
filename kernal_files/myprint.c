#include<linux/linkage.h>
#include<linux/kernel.h>

asmlinkage void sys_myprint(int pid , long long start , long long start_n , long long end , long long end_n)
{
	printk(KERN_INFO "[Project1] %d %lld.%09lld %lld.%09lld\n", pid , start , start_n , end , end_n);
}
