#ifndef RED_H
#define RED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include "../queue.h"

#define max(A, B) (A >= B) ? A : B;

// Algorithm's parameter initialization
static double red_avg = 0; // Average red_queue length
static int red_count = -1; // Count of packets since last probabilistic drop
static double red_wq = 0.002; // Queue weight; standard value of 0.002 for early congestion detection
static int red_min_threshold, red_max_threshold;
static double red_maxp = 0.02; // Maximum probability of dropping a packet; standard value of 0.02
static double red_pb = 0; // Probability of dropping a packet
static time_t red_qtime; // Time since the red_queue was last idle
static Queue * red_queue; // Queue to store the packets

// Handle CTRL+Z(stop signal) keyboard signal
static void sig_handler(int signo)
{
  if (signo == SIGTSTP) {
      red_queue->size = 0;
      red_queue->front = 0;
      red_queue->rear = -1;
      red_qtime = time(NULL);
  }
}

static void error(const char* msg) {
    perror(msg);
    exit(1);
}


static void red(Queue *red_queue, char *buffer) {
    printf("Current packet : %c\n", buffer[0]);
    // Average red_queue length calculation
    if (red_queue->size == 0) {
        double m = (time(NULL) - red_qtime) / 0.001;
        red_avg = pow((1 - red_wq), m) * red_avg;
    } else {
        red_avg = ((1 - red_wq)*red_avg) + (red_wq*red_queue->size);
    }
    printf("Average red_queue length : %f\n", red_avg);
    // If the average length is in between minimum and maximum threshold,
    // Probabilistically drop a packet
    if(red_min_threshold <= red_avg && red_avg < red_max_threshold) {
        red_count++;
        red_pb = red_avg - red_min_threshold;
        red_pb = red_pb * red_maxp;
        red_pb = red_pb / (red_max_threshold - red_min_threshold);
        double pa = red_pb / (1 - (red_count*red_pb));
        if (red_count == 50) {
            printf("Count has reached 1/maxp; dropping the next packet\n");
            pa = 1.0;
        }
        float randomProb = (rand()%100)/100.0;
        if(randomProb < pa) {
            // Drop the packet with probability pa
            printf("Dropping packet : %c with probability : %f\n", buffer[0], pa);
            // Since this packet was dropped, red_count is reinitialized to 0
            red_count = 0;
        } else {
            // Add the packet to the red_queue
            add(red_queue, buffer[0]);
        }
    } else if (red_max_threshold <= red_avg) {
        // Drop the packet
        printf("Packet Dropped : %c\n", buffer[0]);
        // Since this packet was dropped, red_count is reinitialized to 0
        red_count = 0;
    } else {
        // Average red_queue length is below minimum threhold, accept all packets
        // Add packet to the red_queue
        add(red_queue, buffer[0]);
        // Since the average red_queue length is below minimum threshold, initialize red_count to -1
        red_count = -1;
    }
}

// Write PID of the current process to a file for the Queue processor python script to read
void writePID(int pid) {
    FILE* fp;
    fp = fopen("pid", "w");
    fprintf(fp, "%d", pid);
    fclose(fp);
}   

#endif