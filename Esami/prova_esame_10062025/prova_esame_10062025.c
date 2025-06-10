// credentials.c

// Section 0: author & introduction
// Introduction: attempt of exam test of 09062025

// Section 1: index
// 0) author & introduction
// 1) index
// 2) include
// 3) define
// 4) typedef
// 5) global variables
// 6) function declaration
// 7) main
// 8) function definition

// Section 2: include
#include <stdlib.h> // standard C library with many types and functions, EXIT_SUCCESS, EXIT_FAILURE, malloc(), free(), exit(), etc.
#include <stdio.h>  // standard C library for Input/Output operations
#include <string.h> // standard C library for string operations
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Section 3: define
#define STR_DEFAULT 64 // string length not specified, setting custom defualt valueu

// Section 4: typedef
typedef struct thread_args
{
    char username[STR_DEFAULT];
    char password[STR_DEFAULT];
} ThreadArgs;

// Section 5: global variables

// Section 6: function declaration
void handler_SIGTSTP(int signo, siginfo_t *info, void *empty); // custom handler to handle the signal SIGTSTP
void *thread_wait_password(void *arg);                         // threaded; waits indefinitely to read from a FIFO

// Section 7: main
int main(int argc, char *argv[])
{
    unsigned short int exit_status = EXIT_SUCCESS; // stores the exit status of the program

    // check arguments count: there must be no arguments
    if (argc != 1) // 1 means only the program itself
    {
        fprintf(stderr, "ERROR: invalid arguments, there should be no arguments\nUSAGE: ./a.out\n");
        exit_status = 101; // custom error value, arguments
        exit(exit_status); // terminate
    }

    // open file to read
    FILE *input_file = NULL;                          // pointer to the file stream that will be opened
    char *credentials_file = "./tmp/credentials.txt"; // hardcoded, string literal, read-only
    input_file = fopen(credentials_file, "r");        // open stream, fopen returns a file pointer on success, NULL on failure
    if (input_file == NULL)                           // check if file opening succeded
    {
        fprintf(stderr, "ERROR: couldn't open input file %s\n", credentials_file);
        exit_status = 102; // custom error value, credentials file
        exit(exit_status);
    }

    // read file, get username and password
    char username[STR_DEFAULT] = {0}; // stores the username read from file
    char password[STR_DEFAULT] = {0}; // stores the password read from file
    if (!feof(input_file))            // if we are not at the chaarcter end of file (EOF)
    {
        fgets(username, STR_DEFAULT, input_file);
        fgets(password, STR_DEFAULT, input_file);
    }
    else
    {
        fprintf(stderr, "ERROR: couldn't read file, check contents\n");
    }

    // request 8) create queue (coda)
    char * queue_path = "∕tmp∕login.fifo";
    key_t queue_key = ftok(queue_path, 51);
    int queue_id = msgget(queue_key, IPC_CREAT); // IPC_CREAT: if it exists then return it's id, otherwise create a new one

    // write to new file, copy the password in a new file
    FILE *new_file = NULL;
    char *secret_file = "./tmp/secret.txt"; // hardcoded string literal, read-only
    new_file = fopen(secret_file, "w");     // open stream, fopen returns a file pointer on success, NULL on failure
    if (new_file == NULL)
    {
        fprintf(stderr, "ERROR: couldn't open secret file %s\n", secret_file);
        exit_status = 103; // custom error value, secret file
        exit(exit_status);
    }
    fwrite(password, sizeof(char), strlen(password) + 1, new_file);

    // delete the file using shell with system(), or just use remove()
    // char command[STR_DEFAULT] = {0};
    // snprintf(command, sizeof(command), "rm %s", secret_file);
    // int shell_status = system(command);
    // if (shell_status)
    // {
    //     /* code */
    // } else {}
    if (remove(secret_file) != 0)
    {
        fprintf(stderr, "ERROR: couldn't remove credentials file\n");
        exit_status = 104; // custom error value, file removal
        exit(exit_status);
    }

    // handling CTRL+Z (SIGTSTP), upon receiving SIGTSTP the program must terminate instead of waiting
    struct sigaction sa_stop;
    sigemptyset(&sa_stop.sa_mask);          // empty the mask to be used during signal handling
    sigaddset(&sa_stop.sa_mask, SIGTSTP);   // if I receive another SIGTSTP it will be blocked until signal handling terminates
    sa_stop.sa_flags = SA_SIGINFO;          // send additional information, use together with .sa_sigaction and sication(signal)
    sa_stop.sa_sigaction = handler_SIGTSTP; // specify custom handler to be used with sigaction()
    sigaction(SIGTERM, &sa_stop, NULL);

    // request 4) create FIFO with read and write permission then write inside the PID
    char fifo_authenticator[STR_DEFAULT] = "∕tmp∕authenticator.fifo";
    if (mkfifo(fifo_authenticator, S_IWUSR | S_IRUSR) == -1)
    {
        fprintf(stderr, "ERROR: couldn't open fifo %s\n", fifo_authenticator);
        exit_status = 105; // custom error value, fifo authenticator
        exit(exit_status);
    }
    int fifo_id_authenticator = open(fifo_authenticator, O_WRONLY);       // open the FIFO in read only
    char write_msg_fifo[STR_DEFAULT] = {0};                               // will store the message to be sent in the fifo
    snprintf(write_msg_fifo, sizeof(write_msg_fifo), "%d", getpid());     // put the PID in string format inside the message
    write(fifo_id_authenticator, write_msg_fifo, sizeof(write_msg_fifo)); // write message in fifo

    // request 6) create new thread in which the application should wait indefinitely
    // set up arguments to be later cast to void in order to send to threaded function
    pthread_t thread;
    ThreadArgs *argomenti = malloc(sizeof(ThreadArgs));
    snprintf(argomenti->password, sizeof(argomenti->password), "%s", password);
    snprintf(argomenti->username, sizeof(argomenti->username), "%s", username);
    pthread_create(&thread, NULL, thread_wait_password, (void *)argomenti);

    // request 5) receive multiple messages in a specified fifo, wait for messages indefinitely
    char fifo_client[STR_DEFAULT] = "∕tmp∕clients.fifo"; // hardcoded string literal, read-only
    volatile sig_atomic_t wait_for_fifo = true;
    if (mkfifo(fifo_client, S_IRUSR | S_IWUSR) == -1) // create FIFO and check errors
    {
        fprintf(stderr, "ERROR: couldn't open fifo %s\n", fifo_client);
        exit_status = 106; // custom error value, fifo client
        exit(exit_status);
    }
    int fifo_id_client = open(fifo_client, O_RDONLY); // open in read-only using file descriptors

    while (wait_for_fifo)
    {
        char read_msg_fifo[STR_DEFAULT] = {0};
        pid_t pid_received = 0;
        read(fifo_id_client, read_msg_fifo, sizeof(read_msg_fifo));
        pid_received = atoi(read_msg_fifo); // parse PID from buffer and store it
        kill(pid_received, SIGUSR1);        // send message to PID read in FIFO client
    }

    return exit_status;
}

// Section 8: function definition

void handler_SIGTSTP(int signo, siginfo_t *info, void *empty) // custom handler to handle the signal SIGTSTP
{
    // upon receiving SIGTSTP the program must terminate instead of waiting
    exit(EXIT_SUCCESS);
}

void *thread_wait_password(void *arg)
{
    ThreadArgs *params = (ThreadArgs *)arg;
    volatile sig_atomic_t wait_login = true;
    char *fifo_login = "∕tmp∕login.fifo"; // hardcoded string literal, read-only
    // FIFO is already created, open it
    FILE *open_fifo_login = fopen(fifo_login, "r");
    if (open_fifo_login == NULL)
    {
        fprintf(stderr, "ERROR: couldn't open FIFO login at %s\n", fifo_login);
        int exit_status = 107; // custom error value, fifo login
        exit(exit_status);
    }

    while (wait_login)
    {
        char buffer[STR_DEFAULT] = {0};
        fgets(buffer, sizeof(buffer), open_fifo_login);
        if (strcmp(buffer, params->password) == 0) // if the password read in the file and the one read in the fifo match
        {
            printf("OK\n");
            fprintf(stderr, "%s logged in\n", params->username);
        }
        else
        {
            printf("NO\n");
        }
    }
}