/* 
 * pthread_create wrapper for gprof compatibility
 *
 * needed headers: <pthread.h>
 *                 <sys/time.h>
 */

#include <pthread.h>
#include <sys/time.h>


#ifndef __TSP_MTGPROG
#define __TSP_MTGPROG 1

typedef struct wrapper_s
{
    void * (*start_routine)(void *);
    void * arg;

    pthread_mutex_t lock;
    pthread_cond_t  wait;

    struct itimerval itimer;

} wrapper_t;

static void * wrapper_routine(void *);

int gprof_pthread_create(pthread_t * thread, pthread_attr_t * attr,
                         void * (*start_routine)(void *), void * arg);



#endif /* __TSP_MTGPROG */