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
#include "queue.h"

#define max(A, B) (A >= B) ? A : B;

// Algorithm's parameter initialization
double avg = 0; // Average queue length
int count = -1; // Count of packets since last probabilistic drop
double wq = 0.002; // Queue weight; standard value of 0.002 for early congestion detection
int minThreshold, maxThreshold;
double maxp = 0.02; // Maximum probability of dropping a packet; standard value of 0.02
double pb = 0; // Probability of dropping a packet
time_t qTime; // Time since the queue was last idle
Queue *queue; // Queue to store the packets

// Handle CTRL+Z(stop signal) keyboard signal
void sig_handler(int signo)
{
  if (signo == SIGTSTP) {
      queue->size = 0;
      queue->front = 0;
      queue->rear = -1;
      qTime = time(NULL);
  }
}

void error(const char* msg) {
    perror(msg);
    exit(1);
}


void red(Queue *queue, char *buffer) {
    printf("Current packet : %c\n", buffer[0]);
    // Average queue length calculation
    if (queue->size == 0) {
        double m = (time(NULL) - qTime) / 0.001;
        avg = pow((1 - wq), m) * avg;
    } else {
        avg = ((1 - wq)*avg) + (wq*queue->size);
    }
    printf("Average queue length : %f\n", avg);
    // If the average length is in between minimum and maximum threshold,
    // Probabilistically drop a packet
    if(minThreshold <= avg && avg < maxThreshold) {
        count++;
        pb = avg - minThreshold;
        pb = pb * maxp;
        pb = pb / (maxThreshold - minThreshold);
        double pa = pb / (1 - (count*pb));
        if (count == 50) {
            printf("Count has reached 1/maxp; dropping the next packet\n");
            pa = 1.0;
        }
        float randomProb = (rand()%100)/100.0;
        if(randomProb < pa) {
            // Drop the packet with probability pa
            printf("Dropping packet : %c with probability : %f\n", buffer[0], pa);
            // Since this packet was dropped, count is reinitialized to 0
            count = 0;
        } else {
            // Add the packet to the queue
            add(queue, buffer[0]);
        }
    } else if (maxThreshold <= avg) {
        // Drop the packet
        printf("Packet Dropped : %c\n", buffer[0]);
        // Since this packet was dropped, count is reinitialized to 0
        count = 0;
    } else {
        // Average queue length is below minimum threhold, accept all packets
        // Add packet to the queue
        add(queue, buffer[0]);
        // Since the average queue length is below minimum threshold, initialize count to -1
        count = -1;
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