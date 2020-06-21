/***********************RTOS_AUT2020_ASSIGNMENT_3_PROGRAM_2***********************

Name: Kathryn Corpuz
Student Number: 13215785

This program uses the First-In-First-Out algorithm for page replacement in virtual
memory management. 

To compile prog_1 ensure that gcc is installed and run the following command:
gcc program_2.c -o prog_2 -lpthread -lrt

To run Program_2, run the following on the command line
./prog_2 4

**********************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

//Number of pagefaults in the program
int pageFaults = 0;

//Function declaration
void SignalHandler(int signal);

/* Main routine for the program - sets up reference string, num of frames and the FIFO

 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program.
 @return returns 0 upon completion.
 */

int main(int argc, char *argv[])
{
  //Register Ctrl+c(SIGINT) signal and call the signal handler for the function.
  signal(SIGINT, SignalHandler);

  if (argc != 2)
  {
    printf("Error! Please do: .prog_2 4");
  }

  // Iteration variables
  int i, j, k;
  
  // Extra variables (same - in memory or not, flag = if page fault occured or not)
  int same, flag;

  // Reference number
  int REFERENCESTRINGLENGTH = 24;

  // Argument from the user on the frame size, such as 4 frames in the document
  int frameSize = atoi(argv[1]);

  // Frame where we will be storing the references. -1 is equivalent to an empty value
  int frame[REFERENCESTRINGLENGTH];

  // Reference string from the assignment outline
  int referenceString[24] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1, 7, 5};

  // Initialise the empty frame with -1 to simulate empty values
  for (i = 0; i < frameSize; i++)
  {
    frame[i] = -1;
  }

  // Print headings
  printf("\n Frame 1        Frame 2         Frame 3         Frame 4       Page Fault");
  printf("\n--------------------------------------------------------------------------");

  // Loop through the reference string aka pages
  for (j = 0; j < REFERENCESTRINGLENGTH; j++)
  {
    same = 0;
    flag = 0;

    // Checks if pages are already in memory
    for (i = 0; i < frameSize; i++)
    {
      if (referenceString[j] == frame[i])
      {
        same++;
        pageFaults--;
      }
    }

    // If not, pageFaults increase
    pageFaults++;

    // The initial pageFault identifications, when -1 still exists in the table
    if ((pageFaults <= frameSize) && (same == 0))
    {
      frame[j] = referenceString[j];
      flag = 1;
    }
    // The FIFO portion - determine the oldest value in memory and swaps it with current value
    else if (same == 0)
    {
      frame[(pageFaults - 1) % frameSize] = referenceString[j];
      flag =1;
    }

    // Print each value for each frame indicates whether there was a page fault or not
    printf("\n");
    for (i = 0; i < frameSize; i++)
    {
      printf("%d\t\t", frame[i]);
    }
    if (flag == 1)
      {
        printf("%s\t\t", "Yes");
      }
      else
      {
        printf("%s\t\t", "");
      }
  }

  //Sit here until the ctrl+c signal is given by the user.
  while (1)
  {
    sleep(1);
  }

  return 0;
}

/**
 Performs the final print when the signal is received by the program.

 @param signal An integer values for the signal passed to the function.
 */
void SignalHandler(int signal)
{
  if (signal == SIGINT)
  {
    printf("\n Caught Ctrl+c signal. \nTotal Number of Page Faults: \t%d\n", pageFaults);
    exit(0);
  }
}
