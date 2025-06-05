// PostOffice.c

// Section 0: introduction
// Author: Francesco Dall'Agata
// Test exam made on 04062025, postoffice

// Section 1: index
// 0) introduction
// 1) index
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
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h> // floor(), ceil(), neabyint(), remember to link with gcc <name.c> -lm

// Section 3: define
#define RED "\033[1;31m"
#define RESET "\033[0m"
#define STR_BUFFER 256

// Section 5: global varibales
int origin_pid = 0;  // might be accessed asynchronously
bool verbose = true; // hardcoded, set to true if you want debug messages in stdout
volatile sig_atomic_t received_sigwinch = 0;

// Section 6: function declaration
void handler_lavoratore_SIGUSR1(int signo, siginfo_t *info, void *empty);
void handler_lavoratore_SIGUSR2(int signo, siginfo_t *info, void *empty);
void handler_origin_SIGWINCH(int signo, siginfo_t *info, void *empty);

// Section 7: main
int main(int argc, char *argv[])
{
    int exit_value = EXIT_SUCCESS; // return value with error codes if necessary
    int n = 0;                     // will hold variable <n> passed as an argument
    FILE *fileInput = NULL;        // will hold stream of file <path∕to∕file.txt> passed as an argument
    pid_t pidInput = 0;            // will hold variable <pidInput> passed as an argument
    origin_pid = getpid();         // set this process PID as the origin
    key_t queue_key;               // queue key, wil be used to create a queue
    int queue_id;                  // queue id

    if (verbose && getpid() == origin_pid)
    {
        printf("[origin: %d] sono il processo origine\n", getpid());
    }

    if (argc != 4)
    {
        fprintf(stderr, RED "ERROR: wrong number of parameters\n" RESET);
        printf("USAGE: ./<file.out> <n> </path/to/file.txt> <pidInput>\n");
        exit_value = 10;
        return exit_value;
    }

    // read parameters and check them
    n = atoi(argv[1]);
    fileInput = fopen(argv[2], "r");
    pidInput = atoi(argv[3]);

    // check if n is between 1 and 10 included
    if (atoi(argv[1]) < 1 || atoi(argv[1]) > 10)
    {
        fprintf(stderr, RED "ERROR: invalid <n> parameter, it must be between 1-10\n" RESET);
        printf("USAGE: ./<file.out> <n> </path/to/file.txt> <pidInput>\n");
        exit_value = 11;
        return exit_value;
    }

    // check if file exists
    if (fileInput == NULL)
    {
        fprintf(stderr, RED "ERROR: couldn't open file, please review </path/to/file.txt>\n" RESET);
        printf("USAGE: ./<file.out> <n> </path/to/file.txt> <pidInput>\n");
        exit_value = 12;
        return exit_value;
    }

    // check if PID exists, if signal is 0 then no signal is sent but error checking is done
    if (kill(pidInput, 0) != 0)
    {
        fprintf(stderr, RED "ERROR: specified process does not exist, please review <pidInput>\n" RESET);
        printf("USAGE: ./<file.out> <n> </path/to/file.txt> <pidInput>\n");
        exit_value = 13;
        return exit_value;
    }

    queue_key = ftok(argv[2], origin_pid); // generate queue key, la chiave sarà la stessa se path e PID sono uguali ma visto che il PID cambia ad ogni esecuzione essa sarà unica per questo processo
    queue_id = msgget(queue_key, 0777);    // creiamo una coda utilizzando la chiave
    pid_t array_childs[n];
    pid_t lavoratore_pid = 0; // variable that stores fork() value

    // since the parent immediately starts sending singals we need to wait for the childs to set up the masks and handlers properly
    //  block SIGWINCH (28), will remain pending until we do something
    sigset_t block_SIGWINCH, old;
    sigemptyset(&block_SIGWINCH);
    sigaddset(&block_SIGWINCH, SIGWINCH);
    sigprocmask(SIG_BLOCK, &block_SIGWINCH, &old);

    // generazione lavoratori (figli)
    for (int i = 0; i < n && getpid() == origin_pid; i++) // if I'm the origin I will generate <n> lavoratori processes
    {
        lavoratore_pid = fork();
        switch (lavoratore_pid)
        {
        case 0:
            // lavoratore
            // al momento della creazione, ogni lavoratore manda SIGTERM esclusivamente al processo con PID = <pidInput>
            kill(pidInput, SIGTERM);
            if (verbose)
            {
                printf("[lavoratore %d] ho mandato SIGTERM a %d\n", getpid(), pidInput);
            }

            if (getpid() != origin_pid)
            {
                // lavoratore has to receive only 1 SIGUSR1, either using SA_RESETHND flag or setting the mask again when receiving the signal
                struct sigaction sa_lavoratore_SIGUSR1;
                sigemptyset(&sa_lavoratore_SIGUSR1.sa_mask);
                sa_lavoratore_SIGUSR1.sa_sigaction = handler_lavoratore_SIGUSR1;
                sa_lavoratore_SIGUSR1.sa_flags = SA_SIGINFO | SA_RESETHAND;
                sigaction(SIGUSR1, &sa_lavoratore_SIGUSR1, NULL);

                // lavoratore has to receive many SIGUSR2
                struct sigaction sa_lavoratore_SIGUSR2;
                sigemptyset(&sa_lavoratore_SIGUSR2.sa_mask);
                sa_lavoratore_SIGUSR2.sa_sigaction = handler_lavoratore_SIGUSR2;
                sa_lavoratore_SIGUSR2.sa_flags = SA_SIGINFO;
                sigaction(SIGUSR2, &sa_lavoratore_SIGUSR2, NULL);

                // lavoratore will receive a signal, read the next line of the file, send the line as a message in the queue
                sigset_t wait_SIGUSR2;
                sigfillset(&wait_SIGUSR2);
                sigdelset(&wait_SIGUSR2, SIGUSR2);

                while (true)
                {
                    printf("[%d] waiting for SIGUSR2\n", getpid());
                    sigsuspend(&wait_SIGUSR2);
                    char buffer[STR_BUFFER];
                    if (fileInput != NULL && !feof(fileInput))
                    {
                        printf("[%d] reading new line from file\n", getpid());
                        fgets(buffer, sizeof(buffer), fileInput);
                        printf("read: %s", buffer);
                    }
                }
            }

            break;

        default:
            // origin
            if (verbose && getpid() == origin_pid)
            {
                printf("[origin: %d] ho generato il lavoratore %d\n", getpid(), lavoratore_pid);
            }
            // insert child in array
            array_childs[i] = lavoratore_pid;
            break;
        }
    }

    // ora tutti i figli sono stati generati

    // origin
    if (getpid() == origin_pid)
    {
        // origin will wait for SIGWINCH

        // handler
        struct sigaction sa_origin;
        sa_origin.sa_sigaction = handler_origin_SIGWINCH;
        sa_origin.sa_flags = SA_SIGINFO;
        sigemptyset(&sa_origin.sa_mask);
        sigaction(SIGWINCH, &sa_origin, NULL);

        // block
        sigset_t block_set, old_set;
        sigemptyset(&block_set);
        sigaddset(&block_set, SIGWINCH);
        sigprocmask(SIG_BLOCK, &block_set, &old_set);

        // suspend
        // set up waitmask to wait only for SIGWINCH
        sigset_t wait_SIGWINCH;
        sigfillset(&wait_SIGWINCH);          // fill it with everything
        sigdelset(&wait_SIGWINCH, SIGWINCH); // SIGWINCH can go trough and be handled immediately
        while (!received_sigwinch)
        {
            printf("[origin %d] waiting for SIGWINCH(28), %s\n", getpid(), (getpid() == origin_pid) ? "correct" : "incorrect");
            sigsuspend(&wait_SIGWINCH); // wait just this once for SIGWINCH, then go on
        }

        //  origin will send a signal one after the other to a lavoratore until the file is empty, i.e. 20 lines and 5 lavoratori = 20 messages in total, each lavoratore will do 4 messages
        int i = 0;
        while (fileInput != NULL && !feof(fileInput)) // se il puntatore è valido e non siamo alla fine del file
        {
            if (!(i < n)) // se i non è un indice dell'array valido lo resetto
            {
                i = 0;
            }
            printf("[origin %d] sending SIGUSR2 to %d\n", getpid(), array_childs[i]);
            kill(array_childs[i], SIGUSR2);
            i++;
            // after sending a signal to print we have to wait a second
            sleep(1);
        }
    }

    return exit_value;
}

void handler_lavoratore_SIGUSR1(int signo, siginfo_t *info, void *empty)
{
    if (verbose)
    {
        printf("[%d] Received %d, it should be SIGUSR1 = 10\n", getpid(), signo);
        printf("[%d] Sending %d back, %s\n", getpid(), signo, (signo == SIGUSR1) ? "correct" : "incorrect");
    }
    kill(info->si_pid, SIGUSR1);
}

void handler_lavoratore_SIGUSR2(int signo, siginfo_t *info, void *empty)
{
    if (verbose)
    {
        printf("[%d] Received %d, it should be SIGUSR2 = 12\n", getpid(), signo);
        printf("[%d] Sending %d back, %s\n", getpid(), signo, (signo == SIGUSR2) ? "correct" : "incorrect");
    }
    kill(info->si_pid, SIGUSR2);
}

void handler_origin_SIGWINCH(int signo, siginfo_t *info, void *empty)
{
    if (getpid() == origin_pid)
    {
        printf("[origin %d] received %d, should be SIGWINCH(28)\n", getpid(), signo);
    }
}
