#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#define MAX_SAMPLES 256

#include <time.h>

typedef struct {
    float pv;
    float setpoint;
    float Kp;
    float Ki;
    float Kd;
    float prev_error;
    float integral;
    float output;
} pid_config_t;


typedef struct {
    int top;
    float process_variable[MAX_SAMPLES];
    float setpoint[MAX_SAMPLES];
    float output[MAX_SAMPLES];
    float elapsed_time[MAX_SAMPLES];
} pid_buffer_t;

float update_sensor_setpoint(float *sp_ptr, pid_config_t *p1);
float pid_equation(pid_config_t *pid, float *dt1, pid_buffer_t *pid_buf);
int main_pid(pid_buffer_t *pid_buf);


#endif
