// Appello 3 del 06/09/2024

#include <stdio.h>     // standard input/output, printf, scanf etc.
#include <stdlib.h>    // atoi, rand, srand, malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE, size_t etc.
#include <signal.h>    // signals
#include <unistd.h>    // fork()
#include <sys/types.h> // defines various types such as pid_t, usually not necessary in modern machines
#include <sys/wait.h>  // defines constants for use with waitpid()
#include <sys/stat.h>  // defines the structure of the data returned by the functions fstat(), lstat(), and stat()
#include <fcntl.h>     // defines the following requests and arguments for use by the functions fcntl() and open()

#define ANSI_COLOR_YELLOW "\x1b[33m" // ANSI escape code for yellow colour in stdout, using yellow for minor errors or warnings
#define ANSI_COLOR_RED "\x1b[31m"    // ANSI escape code for red colour in stdout, using red for errors
#define ANSI_COLOR_RESET "\x1b[0m"   // ANSI escape code to reset the colour to it's default setting

volatile sig_atomic_t signal_received_child = 0; // variable used for robustness and signal safety, check pause() while loop

void handler_child(int sig, siginfo_t *info, void *empty) // signal handler for all childs
{
  if (sig = SIGUSR1)
  {
    signal_received_child = 1; // setting variable as 1 (true) to exit the while loop
  }
}

int main(int argc, char *argv[])
{
  int returnCode = EXIT_SUCCESS; // default return code set to success
  if (argc == 3)                 // exactly two arguments
  {
    int n = atoi(argv[1]); // string -> int
    if (!(n >= 1 && n <= 20))
    {
      returnCode = 11; // return code set to 11, as specified in case <n> is not between 1 and 20 included
      printf(ANSI_COLOR_RED "ERROR: wrong parameter <n>, must be between 1 and 20 included\n" ANSI_COLOR_RESET);
      printf("USAGE: ./a.out <n> <file>\n\ti.e. ./a.out 3 tmp/scriviqui.txt\n");
    }

    FILE *fileArg;
    fileArg = fopen(argv[2], "r+"); // read and write
    if (fileArg != NULL)            // if the file can be opened
    {
      // now that everything is set we can continue
      pid_t parent_PID = getpid(); // retrieving parent PID before fork() so as to be executed first by the original process (parent process)
      for (int i = 0; i < n; i++)
      {
        pid_t child_PID;
        switch ((child_PID = fork()))
        {
        case -1: // fork() returns -1 if there was an error
          printf(ANSI_COLOR_RED "ERROR: fork() error\n" ANSI_COLOR_RESET);
          returnCode = 13; // return code set to 13, custom value to indicate fork error
          break;

        case 0: // fork() returns 0 to the child
          printf("Hello, I'm the child %d, created from parent %d\n", (int)getpid(), (int)getppid());
          parent_PID = getppid();             // setting parent PID
          sigset_t listaBloccati, old;        // defining signal list data structures
          sigfillset(&listaBloccati);         // adding ALL signals to the list
          sigdelset(&listaBloccati, SIGUSR1); // removing specific signals from the list
          sigdelset(&listaBloccati, SIGUSR2);
          sigdelset(&listaBloccati, SIGTSTP);
          // SIG_SETMASK sets the sigset_t as the new mask, SIG_BLOCK blocks the signals in sigset_t, SIG_UNBLOCK removes the signals in sigset_t
          sigprocmask(SIG_SETMASK, &listaBloccati, &old); // we can't directly modify the signal mask but we can do it by using sigprocmask()

          // SIGACTION, signal() is deprecated
          // sa.sa_handler vs sa.sa_sigaction = handler; https://stackoverflow.com/questions/12587621/signal-handler-sa-sigaction-arguments
          struct sigaction struct_child;
          // using sa_flags = SA_SIGINFO, thus no struct_child.sa_handler = handler_child;
          // struct_child.sa_flags = 0; // set to 0 because there is no need for behaviors such as SA_NOCLDSTOP, SA_ONSTACK, SA_RESTART
          struct_child.sa_flags = SA_SIGINFO;
          struct_child.sa_mask = listaBloccati;
          struct_child.sa_sigaction = handler_child;
          sigaction(SIGUSR1, &struct_child, NULL); // what function to call upon signal SIGUSR1

          while (!signal_received_child) // loop to pause(), keeping the childs active without wasting CPU cycles and waiting for a specific signal
          {
            pause(); // waits for any signal but we added the while for specific signal
          }
          exit(EXIT_SUCCESS);
          break;

        default: // parent, fork() returns the child's PID to the parent
          printf("Mr. Parent, my PID is %d\n", (int)getpid());
          // FIFO
          char *fifoName = "/tmp/fifo";
          short int checkFIFO = mkfifo(fifoName, S_IRUSR | S_IWUSR); // creates pipe if it doesn’t exist, set read and write permissions for the user who owns the file, mode const defined in <sys/stat.h>
          short int alreadyExistsFIFO = -1;                          // default to non-existent FIFO
          if (checkFIFO == 0)
          {
            printf("FIFO creation successful\n");
            alreadyExistsFIFO = 0;
          }
          else if (checkFIFO == -1 && alreadyExistsFIFO != 0) // if mkfifo() failed not because it already exists
          {
            perror(ANSI_COLOR_YELLOW "ERROR: FIFO creation failed" ANSI_COLOR_RESET);
          }
          // int fd = open(fifoName, O_RDONLY);             // opens the FIFO, mode const defined in <fcntl.h>
          char messaggio[10]; // short character array since the received messages are i.e. '33-55', '32-62' etc.
          // read(fd, messaggio, 10);                       // read 10 bytes from FIFO
          // printf("- Received message: %s\n", messaggio); // printing message on stdout as specified
          // close(fd);                                     // closes the FIFO
          // unlink(fifoName);                              // deletes the specified FIFO
          break;
        }
      }
    }
    else // if the file COULDN'T be opened
    {
      returnCode = 12; // return code set to 12, as specified in case <file> is not readable
      printf(ANSI_COLOR_RED "ERROR: couldn't read parameter <file>\n" ANSI_COLOR_RESET);
      printf("USAGE: ./a.out <n> <file>\n\ti.e. ./a.out 3 tmp/scriviqui.txt\n");
    }

    /* SAME AS SWITCH BUT WITH IF STATEMENT
    if ((child_PID = fork()) < 0) // fork() returns -1 if there was an error
    {
      printf("ERROR: fork() error\n");
      returnCode = 13; // return code set to 13, custom value to indicate fork error
    }
    else if (child_PID > 0) // we are the parent, fork() returns the child's PID to the parent
    {
      // do FORK
    }
    else // if we are not the parent nor there was an error then it means fork() returned 0, we are the child
    {
      // do NOT fork
    }
    */
  }
  else // wrong number or arguments
  {
    returnCode = 10; // return code set to 10, as specified in case of wrong # of arguments
    printf(ANSI_COLOR_RED "ERROR: wrong number of parameters\n" ANSI_COLOR_RESET);
    printf("USAGE: ./a.out <n> <file>\n\ti.e. ./a.out 3 tmp/scriviqui.txt\n");
  }

  return returnCode;
}