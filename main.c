#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sched.h>

int time;


struct task{
	char name[8];
	int ready_time;
	int exe_time;
	int id;
	pid_t pid;
};

void time_unit()
{
	volatile unsigned long i;
	for(i=0;i<1000000UL;i++);  
}

int cmp(const void *p1, const void *p2)
{
	struct task *a = (struct task *)p1;
	struct task *b = (struct task *)p2;
	if (a->ready_time < b->ready_time)
        return -1;
	else if (a->ready_time == b->ready_time && a->id < b->id)
	return -1;
    else
        return 1;
}

void assign_cpu(pid_t pid , int core)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core , &mask);
	sched_setaffinity(pid , sizeof(mask) , &mask);
}

void higher_priority(pid_t pid)
{
	struct sched_param param;
	param.sched_priority = 0;
	sched_setscheduler(pid, SCHED_OTHER, &param);
}

void lower_priority(pid_t pid)
{
	struct sched_param param;
    param.sched_priority = 0;
    sched_setscheduler(pid, SCHED_IDLE, &param);
}


pid_t proc_exe(struct task task)
{
	
	pid_t pid = fork();

	if(pid == 0)
	{	
		assign_cpu(pid , 1);
		long long start , end , n_start , n_end;
		syscall(333 , &start , &n_start);
		for(int i = 0 ; i < task.exe_time ; i++)
			time_unit();
		syscall(333 , &end , &n_end);
		syscall(334 , getpid() , start , n_start , end , n_end); 
		exit(0);
	}
	
	
	return pid;
}




void schedule(struct task *task , int policy , int task_num)
{
	assign_cpu(getpid() , 0);
	higher_priority(getpid());
	int finish = 0 , running = -1 , RR_time = 0;
	int *RR_queue = (int *)malloc(sizeof(int) * (task_num));
	int queue_len = 0;
	int PSJF_new = 0;
	for(int i = 0 ; i < task_num ; i++)
	{
		RR_queue[i] = -1;
	}

	while(finish < task_num)
	{
		for(int i = 0 ; i < task_num ; i++)
		{
			if(time == task[i].ready_time)
			{
				task[i].pid = proc_exe(task[i]);
				lower_priority(task[i].pid);
				RR_queue[queue_len] = i;
				queue_len++;
				PSJF_new = 1;
			}
		}
		int next = -1;
		int exe_t_min = 100000007;
		
		switch(policy)
		{
			
			case 0:
				if(running == -1)
				{
					for(int i = 0 ; i < task_num ; i++)
					{
						if(task[i].pid != -1 && task[i].exe_time != 0)
						{
							next = i;
							break;
						}
					}
				}
				else
					next = running;
				break;
				
			case 1:
				if(PSJF_new == 1 || running == -1){
				for(int i = 0 ; i < task_num ; i++)
				{
					if(task[i].pid != -1 && task[i].exe_time != 0 && task[i].exe_time < exe_t_min)
					{
						next = i;
						exe_t_min = task[i].exe_time;
					}
				}
				PSJF_new = 0;
				}
				break;
				
			case 2:
				if(running == -1 && queue_len > 0)   
				{
					next = RR_queue[0];
					queue_len--;
					for(int i = 0 ; i < queue_len ; i++)
					{
						RR_queue[i] = RR_queue[i+1];
					}

				}
				else if(running != -1)
				{
					//a process end running for a time quantum
					if((time - RR_time) % 500 == 0)  
					{
						next = RR_queue[0];
						for(int i = 0 ; i < queue_len - 1 ; i++)
						{
							RR_queue[i] = RR_queue[i+1];
						}

						//wait in the end of queue
						RR_queue[queue_len-1] = running;  
					}
				}
				break;
				
			case 3:
				if(running == -1)
				{
					for(int i = 0 ; i < task_num ; i++)
					{
						if(task[i].pid != -1 && task[i].exe_time != 0 && task[i].exe_time < exe_t_min)
						{
							next = i;
							exe_t_min = task[i].exe_time;
						}
					}
				}
				else
					next = running;
				break;
		}
		//contextswitch
		if(next != -1 && next != running)  
		{
			if(running != -1) lower_priority(task[running].pid);
			higher_priority(task[next].pid);
			running = next;
			RR_time = time;
		}
		
		time_unit();
		time++;
		if(running != -1)
		{
			task[running].exe_time--;
		}
		
		//a process finish
		if(running != -1 && task[running].exe_time == 0)
		{
			printf("%s %d\n", task[running].name , task[running].pid); 
            waitpid(task[running].pid , NULL , 0);
            running = -1;
            finish++;
		}
	}
}

int main(int argc, char *argv[])
{
	char policy[8];
	scanf("%s",policy);
	int task_num , policy_n;
	scanf("%d",&task_num);
	struct task *task = (struct task *) malloc(sizeof(struct task) * task_num);
	for(int i = 0 ; i < task_num ; i++)
	{
		scanf("%s%d%d" , task[i].name , &task[i].ready_time , &task[i].exe_time);
		task[i].pid = -1;
		task[i].id = i;
	}
	qsort(task , task_num , sizeof(struct task) , cmp);
	if(strcmp(policy , "FIFO") == 0)
		policy_n = 0;
	else if(strcmp(policy , "PSJF") == 0)
		policy_n = 1;
	else if(strcmp(policy , "RR") == 0)
		policy_n = 2;
	else if(strcmp(policy , "SJF") == 0)
		policy_n = 3;
	schedule(task , policy_n , task_num);
}
