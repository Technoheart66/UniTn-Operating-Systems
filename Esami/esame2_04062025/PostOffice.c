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
#define MSG_NOFLAG 0 // purely for readability when setting flags in queues

// Section 4: typedef
typedef struct msg_queue
{
    long mtype; // when defining the queue message struct the first field MUST be a long
    char mtext[STR_BUFFER];
} MsgQueue; // Pascal case

// Section 5: global varibales
int origin_pid = 0;                      // might be accessed asynchronously
bool verbose = true;                     // hardcoded, set to true if you want debug messages in stdout
volatile sig_atomic_t flag_sigwinch = 0; // treated as binary flag to be set upon receiving SIGWINCH(28)
volatile sig_atomic_t flag_terminate_lavoratore = 0;

// Section 6: function declaration
void handler_lavoratore_SIGUSR1(int signo, siginfo_t *info, void *empty);
void handler_lavoratore_SIGUSR2(int signo, siginfo_t *info, void *empty);
void handler_origin_SIGWINCH(int signo, siginfo_t *info, void *empty);
void handler_lavoratore_SIGWINCH(int signo, siginfo_t *info, void *empty);

// Section 7: main
int main(int argc, char *argv[])
{
    int exit_value = EXIT_SUCCESS; // return value with error codes if necessary
    int n = 0;                     // will hold variable <n> passed as an argument
    FILE *fileInput = NULL;        // will hold stream of file <path∕to∕file.txt> passed as an argument
    pid_t pidInput = 0;            // will hold variable <pidInput> passed as an argument
    origin_pid = getpid();         // set this process PID as the origin
    key_t queue_key;               // queue key, wil be used to create a queue
    int queue_id;                  // queue id, main queue where lavoratori (childs) will print the content of the file line by line
    int hidden_queue_id;           // this queue is used to syncrhonize the read with parent and children

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

    if (verbose && getpid() == origin_pid)
    {
        printf("[origin] sono il processo origine, il mio PID: %d\n", getpid());
    }

    queue_key = ftok(argv[2], origin_pid); // generate queue key, la chiave sarà la stessa se path e PID sono uguali ma visto che il PID cambia ad ogni esecuzione essa sarà unica per questo processo
    queue_id = msgget(queue_key, 0777);    // creiamo una coda utilizzando la chiave
    hidden_queue_id = msgget(IPC_PRIVATE, 0666);
    pid_t array_childs[n];
    pid_t lavoratore_pid = 0; // variable that stores fork() value

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

            // lavoratore will receive a signal SIGWINCH sent from origin (PostOffice), read the next line of the file, send the line as a message in the queue

            // SIGUSR1 handler for lavoratore, it has to receive only 1 SIGUSR1, either using SA_RESETHND flag or setting the mask again when receiving the signal
            struct sigaction sa_lavoratore_SIGUSR1;
            sigemptyset(&sa_lavoratore_SIGUSR1.sa_mask);
            sa_lavoratore_SIGUSR1.sa_sigaction = handler_lavoratore_SIGUSR1;
            sa_lavoratore_SIGUSR1.sa_flags = SA_SIGINFO | SA_RESETHAND;
            sigaction(SIGUSR1, &sa_lavoratore_SIGUSR1, NULL);

            // SIGUSR2 handler for lavoratore, it has to receive many SIGUSR2
            struct sigaction sa_lavoratore_SIGUSR2;
            sigemptyset(&sa_lavoratore_SIGUSR2.sa_mask);
            sa_lavoratore_SIGUSR2.sa_sigaction = handler_lavoratore_SIGUSR2;
            sa_lavoratore_SIGUSR2.sa_flags = SA_SIGINFO;
            sigaction(SIGUSR2, &sa_lavoratore_SIGUSR2, NULL);

            // SIGWINCH handler for lavoratore, it has to handle SIGWINCH and print one line
            struct sigaction sa_lavoratore_SIGWINCH;
            sa_lavoratore_SIGWINCH.sa_sigaction = handler_lavoratore_SIGWINCH;
            sa_lavoratore_SIGWINCH.sa_flags = SA_SIGINFO;
            sigemptyset(&sa_lavoratore_SIGWINCH.sa_mask);
            sigaction(SIGWINCH, &sa_lavoratore_SIGWINCH, NULL);

            // block (set as pending) all relevant signals to avoid race conditions
            sigset_t lavoratore_block;
            sigemptyset(&lavoratore_block);
            sigaddset(&lavoratore_block, SIGUSR1);
            sigaddset(&lavoratore_block, SIGUSR2);
            sigaddset(&lavoratore_block, SIGWINCH);

            // sigsuspend mask for lavoratore, it has to wait SIGUSR1, SIGUSR2 and SIGWINCH
            sigset_t lavoratore_wait_SIGWINCH;
            sigfillset(&lavoratore_wait_SIGWINCH);
            sigdelset(&lavoratore_wait_SIGWINCH, SIGUSR1);
            sigdelset(&lavoratore_wait_SIGWINCH, SIGUSR2);
            sigdelset(&lavoratore_wait_SIGWINCH, SIGWINCH);
            sigdelset(&lavoratore_wait_SIGWINCH, SIGTERM); // the parent will tell us when to terminate

            // sigsupend, waiting for a signal to print the content
            while (!flag_terminate_lavoratore) // we only exit when a flag is set, the parent will kill this process using SIGTERM. the process can terminate by setting it's own flag when it's needed
            {
                printf("[%d] waiting for SIGUSR1(10), SIGUSR2(12) and SIGWINCH(28)\n", getpid());
                sigsuspend(&lavoratore_wait_SIGWINCH);
                if (flag_sigwinch)
                {
                    MsgQueue rcv, snd;
                    printf("[lavoratore %d] trying to read from hidden queue\n", getpid());
                    msgrcv(hidden_queue_id, &rcv, sizeof(rcv.mtext), origin_pid, 0);
                    // replace print with msgsnd in main queue
                    // set type and text content
                    snd.mtype = getpid();
                    snprintf(snd.mtext, sizeof(snd.mtext), "%s", rcv.mtext);
                    printf("[lavoratore %d] read from hidden queue: %s", getpid(), snd.mtext);
                    msgsnd(queue_id, &snd, sizeof(snd.mtext), MSG_NOFLAG);
                }
            }
            break;

        default:
            // origin
            if (verbose && getpid() == origin_pid)
            {
                printf("[origin] ho generato il lavoratore %d\n", lavoratore_pid);
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
        sigemptyset(&sa_origin.sa_mask); // mask that is used ONLY DURING signal handling, to avoid race condition during signal handling we block other signals
        // let's fill the mask in case other signals that I care about arrive, in this case Postoffice only cares about SIWINCh
        sigaddset(&sa_origin.sa_mask, SIGWINCH); // redundant
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
        sigdelset(&wait_SIGWINCH, SIGINT);   // I also want SIGINT (CTRL+C) to go trough and terminate the process 8default behavior)
        sigdelset(&wait_SIGWINCH, SIGWINCH); // SIGWINCH can go trough and be handled immediately
        while (!flag_sigwinch)               // to avoid race conditions (potential deadlock) I check a flag that will be set inside the signal handler. If the signal arrived before the flag is set and we don't wait with sigsuspend
        {
            printf("[origin] waiting for SIGWINCH(28), %s\n", (getpid() == origin_pid) ? "correct" : "incorrect");
            sigsuspend(&wait_SIGWINCH); // wait just this once for SIGWINCH, then go on
            printf("[%s] received SIGWINCH(28)\n", (getpid() == origin_pid) ? "origin" : "");
        }

        //  origin will send a signal one after the other to a lavoratore until the file is empty, i.e. 20 lines and 5 lavoratori = 20 messages in total, each lavoratore will do 4 messages
        int i = 0;
        while (fileInput != NULL && !feof(fileInput)) // se il puntatore è valido e non siamo alla fine del file
        {
            if (!(i < n)) // se i non è un indice dell'array valido lo resetto
            {
                i = 0;
            }

            // read the line from file
            char buffer[STR_BUFFER];
            printf("[origin] reading new line from file\n");
            fgets(buffer, sizeof(buffer), fileInput); // read the line
            size_t len = strlen(buffer);              // calculate the length of the string
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[len - 1] = '\0';
            }
            printf("[origin] read from file: {%s}", buffer);

            // send a message on the  hidden queue containing the read content
            MsgQueue msg;
            msg.mtype = getpid(); // should be origin
            snprintf(msg.mtext, sizeof(msg.mtext), "%s", buffer);
            msgsnd(hidden_queue_id, &msg, sizeof(msg.mtext), 0);

            // send a signal to a lavoratore, it should read from the hidden queue and print in the main queue
            printf("[origin] sending SIGWINCH(28) to %d\n", array_childs[i]);
            kill(array_childs[i], SIGWINCH);
            i++;
            // after sending a signal to print we have to wait a second
            sleep(1);
        }

        // since we read the whole file, let's terminate all child processes
        for (int i = 0; i < n; i++)
        {
            if (verbose)
            {
                printf("[origin] killing process %d\n", array_childs[i]);
            }
            kill(array_childs[i], SIGTERM);
        }

        // ensure all childs are terminated by waiting for them using waitpid

        pid_t exiting_pid;
        int child_status;

        while ((exiting_pid = waitpid(-1, &child_status, 0)) > 0)
        {
            if (WIFEXITED(child_status))
            {
                printf("Child %d exited with code %d\n", exiting_pid, WEXITSTATUS(child_status));
            }
            else if (WIFSIGNALED(child_status))
            {
                printf("Child %d was killed by signal %d\n", exiting_pid, WTERMSIG(child_status));
            }
        }

        // delete queues
        msgctl(queue_id, IPC_RMID, NULL);
        msgctl(hidden_queue_id, IPC_RMID, NULL);
    }

    return exit_value;
}

void handler_lavoratore_SIGUSR1(int signo, siginfo_t *info, void *empty)
{
    if (verbose && (getpid() != origin_pid))
    {
        printf("[%d] Received %d, it should be SIGUSR1 = 10\n", getpid(), signo);
        printf("[%d] Sending %d back, %s\n", getpid(), signo, (signo == SIGUSR1) ? "correct" : "incorrect");
    }
    // kill(info->si_pid, SIGUSR1);
}

void handler_lavoratore_SIGUSR2(int signo, siginfo_t *info, void *empty)
{
    if (verbose && (getpid() != origin_pid))
    {
        printf("[%d] Received %d, it should be SIGUSR2 = 12\n", getpid(), signo);
        printf("[%d] Sending %d back, %s\n", getpid(), signo, (signo == SIGUSR2) ? "correct" : "incorrect");
    }
    // kill(info->si_pid, SIGUSR2);
}

void handler_origin_SIGWINCH(int signo, siginfo_t *info, void *empty)
{
    if (verbose && (getpid() == origin_pid))
    {
        printf("[origin] received %d, should be SIGWINCH(28): %s\n", signo, (signo == SIGWINCH) ? "correct" : "incorrect");
        flag_sigwinch = 1;
    }
}

void handler_lavoratore_SIGWINCH(int signo, siginfo_t *info, void *empty)
{
    if (verbose && (getpid() != origin_pid))
    {
        printf("[%d] Inside handler_lavoratore_SIGWINCH\n", getpid());
        flag_sigwinch = 1;
    }
}
