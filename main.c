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

void* rt_func(void* data) {
    printf("[RT FUNC] Entering task\n");
    fflush(stdout);
    main_pid();
    printf("[RT FUNC] Exiting task\n");
    fflush(stdout);
    return NULL;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);  // disable buffering
    fprintf(stderr, ">>> stdout buffering disabled\n");
    fflush(stderr);

    struct sched_param sp;
    pthread_attr_t attr;
    pthread_t thread;
    int ret;

    printf("[1] Main started executing\n");
    fflush(stdout);

    // -------- Step 1: Lock memory (optional, safe failure)
    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    printf("[2] mlockall() returned %d\n", ret);
    if (ret < 0) {
        printf("[WARN] mlockall failed (not fatal): %s (errno=%d)\n", strerror(errno), errno);
    }
    fflush(stdout);

    // -------- Step 2: Init thread attributes
    ret = pthread_attr_init(&attr);
    printf("[3] pthread_attr_init() returned %d\n", ret);

    // -------- Step 3: Use normal scheduling instead of SCHED_FIFO
    // (Avoids needing CAP_SYS_NICE privilege)
    ret = pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    printf("[4] pthread_attr_setschedpolicy(SCHED_OTHER) returned %d\n", ret);
    if (ret != 0) {
        printf("[WARN] pthread_attr_setschedpolicy failed: %s (ret=%d)\n", strerror(ret), ret);
    }

    // -------- Step 4: Keep default (non-RT) priority = 0
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 0;
    ret = pthread_attr_setschedparam(&attr, &sp);
    printf("[5] pthread_attr_setschedparam(priority=0) returned %d\n", ret);
    if (ret != 0) {
        printf("[WARN] pthread_attr_setschedparam failed: %s (ret=%d)\n", strerror(ret), ret);
    }

    // -------- Step 5: Let thread inherit normal scheduling
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    printf("[6] pthread_attr_setinheritsched(PTHREAD_INHERIT_SCHED) returned %d\n", ret);
    if (ret != 0) {
        printf("[WARN] pthread_attr_setinheritsched failed: %s (ret=%d)\n", strerror(ret), ret);
    }

    // -------- Step 6: Create thread
    printf("[7] Creating worker thread...\n");
    ret = pthread_create(&thread, &attr, rt_func, NULL);
    printf("[8] pthread_create() returned %d\n", ret);
    if (ret != 0) {
        printf("[ERR] pthread_create failed: %s (ret=%d)\n", strerror(ret), ret);
    }

    // -------- Step 7: Wait for thread
    printf("[9] Joining thread...\n");
    ret = pthread_join(thread, NULL);
    printf("[10] pthread_join() returned %d\n", ret);
    if (ret != 0) {
        printf("[ERR] pthread_join failed: %s (ret=%d)\n", strerror(ret), ret);
    }

    printf("[11] Program exiting normally\n");
    return 0;
}
