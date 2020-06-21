/*************************RTOS_AUT2020_ASSIGNMENT_2_PROGRAM_******************************
 
  Name: Kathryn Corpuz
	Student Number: 13215785

  This program gets reads each line (that is not part of the header) and writes each line 
  to a new file. This is done with 3 pipes, 3 semaphores and shared memory, as it is completed within a 
  loop, line by line sequentially, until there are no more lines to read. 

  To compile this program ensure that gcc is installed and run the following command:
  gcc prog_1.c -o p1 -lpthread -lrt

	To run Program_1, perform the following on the command line 
	./p1 output.txt

****************************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/time.h>

/* --- Structs --- */

typedef struct ThreadParams
{
  int pipeFile[2];
  sem_t sem_A_to_B, sem_B_to_C, sem_C_to_A;
  char message[255];

} ThreadParams;

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void *ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void *ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void *ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[])
{

  int result;
  pthread_t tid1, tid2, tid3; //Thread ID
  pthread_attr_t attr;

  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

  // Create pipe
  result = pipe(params.pipeFile);
  if (result < 0)
  {
    perror("pipe error");
    exit(1);
  }

  // Create Threads
  if (pthread_create(&tid1, &attr, ThreadA, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid2, &attr, ThreadB, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid3, &attr, ThreadC, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  //TODO: add your code

  //Checks if the correct number of params has been entered in the command line
  if (argc != 2)
  {
    fprintf(stderr, "Error! Please do: ./prog_1.c data.txt");
  }

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  //TODO: add your code

  printf("Threads have completed. Please view output.txt file to view lines.\n");

  return 0;
}

void initializeData(ThreadParams *params)
{
  // Initialize Sempahores
  sem_init(&(params->sem_A_to_B), 0, 1);
  sem_init(&(params->sem_B_to_C), 0, 0);
  sem_init(&(params->sem_C_to_A), 0, 0);

  //TODO: add your code

  return;
}

//Reads data from the file and writes to a pipe
void *ThreadA(void *params)
{
  FILE *fptr;
  fptr = fopen("data", "r");
  ThreadParams *A_thread_params = (ThreadParams *)(params);
  int result;
  char line[255];

  if (fptr == NULL ) 
  {
  printf("Error opening file");
    exit(-1);
  }
  
  //Reads from file
  while (fgets(line, sizeof(line), fptr) != NULL)
  {
    //Unlocks semaphore - decrements its value by 1
    sem_wait(&(A_thread_params->sem_A_to_B));

    //Writes line to the pipe
    result = write(A_thread_params->pipeFile[1], line, sizeof(line));

    //Check if line was written to pipe successfully
    if (result == -1)
    {
      perror("write");
      exit(-1);
    }

    //Informs sem_B_to_C that it can unlock - increments the value of the semaphore by 1
    sem_post(&(A_thread_params->sem_B_to_C));
    
  }

  fclose(fptr);

  printf("ThreadA is processing\n");
}

void *ThreadB(void *params)
{
  //TODO: add your code
  int result;
  ThreadParams *B_thread_params = (ThreadParams *)(params);
  
  while (strcmp(B_thread_params->message, "\n") != 0)
  {
  sem_wait(&(B_thread_params->sem_B_to_C));

  //Reads from the pipe
    result = read(B_thread_params->pipeFile[0], B_thread_params->message, sizeof(B_thread_params->message));

    if (result == -1)
    {
      perror("read");
      exit(-1);
    }

  //Informs sem_C_to_A that it can unlock
  sem_post(&(B_thread_params->sem_C_to_A));
  
  }

  printf("ThreadB is processing\n");

}

void *ThreadC(void *params)
{
  //TODO: add your code
  FILE *newFile;
  newFile = fopen("output.txt", "w");
  char check[12] = "end_header\n";
  int sig = 0;
  ThreadParams *C_thread_params = (ThreadParams *)(params);

  //Reads message until it has reached the end of the line and writes it to output file if not part of header
  while (strcmp(C_thread_params->message, "\n") != 0)
  {
    //Unlocks sem_C_to_A
    sem_wait(&(C_thread_params->sem_C_to_A));

    if (sig == 1) {
      fputs(C_thread_params->message, newFile);
    //   fprintf(newFile, "%s", C_thread_params->message);
    }

    if ((sig == 0) && strcmp(C_thread_params->message, check) == 0)
    {
      sig = 1;
    }

    sem_post(&(C_thread_params->sem_A_to_B));

  }

  fclose(newFile);

  //Informs sem_A_to_B that it can unlock again
  
  printf("ThreadC is processing\n");

  return 0;
}
