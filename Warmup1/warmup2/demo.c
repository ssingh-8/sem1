#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

pthread_t tid[2];
int ret1,ret2;

void* doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
        printf("\n First thread processing\n");
	ret1  = 100;
        pthread_exit(&ret1);
    }
    else
    {
        printf("\n Second thread processing\n");
	ret2  = 200;
        pthread_exit(&ret2);
    }

    for(i=0; i<(0xFFFFFFFF);i++);
	
	sigset_t *set = arg;
           int s, sig;

           for (;;) {
               s = sigwait(set, &sig);
               if (s != 0)
                   printf("\ncan't create mask :[%s]", strerror(s));
               printf("Signal handling thread got signal %d\n", sig);
           }

    return NULL;
}

int main(void)
{
    int i = 0;
    int err;
    int *ptr[2];
    sigset_t set;

    while(i < 2)
    {
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	err = pthread_sigmask(SIG_BLOCK,&set,NULL);
	if(err!=0)
		printf("\ncan't create mask :[%s]", strerror(err));

        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");

        i++;
    }

    pthread_join(tid[0], (void**)&(ptr[0]));
    pthread_join(tid[1], (void**)&(ptr[1]));

    printf("\n return value from first thread is [%d]\n", *ptr[0]);
    printf("\n return value from second thread is [%d]\n", *ptr[1]);
    
    pause();
    //sleep(1);
    return 0;
}
