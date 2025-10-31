#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

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

float update_sensor_setpoint(float *sp_ptr, pid_config_t *p1);
float pid_equation(pid_config_t *pid, float *dt1);
int main_pid(void);

#endif
