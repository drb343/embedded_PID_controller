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

volatile float dt = 0.1f;

// Update setpoint only if changed
float update_sensor_setpoint(float *sp_ptr, pid_config_t *p1) {
    if (*sp_ptr != p1->setpoint) {
        p1->setpoint = *sp_ptr;
    }
    return p1->setpoint;
}

// PID core logic — fills shared buffer
float pid_equation(pid_config_t *pid, float *dt1, pid_buffer_t *pid1) {
    static float total_time = 0.0f;
    float error = pid->setpoint - pid->pv;
    pid->integral += error * (*dt1);
    float derivative = (error - pid->prev_error) / (*dt1);

    pid->output = pid->Kp * error +
                  pid->Ki * pid->integral +
                  pid->Kd * derivative;

    pid->prev_error = error;

    // Push values to shared buffer for plotting
    pid1->process_variable[pid1->top] = pid->pv;
    pid1->setpoint[pid1->top]         = pid->setpoint;
    pid1->output[pid1->top]           = pid->output;

    total_time += *dt1;
    pid1->elapsed_time[pid1->top] = total_time;

    pid1->top++;
    if (pid1->top >= MAX_SAMPLES)
        pid1->top = 0;

    return pid->output;
}

// Main PID control loop
int main_pid(pid_buffer_t *pid_buffer) {
    pid_config_t pid_controller = { .Kp=1, .Ki=1, .Kd=1, .prev_error=0 };
    int sensor_fd;
    char buffer[BUFFER_CAP];
    float user_setpoint = 0.0f;

    printf("\n=== PID Controller Initialized ===\n");
    printf("Gains: Kp=%.1f, Ki=%.1f, Kd=%.1f\n", 
           pid_controller.Kp, pid_controller.Ki, pid_controller.Kd);
    printf("----------------------------------\n");

    // Timer setup for 100ms loop
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd < 0) { perror("timerfd_create"); return 1; }

    struct itimerspec timer_spec = {
        .it_interval = {0, 100 * 1000000},
        .it_value    = {0, 100 * 1000000}
    };

    if (timerfd_settime(tfd, 0, &timer_spec, NULL) < 0) {
        perror("timerfd_settime");
        return 1;
    }

    struct timespec last_time, now_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    uint64_t expirations;

    // Main loop
    while (1) {
        if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations)) {
            perror("timerfd read");
            continue;
        }

        // Compute Δt for dynamic sampling time
        clock_gettime(CLOCK_MONOTONIC, &now_time);
        dt = (now_time.tv_sec - last_time.tv_sec) +
             (now_time.tv_nsec - last_time.tv_nsec) / 1e9f;
        last_time = now_time;

        printf("\n[Loop] Δt = %.3f s\n", dt);
        printf("Enter new setpoint: ");
        if (scanf("%f", &user_setpoint) == 1) {
            pid_controller.setpoint = update_sensor_setpoint(&user_setpoint, &pid_controller);
            printf("Setpoint updated to %.2f\n", user_setpoint);
        }

        FILE *sensor_file = fopen("sensor.txt", "r");
        if (!sensor_file) {
            perror("fopen sensor");
            continue;
        }

        while (fgets(buffer, sizeof(buffer), sensor_file) != NULL) {
            pid_controller.pv = atof(buffer);
            pid_controller.output = pid_equation(&pid_controller, &dt, pid_buffer);
        }
        fclose(sensor_file);
    }

    close(tfd);
    return 0;
}
