// main_19062025.c

// Section 0: author & introduction
// Author: Francesco Dall'Agata
// Introduction: exam of 11/06/2025 of Sistemi Operativi (Laboratorio) by PhD M. Grisafi

// Section 1: index
// 0) index
// 1) author & introduction
// 2) include
// 3) define
// 4) typedef
// 5) global variables
// 6) function declaration
// 7) main
// 8) function definition

// Section 2: include
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h> // to access errno
#include <stdbool.h>
#include <unistd.h>
#include <sys/msg.h> // queue

// Section 3: define
#define STR_DEFAULT 128

// Section 4: typedef

// Section 5: global variables
pid_t timonierePID = 0;
unsigned short int n = 0;

// Section 6: function declaration
void vedetta();               // manages the tasks of process 'vedetta'
void arpioniere(pid_t group); // manages the tasks of each process 'arpioniere'
void handler_vedetta_avvistamento(int signo, siginfo_t *info, void *empty);

// Section 7: main
int main(int argc, char *argv[])
{
  // process 'nave'
  // request #1, check if parameters are present, <timonierePID> and <n>, if not exit with code 90
  unsigned short int exit_code = EXIT_SUCCESS; // assume success by default
  if (argc != 3)                               // if there are exactly two parameters in addition to the program itself
  {
    fprintf(stderr, "ERROR: missing parameters\nUSAGE: %s <timonierePID> <n>\n", argv[0]);
    exit_code = 90; // request #1, a different count of parameters shall terminate the program with code 90
    return exit_code;
  }

  // assign parameters to variables for readability
  char *end_ptr;                               // stores termination character '\0' for strtol error checking
  timonierePID = strtol(argv[1], end_ptr, 10); // stores the input parameter <timonierePID>
  n = strtol(argv[2], end_ptr, 10);            // sotres the input parameter <n>

  // check if parameter <timonierePID> is correct
  if (kill(timonierePID, 0) != 0) // with signal = 0  no signal is actually sent but error checking is performed
  {
    fprintf(stderr, "ERROR: invalid parameter <timonierePID> = %d, it must be an existing PID\nUSAGE: %s <timonierePID> <n>\n", argv[0], argv[1]);
    exit_code = 91; // custom exit code
    return exit_code;
  }

  // check if parameter <n> is correct
  if (n <= 0) // n must be at least 1
  {
    fprintf(stderr, "ERROR: invalid parameter <n>, it must be greater than 0\nUSAGE: %s <timonierePID> <n>\n", argv[0], argv[2]);
    exit_code = 92; // custom exit code
    return exit_code;
  }

  // request #2, process 'nave' must generate a new process 'vedetta'
  pid_t nave_pid, vedetta_pid;
  nave_pid = getpid();  // store the PID of process 'nave'
  vedetta_pid = fork(); // create a new process with fork(), 'nave' will receive the PID while the new process 'vedetta' will receive 0

  if ((vedetta_pid == 0) && (getpid() != nave_pid)) // if this is the new process
  {
    vedetta();
    return EXIT_SUCCESS; // extra layer of protection
  }

  // request #3, process 'nave' must create <n> child processes, they are 'arpionieri'
  bool guard_exit = false; // guard to exit for statement
  pid_t arpionieri_pid[n]; // used by 'nave' to store child processes information
  char folder[STR_DEFAULT] = "./tmp/registrazione/";
  char file_registrazione[STR_DEFAULT] = {0};
  for (int i = 0; i < n && !guard_exit; i++)
  {
    arpionieri_pid[i] = fork();
    if (arpionieri_pid[i] == 0) // if it is the child process
    {
      guard_exit = true; // set guard to exit the cycle
      if (i == 0)        // if this is the first process let's make it leader
      {
        arpioniere(0); // call function for process 'arpioniere' with 0 to make it captain
      }
      else
      {
        arpioniere(arpionieri_pid[0]); // call function for process 'arpioniere' to join group of captain
      }

      return EXIT_SUCCESS; // extra layer of safeness
    }
  }

  return exit_code;
}

// Section 8: function definition
void vedetta() // manages the tasks of process 'vedetta'
{
  // request #2, when created process 'vedetta' must send SIGUSR1 to <timonierePID>
  kill(timonierePID, SIGUSR1);

  // request #5, upon seeing a whale (balena) via SIGUSR2, process 'vedetta' must print on stdout
  struct sigaction sa_vedetta_avvistamento;
  sigemptyset(&sa_vedetta_avvistamento.sa_mask);        // ensure the signal mask is empty
  sigaddset(&sa_vedetta_avvistamento.sa_mask, SIGUSR2); // add SIGUSR2 to the mask so that the same signal can't interrupt the current execution of the handler
  sa_vedetta_avvistamento.sa_flags = SA_SIGINFO;
  sa_vedetta_avvistamento.sa_sigaction = handler_vedetta_avvistamento;
  sigaction(SIGUSR2, &sa_vedetta_avvistamento, NULL);

  // request #6, process 'vedetta' must read on a queue the type and distance of the whale
  key_t queue_key = 
  msgget();

  exit(EXIT_SUCCESS);
}

void arpioniere(pid_t group) // manages the tasks of process 'arpioniere'
{

  // request #4, all harpooners must be part of the same squad and there is always a captain harpooner
  if (group == 0) // if group is 0 the process will become captain
  {
    setpgid(0, 0); // the process itself uses it's own PID and becomes part of the process's own PID group, thus becoming group leader
  }
  else
  {
    setpgid(0, group); // the process itself uses it's own PID and joins the specified group passed to the function
  }

  exit(EXIT_SUCCESS);
}

void handler_vedetta_avvistamento(int signo, siginfo_t *info, void *empty)
{
  printf("Balena!\n");
}
