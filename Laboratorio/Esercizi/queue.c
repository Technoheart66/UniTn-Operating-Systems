// queue.c

// Section 0: author, copyright & licensing
// Author: Francesco Dall'Agata
// breif showcase and tutorial on how to use queues in C

// Section 1: index
// 0) author, copyright & licensing
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
#include <stdbool.h>
#include <string.h>
#include <sys/types.h> // needed for queue
#include <sys/ipc.h>   // needed for queue
#include <sys/msg.h>   // needed for queue
#include <fcntl.h>     // needed for queue

// Section 3: define

// Section 4: typedef
typedef struct book
{
    char title[50];
    char description[100];
    short chapers;
} Book;

typedef struct msg_buffer
{
    long mtype;
    Book libro;
} queue_msg;

// Section 5: global variables

// Section 6: function declaration

// Section 7: main
int main(int *argc, char *argv[])
{
    int main_result = EXIT_SUCCESS;
    key_t queue_key_hardcoded = 19; // unique key, but it is hardcoded
    char *path = argv[1];
    path = "./";
    key_t queue_key_generated = ftok(path, 1); // estituisce una chiave basandosi sul path (una cartella o un file), esistente ed accessibile nel file-system, e sull’id numerico. La chiave generata dovrebbe essere univoca e sempre la stessa per ogni coppia <path,id> in ogni istante sullo stesso sistema
    printf("How to use queues\nQueues need 2 identifiers: key and queue ID. Look in the code for details.\n");
    printf("I generated the key: %d\n", queue_key_generated);
    // int queueID = msgget(IPC_PRIVATE , 0666 | IPC_CREAT); // creates a private queue to be used inside the same process group, the IDs are always different
    int queue_id = msgget(queue_key_generated, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL); // creates queue if not already existing, otherwise returns the ID of the pre-existing queue
    if (queue_id >= 0)
    {
        printf("DEBUG: success, created the queue, queue ID: %d\n", queue_id);
    }
    else
    {
        printf("DEBUG: fail, didn't create the queue. Error value: %d\n", queue_id); // could because it failed or the queue already existed
    }

    // creating the message to send in the queue
    queue_msg msg_test;
    msg_test.mtype = 20; // type 20, I could use this number to fecth only certain messages from the queue!
    snprintf(msg_test.libro.title, sizeof(msg_test.libro.title), "%s", "The Lord of The Rings");
    strcpy(msg_test.libro.description, "Fantasy, by J.R.R. Tolkien");
    msg_test.libro.chapers = 24;

    // sending the message via queue
    int queue_msg_outcome = msgsnd(queue_id, &msg_test, sizeof(msg_test), 0); // 0 = no flags
    if (queue_msg_outcome == 0)
    {
        printf("DEBUG: success, message sent\n");
    }
    else
    {
        printf("DEBUG: fail, message not sent. Error value: %d\n", queue_msg_outcome);
    }

    // receiving the message via queue
    queue_msg msg_received;
    queue_msg_outcome = msgrcv(queue_id, &msg_received, sizeof(msg_received.libro), 20, MSG_NOERROR); // MSG_NOERROR flags truncates the message if it is longer than size but no error, if there is no MSG_NOERROR and the message is longer then the call fails but the message will be kept inside the queue
    if (queue_msg_outcome != -1)
    {
        printf("DEBUG: success, message received, number of bytes copied: %d\n", queue_msg_outcome);
    }
    else
    {
        printf("DEBUG: fail, message not received\n");
    }
    printf("Received book message:\nTitle: %s \nDescription: %s \nChapters: %d \n", msg_received.libro.title, msg_received.libro.description, msg_received.libro.chapers);

    int queue_del = msgctl(queue_id, IPC_RMID, NULL);
    if (queue_del == 0)
    {
        printf("DEBUG: success, deleted the queue\n");
    }
    else
    {
        printf("DEBUG: fail, queue not deleted. Error value: %d\n", queue_del);
    }

    return main_result;
}