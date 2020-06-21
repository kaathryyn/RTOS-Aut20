/*************************RTOS_AUT2020_ASSIGNMENT_3_PROGRAM_1***************************
 
  Name: Kathryn Corpuz
	Student Number: 13215785

  This program calculates the average wait time and turn-around time for a specific
	set of processes, by applying the Shortest-Remaining-Time-First algorithm. The
	arrival time and burst time of these processes are inputted from an input.txt file. 
	Once the average wait time and turn-around time have been calculated they will be
	written onto a new file, called output.txt.

  To compile Program_1 ensure that gcc is installed and run the following command:
  gcc program_1.c -o prog_1 -lpthread -lrt

	To run Program_1, perform the following on the command line 
	./prog_1 output.txt

****************************************************************************************/

#include <pthread.h> /* pthread functions and data structures for pipe */
#include <unistd.h>	 /* for POSIX API */
#include <stdlib.h>	 /* for exit() function */
#include <stdio.h>	 /* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

typedef struct SRTF_Params
{
	int pid;
	int arrive_t, wait_t, burst_t, turnaround_t, remain_t;
} SRTF_Params;

/* Semaphore */
sem_t semFIFO;

/* Max number of processes */
#define PROCESSNUM 7

/* Array of processes with 1 extra for placeholder remain_t */
SRTF_Params processes[8];

/* Index variable */
int i;

/* Setting average wait time and turnaround time */
float avg_wait_t = 0.0, avg_turnaround_t = 0.0;

/* Create process arrive times & burst times, taken from assignment */
void input_processes();

/* Schedule processes according to SRTF rule */
void process_SRTF();

/* Calcualte average wait time & turn-around time */
void calculate_average();

/* Print the processes onto the command line */
void printProcesses();

/* Calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1();

/* Reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2();

/* This main function creates named pipe and threads */
int main(int argc)
{
	pthread_t thread1, thread2;
	int result;

	char *myFIFO = "/tmp/myFIFO2";

	/* Creating a named pipe (FIFO) with read/write permission */
	result = mkfifo(myFIFO, 0777);
	if (result < 0)
	{
		perror("FIFO error");
		exit(1);
	}

	/* Initialise semaphores */
	if (sem_init(&semFIFO, 0, 0) != 0)
	{
		printf("Semaphore initialisation error\n");
		exit(1);
	}

	/* Create threads */
	if (pthread_create(&thread1, NULL, (void *)worker1, NULL) != 0)
	{
		perror("Error creating threads: ");
		exit(1);
	}

	if (pthread_create(&thread2, NULL, (void *)worker2, NULL) != 0)
	{
		perror("Error creating threads: ");
		exit(1);
	}

	if (argc != 2)
	{
		fprintf(stderr, "Error! Please do: ./prog_1 output.txt");
	}

	/* Wait for the thread to exit */
	if (pthread_join(thread1, NULL) != 0)
	{
		printf("Join Thread 1 error\n");
		exit(1);
	}

	if (pthread_join(thread2, NULL) != 0)
	{
		printf("Join Thread 2 error\n");
		exit(1);
	}

	if(sem_destroy(&semFIFO) != 0)
	{
		printf("Semaphore destroy error\n");
		exit(1);
	}

	printf("Average wait time and turn-around time have written to output.txt\n");

			return 0;
}

/* Calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(void *params)
{
	// Create wait and turnaround times and other variables
	int result1, result2;

	// Calculate CPU SRTF scheduling
	input_processes();
	process_SRTF();
	
	// Calculate average times
	avg_wait_t /= PROCESSNUM;
	avg_turnaround_t /= PROCESSNUM;

	// Unlocks sempahore so that it has access to FIFO
	sem_post(&semFIFO);
	
	// Opens the FIFO as write only
	char *myFIFO = "/tmp/myFIFO2";
	
	int fd = open(myFIFO, O_WRONLY);
	if (fd < 0)
	{
		perror("open pipeFIFO returned NULL");
		exit(1);
	}

	// Writes average wait time to FIFO
	result1 = write(fd, &avg_wait_t, sizeof(avg_wait_t));

	// Error occurs if write is unsuccessful
	if (result1 == -1)
	{
		perror("Write error");
		exit(1);
	}

	// Writes average turn-around time to FIFO
	result2 = write(fd, &avg_turnaround_t, sizeof(avg_turnaround_t));

	// Error occurs if write is unsuccessful
	if (result2 == -1)
	{
		perror("Write error");
		exit(1);
	}

	close(fd);

	unlink(myFIFO);
}

/* Reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2()
{
	// Creates outputFile, average wait and turn-around times and other variables
	int result1, result2;
	float fifo_avg_wait_t, fifo_avg_turnaround_t;
	FILE *outputFile;

	sem_wait(&semFIFO);

	// Checks if output file exists and returns an error if it does not
		outputFile = fopen("output.txt", "w");
	if (outputFile == NULL)
	{
		perror("File error");
		exit(1);
	}

	char *myFIFO = "/tmp/myFIFO2";

	int fd2 = open(myFIFO, O_RDONLY);
	if (fd2 < 0)
	{
		perror("open pipeFIFO returned NULL");
		exit(1);
	}

	// Reads average wait time from FIFO
	result1 = read(fd2, &fifo_avg_wait_t, sizeof(int));
	if (result1 < 0)
	{
		perror("Read error");
		exit(1);
	}

	// Writes average wait time to output.txt
	fputs("Average Wait Time: ", outputFile);
	fprintf(outputFile, "%f", fifo_avg_wait_t);

	// Reads turn-around time from FIFO
	result2 = read(fd2, &fifo_avg_turnaround_t, sizeof(int));
	if (result2 < 0)
	{
		perror("Read error");
		exit(1);
	}

	// Writes turn-around time to output.txt
	fputs("\nAverage Turnaround Time: ", outputFile);
	fprintf(outputFile, "%f", fifo_avg_turnaround_t);

	// Close the output file
	fclose(outputFile);

	close(fd2);

	remove(myFIFO);

	printProcesses();
}

/* The input data of the CPU scheduling algorithm is:
------------------------------------------------------------
		Process ID				Arrive Time				Burst Time
				1									 8								10
				2									10								 3
				3									14								 7
				4									 9								 5
				5									16								 4
				6									21								 6
				7									26								 2
-------------------------------------------------------------
*/

/* Create process arrive times & burst times, taken from assignment */
void input_processes()
{
	processes[0].pid = 1; processes[0].arrive_t = 8; processes[0].burst_t = 10;
	processes[1].pid = 2; processes[1].arrive_t = 10; processes[1].burst_t = 3;
	processes[2].pid = 3; processes[2].arrive_t = 14; processes[2].burst_t = 7;
	processes[3].pid = 4; processes[3].arrive_t = 9; processes[3].burst_t = 5;
	processes[4].pid = 5; processes[4].arrive_t = 16; processes[4].burst_t = 4;
	processes[5].pid = 6; processes[5].arrive_t = 21; processes[5].burst_t = 6;
	processes[6].pid = 7; processes[6].arrive_t = 26; processes[6].burst_t = 2;

	// Initialise remain_t to be the same as burst_t
	for (i=0; i < PROCESSNUM; i++)
	{
		processes[i].remain_t = processes[i].burst_t;
	}
}

/* Schedule processes according to SRTF rule */
void process_SRTF()
{	
	int endTime, smallest, time, remain = 0;

	// Placeholder for remain_t to be replaced
	processes[7].remain_t = 9999;

	// Run fucntion until remain_t is equal to the number of processes
	for (time = 0; remain != PROCESSNUM; time++)
	{
		// Assign placeholder remain_t as smallest
		smallest = 7;

		// Check all processes that have arrived for lowest remain_t, then set the lowest to be the smallest
		for (i = 0; i < PROCESSNUM; i++)
		{
			if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0)
			{
				smallest = i;
			}
		}

		// Decrease remain_t as time increases
		processes[smallest].remain_t--;

		// If process is finished, save the information, add average to totals & increase remain
		if (processes[smallest].remain_t == 0)
		{
			remain++;

			endTime = time + 1;

			processes[smallest].turnaround_t = endTime - processes[smallest].arrive_t;

			processes[smallest].wait_t = endTime - processes[smallest].burst_t - processes[smallest].arrive_t;

			avg_wait_t += processes[smallest].wait_t;

			avg_turnaround_t += processes[smallest].turnaround_t;
		}
	}
}

/* Print the processes onto the command line */
void printProcesses() {
	
	printf("Process Schedule Table: \n");
	
	printf("\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n");
	
	for (i = 0; i<PROCESSNUM; i++) {
	  	printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].pid,processes[i].arrive_t, processes[i].burst_t, processes[i].wait_t, processes[i].turnaround_t);
	}
}