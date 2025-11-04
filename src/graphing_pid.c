#include "gnuplot_i.h"
#include "pid_controller.h"
#include <unistd.h>
#include <pthread.h>

#define MAX_SAMPLES 200

/*
Plotting:

-Take the values from the PV, setpoint, and output buffers and plot them in real-time using the gnuplot library
-Each time a new setpoint is set, the three values will be plotted on a values vs. time axis
-Use the gnuplot command function to plot
-Plot will be saved to a pid_plot.png file
*/
void plot_task(pid_buffer_t *pid_buf)
{
    gnuplot_ctrl *gp = gnuplot_init(); 
    gnuplot_setstyle(gp, "lines");
    gnuplot_cmd(gp, "set grid");
    gnuplot_cmd(gp, "set title 'PID Controller Response'");
    gnuplot_set_axislabel(gp, "x", "Time (s)");
    gnuplot_set_axislabel(gp, "y", "Value");

    double local_time[MAX_SAMPLES];
    double local_pv[MAX_SAMPLES];
    double local_sp[MAX_SAMPLES];
    double local_out[MAX_SAMPLES];

    while (1)
    {
        int count = pid_buf->top;
        for (int i = 0; i < count; i++) {
            local_time[i] = pid_buf->elapsed_time[i];
            local_pv[i]   = pid_buf->process_variable[i];
            local_sp[i]   = pid_buf->setpoint[i];
            local_out[i]  = pid_buf->output[i];
        }

        if (count <= 0) {
            usleep(500000);
            continue;   // skip until PID thread fills the buffer
        }


        gnuplot_cmd(gp, "reset");
        gnuplot_cmd(gp, "set title 'PID Controller Response'");
        gnuplot_cmd(gp, "set xlabel 'Time (s)'");
        gnuplot_cmd(gp, "set ylabel 'Value'");
        gnuplot_cmd(gp, "set grid");
        gnuplot_cmd(gp, "set terminal pngcairo size 800,400");
        gnuplot_cmd(gp, "set output 'pid_plot.png'");

        gnuplot_cmd(gp,
            "plot '-' with lines title 'Setpoint', "
            "'-' with lines title 'Process Variable', "
            "'-' with lines title 'Output'");

        for (int i = 0; i < count; i++)
            fprintf(gp->gnucmd, "%f %f\n", local_time[i], local_sp[i]);
        fprintf(gp->gnucmd, "e\n");

        for (int i = 0; i < count; i++)
            fprintf(gp->gnucmd, "%f %f\n", local_time[i], local_pv[i]);
        fprintf(gp->gnucmd, "e\n");

        for (int i = 0; i < count; i++)
            fprintf(gp->gnucmd, "%f %f\n", local_time[i], local_out[i]);
        fprintf(gp->gnucmd, "e\n");

        fflush(gp->gnucmd);
    }

    gnuplot_close(gp); 
}
