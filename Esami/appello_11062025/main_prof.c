#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>


//Define a struct for the queue messages
typedef struct message{
  long mtype;
  char mtext[50];
} message;

void vedetta();
void arpioniere(pid_t capoGruppo);

pid_t timoniere, nave;
int arpionieri = 0;
int queuePrivata;
int mobyCheck = 0;


//This is handle the sight of moby dick
void mobydickHandler(int signum){
  if(signum == SIGRTMAX){
    //It's moby dick who has finished attacking
    //Set a flag to continue in the main
    mobyCheck = 1;
  }
  
}

//When the group leader has created the group then we move on to the creation of the rest of the arpionieri
int groupLeaderReady = 0;
void handlerGroupReady(int signo){
  groupLeaderReady =1;
}

int main(int argc, char **argv){
  
  //Check number of arguments
  if(argc!=3){
    fprintf(stderr, "Usage: %s <pid> <arpionieri>\n", argv[0]);
    return 90;
  }
  //Convert the parameters to two numbers
  timoniere = strtol(argv[1], NULL, 10);
  arpionieri = strtol(argv[2], NULL, 10);

  //Create a private queue to handle the communication between sailors
  queuePrivata = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

  //Set a signal handler to be notified when the harpooner leader is ready
  //(This is not mandatory, but is is more robust)
  struct sigaction sa;
  sa.sa_handler = handlerGroupReady;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  //Use SIGRTMIN+1 
  sigaction(SIGRTMIN+1, &sa, NULL);

  //Save all the harpooners in an array
  pid_t pidArpionieri[arpionieri];

  for(int i=0; i<arpionieri; i++){
    //Create the harpooners
    pidArpionieri[i] = fork();
    if(pidArpionieri[i] == 0){
      //Call the harpooner function passing the pid of the group leader (i.e. the group id)
      arpioniere(i==0 ? 0 : pidArpionieri[0]);
      return 0;
    }else if(i==0){
      //This is not mandatory in practice, but it ensures that the group leader has 
      //created the group before creating the other harpooners.
      while(!groupLeaderReady) sleep(1);
    }
  }
  
  nave = getpid();
  //Create the lookout process
  pid_t pidVedetta = fork();
  if(pidVedetta == 0){
    //Invoke its function
    vedetta();
    return 0;
  }

  //Set the handler for moby dick who will send us a SIGRTMAX when she's 
  //finished killing the harpooners.
  sa.sa_handler = mobydickHandler;
  sigaction(SIGRTMAX, &sa, NULL);

  while(1){
    pause();
    //Check if mobydick was sighted, i.e. if we were notified
    if(mobyCheck){
      int count = 0;
      //There are several approaches that we could follow, we go for the simplest one:
      //We check whether each process is still alive (making sure they are not zombies)

      //remove all zombie processes by waiting for them
      while(waitpid(-1, NULL, WNOHANG) > 0);

      //For each harpooner, check its status by sending a 0 signal
      for(int i = 0; i<arpionieri; i++){
        if(kill(pidArpionieri[i], 0) == -1){
          //If this is the case then this harpooner was killed
          count++;
        }
      }
      //Now we can send a signal with this count to the timoniere
      sigqueue(timoniere, SIGRTMIN, (union sigval){.sival_int = count});
    }
  }
  return 0;
}

//This handler will set a flag in the lookout process
short balenaAvvistata = 0;
void vedettaHandler(int signum){
  balenaAvvistata= 1;
}

void vedetta(){
  //First we notify the timoniere that we are ready
  kill(timoniere, SIGUSR1);

  //We set up the handler for the signal SIGUSR2
  struct sigaction sa;
  sa.sa_handler = vedettaHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGUSR2, &sa, NULL);

  //We open the queue to observe the sea!
  key_t key = ftok("/tmp/mare", timoniere);
  int coda = msgget(key,0);

  message msg;

  while(1){
    //We wait for SIGUSR2 signals
    pause();
    if(balenaAvvistata){
      balenaAvvistata = 0;
      //Shout out that we have seen a whale!
      printf("Balena!\n");
      //Listen for any message in the queue
      int rcv = msgrcv(coda, &msg, sizeof(msg.mtext), 0, 0);
      if(rcv == -1){
        fprintf(stderr, "Errore nella ricezione del messaggio\n");
      }else{
        printf("Avvistata una %s a %ld metri\n", msg.mtext, msg.mtype);
        //Check if we have seen moby dick
        if(strcmp(msg.mtext, "mobydick") == 0){
          //Then we simply create a new process to host the captain
          if(fork()==0){
            char buf[10];
            snprintf(buf,10,"%d",timoniere);
            //We invoke the binary passing its name and the pid of the timoniere
            execl("./ahab.out","./ahab.out",buf,NULL);
          }
        }
        //We notify all the harpooners about the sighting using the private queue. We simply
        //send a message to each of them. If we send enough messages, then we can be sure that
        //all harpooners will receive one.
        for(int i=0; i<arpionieri; i++){
          msgsnd(queuePrivata, &msg, sizeof(msg.mtext), 0);
        }
      }
    }
  }
}

void arpioniere(pid_t capoGruppo){
  if(capoGruppo == 0){
    //Create a new group and become a group leader
    capoGruppo = setpgid(0,0);
    //Notify the ship that the group leader is ready. This is not mandatory, but it is more robust.
    kill(getppid(),SIGRTMIN+1);
  }else{
    //We simply join the group of the group leader, which was passed as an argument
    setpgid(0, capoGruppo);
  }

  //Create a file to register the harpooner
  char nomeFile[50];
  snprintf(nomeFile,50, "/tmp/registrazione/%d", getpid());
  int fd = open(nomeFile, O_WRONLY | O_CREAT, 0666);
  close(fd);

  message msg;
  
  
  while(1){
    //Receive the message from the private queue
    msgrcv(queuePrivata, &msg, sizeof(msg.mtext), 0, 0);
    char nomeFifo[50];
    //Create name for the FIFO
    snprintf(nomeFifo, 50,"/tmp/%d-%ld", getpid(),msg.mtype);
    //Create FIFO
    mkfifo(nomeFifo, 0666);
    //Open the FIFO --> blocking until someone reads it
    int fd = open(nomeFifo, O_WRONLY);

    //Write the message to the FIFO
    int written = write(fd, msg.mtext, strlen(msg.mtext));
    close(fd);
  }
}