#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>


#include "pid_controller.h"

#define BUFFER_CAP 64
#define CLOCK_MONOTONIC 1

float update_sensor_setpoint(float *sp_ptr, pid_config_t *p1) {
    if (*sp_ptr != p1->setpoint) {
        p1->setpoint = *sp_ptr;
    }
    return p1->setpoint;
}

float pid_equation(pid_config_t *pid, float *dt1) {
    float error = pid->setpoint - pid->pv;
    pid->integral += error * (*dt1);
    float derivative = (error - pid->prev_error) / (*dt1);

    float output = pid->Kp * error +
                   pid->Ki * pid->integral +
                   pid->Kd * derivative;

    pid->prev_error = error;
    printf("Control output %.3f\n", output);
    return output;
}

int main_pid(void) {
    pid_config_t pid_controller = { .Kp=1, .Ki=1, .Kd=1, .prev_error=0 };

    int sensor_fd;
    char buffer[BUFFER_CAP];
    ssize_t bytes_read;
    float user_setpoint = 0.0f;
    float dt = 0.1f; // initialized guess

    /* Timer setup */
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd < 0) { perror("timerfd_create"); return 1; }

    struct itimerspec timer_spec = {
        .it_interval = {0, 100 * 1000000},  // 100 ms
        .it_value    = {0, 100 * 1000000}
    };

    if (timerfd_settime(tfd, 0, &timer_spec, NULL) < 0) {
        perror("timerfd_settime");
        return 1;
    }

    struct timespec last_time, now_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    uint64_t expirations;

    while (1) {
        if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations)) {
            perror("timerfd read");
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &now_time);
        dt = (now_time.tv_sec - last_time.tv_sec) +
             (now_time.tv_nsec - last_time.tv_nsec) / 1e9f;
        last_time = now_time;

        printf("Enter new setpoint: ");
        if (scanf("%f", &user_setpoint) == 1) {
            pid_controller.setpoint = update_sensor_setpoint(&user_setpoint, &pid_controller);
            printf("New setpoint = %.2f\n", user_setpoint);
        }

        sensor_fd = open("sensor.txt", O_RDONLY);
        if (sensor_fd < 0) { perror("open sensor"); continue; }

        bytes_read = read(sensor_fd, buffer, sizeof(buffer) - 1);
        close(sensor_fd);
        if (bytes_read <= 0) { perror("read sensor"); continue; }

        buffer[bytes_read] = '\0';
        pid_controller.pv = atof(buffer);
        printf("Process variable: %.2f\n", pid_controller.pv);

        pid_controller.output = pid_equation(&pid_controller, &dt);
    }

    close(tfd);
    return 0;
}
