#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <errno.h>     // for strerror()

#include "pid_controller.h"
#include "graphing_pid.h"

/*
Task 1
-Task 1 is running the main pid logic
-Has priority 0 (same as task 2)
-pid_buffer_t passed in as an argument, will be used for graphing in task 2
*/
void* rt_func(void* data) {
    pid_buffer_t *pid_buf = (pid_buffer_t*) data;
    printf("[PID] Started control loop\n");
    main_pid(pid_buf);
    return NULL;
}

/*
Task 2
-Task 1 is running the real-time plotting logic
-Has priority 0 (same as task 1)
-pid_buffer_t passed in as an argument, used in graphing for task 1
*/
void* plot_thread_func(void* arg) {
    pid_buffer_t *pid_buf = (pid_buffer_t*) arg;
    printf("[PLOT] Started graphing thread\n");
    plot_task(pid_buf);
    return NULL;
}


//Create two tasks and wait for them of them to finish
//As of now, the only way to finish execution is Ctrl+C in the terminal
int main(void) {
    //Initialize stack pointer, the two threads (inside of the one process)
    struct sched_param sp;
    pthread_attr_t attr;
    pthread_t pid_thread;
    pthread_t plot_thread;

    pid_buffer_t pid_buffer = { .top = 0 };

    // Lock memory to prevent paging
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // Basic pthread setup, use normal scheduling (0 priority), non-FIFO
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 0;
    pthread_attr_setschedparam(&attr, &sp);
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

    // Create the two tasks
    pthread_create(&pid_thread, &attr, rt_func, &pid_buffer); //Task created with a thread
    pthread_create(&plot_thread, &attr, plot_thread_func, &pid_buffer);
    printf("[MAIN] PID and Plot tasks created\n");

    // Wait for both to finish
    pthread_join(pid_thread, NULL);
    pthread_join(plot_thread, NULL);

    printf("[MAIN] Program exiting\n");
    return 0;
}

