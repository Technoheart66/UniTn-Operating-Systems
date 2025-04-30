// Esercitazione Laboratorio Sistemi Operativi 28/04/2025

/*
Contatore segnali

Creare un'applicazione che effettua un conteggio dei segnali SIGUSR1 e SIGUSR2 ricevuti/gestisti
da processi esterni ("mittenti").

L'applicazione deve intercettare i due tipi di segnali indicati e tenere un conteggio distinto
in base al processo mittente.
Il numero massimo di "mittenti" gestibili deve essere impostato con un #define
  - All'avvio il programma deve mostrare almeno il proprio PID
  - Alla ricezione di un segnale SIGUSR1 o SIGUSR2 deve mostrare un feedback con almeno il codice
    del segnale e il riferimento del mittente
  - Alla ricezione di un segnale SIGINT o SIGTERM il programma deve terminare mostrando un report
    dei conteggi suddiviso per mittente (per esempio un elenco con PID mittente, numero SIGUSR1 ricevuti,
    numero SIGUSR2 ricevuti)
*/

// Tutte librerie essenziali
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

// maximum number of senders, as specified in the requests
#define MAXMITTENTI 10

// we need to keep track of who sent the signal and how many signals a specific sender has sent

/*
  int lista_mittenti[MAXMITTENTI] = {0} // define the senders list and initialize all values to 0
  this list is too simple and unusable in our situation
*/

int n_mittenti = 0;

typedef struct mittente
{
  int PID_mittente;
  short int conto_SIGUSR1;
  short int conto_SIGUSR2;
} Mittente;

Mittente lista_mittenti[MAXMITTENTI] = {0};

void gestisci_mittenti(pid_t PID_mittente, int segnale, Mittente *lista)
{
  bool is_missing = true;
  bool found = false;
  for (int i = 0; i < MAXMITTENTI && !found; i++)
  {
    printf("\n siamo a indice i:%d\n", i);
    // remember the syntax (*pippo).pluto or pippo->pluto
    // if the slot is empty OR is the list PID matches the sender PID
    if (lista[i].PID_mittente == 0 || lista[i].PID_mittente == PID_mittente) // logical OR operator, left-to-right association
    {
      found = true;                         // if we found a free slot or a correspondence then we can quit
      lista[i].PID_mittente = PID_mittente; // if it is zero then we must set it, if it's the same it will stay the same anyway
      printf("siamo dentro il primo if\n");
      is_missing = false;
      if (segnale == SIGUSR1)
      {
        printf("siamo dentro il secondo if SIGUSR1\n");

        lista[i].conto_SIGUSR1++;
      }
      else
      {
        printf("siamo dentro il terxo if SIGUSR2\n");

        lista[i].conto_SIGUSR2++;
      }
    }
  }

  if (is_missing && n_mittenti >= MAXMITTENTI)
  {
    n_mittenti++;
    printf("Maximum number of senders exceeded\n");
    printf("There have been %d senders, but only %d were available\n", n_mittenti, MAXMITTENTI);
    printf("The signal will not be recorded, please terminate and visualize the recap\n");
  }
};

/*
Below is the simple handler that does not save other information such as:
  - PID of the process who sent the signal
  - reason why of the signal
  - may others, search on-line https://pubs.opengroup.org/onlinepubs/7908799/xsh/sigaction.html#:~:text=If%20the%20SA_SIGINFO%20flag%20(see,specifies%20a%20signal%2Dcatching%20function.
void handler_mittenti(int signal_received)
{
  if (signal_received == SIGUSR1){}
}
*/

// This is the complete handler that retains all the info we need
void handler_mittenti(int signo, siginfo_t *info, void *empty)
{
  printf("\n- - - - -\t [%d]: Signal received \t- - - - -\n", getpid());
  printf("Signal sent by: %d\n", info->si_pid);
  printf("Signal code: %d\n", signo);
  switch (signo)
  {
  case 10:
    printf("Signal translation: SIGUSR1\n");
    break;

  case 12:
    printf("Signal translation: SIGUSR2\n");
    break;

  default:
    printf("Signal translation: not yet translated\n"); // Message in case we put this handler to a wrong signal
    break;
  }
  gestisci_mittenti(info->si_pid, signo, lista_mittenti);
}

void handler_terminazione(int signo, siginfo_t *info, void *empty)
{
  printf("\n- - - - -\t [%d]: Signal received \t- - - - -\n", getpid());
  printf("Signal sent by: [%d]\n", info->si_pid);
  printf("Signal code: %d\n", signo);
  switch (signo)
  {
  case 2:
    printf("Signal translation: SIGINT\n");
    break;

  case 15:
    printf("Signal translation: SIGTERM\n");
    break;

  default:
    printf("Signal translation: not yet translated\n"); // Message in case we put this handler to a wrong signal
    break;
  }
  // Recap terminazione
  printf("\nTerminating\nRecap:\n");
  for (int i = 0; i < MAXMITTENTI; i++)
  {
    printf("PID: [%d]\n", lista_mittenti[i].PID_mittente);
    printf("\tSIGUSR1 sent: %d\n", lista_mittenti[i].conto_SIGUSR1);
    printf("\tSIGUSR2 sent: %d\n", lista_mittenti[i].conto_SIGUSR2);
  }
  exit(EXIT_SUCCESS);
}

int main(void)
{

  // printf("lista mittenti[1] PID: %d\n", lista_mittenti[1].PID_mittente);
  // printf("lista mittenti[1] conto: %d\n", lista_mittenti[1].conto_SIGUSR1);

  printf("\nSTART\n");
  printf("This is the main process, PID: [%d] \n", getpid());
  printf("To use: send signals SIGUSR1 and SIGUSR2\n");
  printf("To terminate: send signals SIGINT or SIGTERM\n");

  struct sigaction sa_mittenti;
  /*
  sa_mittenti.sa_handler = handler_mittenti; // my custom handler defined above

  Since we need more info we have to use:
    x.sa_sigaction = handler;
    x.sa_flags = SA_SIGINFO;

  The difference is that sa_handler does not save many informations such as the sender info and the reason why of the signal
  */
  sa_mittenti.sa_sigaction = handler_mittenti;
  sa_mittenti.sa_flags = SA_SIGINFO; // Use sa_sigaction
  sigemptyset(&sa_mittenti.sa_mask); // using 'sigemptyset' let's use an empty mask to block no signal
  // let's customize the handler of our signals of interest: SIGUSR1, SIGUSR2, SIGTERM, SIGINT
  sigaction(SIGUSR1, &sa_mittenti, NULL);
  sigaction(SIGUSR2, &sa_mittenti, NULL);

  struct sigaction sa_terminatione;
  sa_terminatione.sa_sigaction = handler_terminazione;
  sa_terminatione.sa_flags = SA_SIGINFO;
  sigemptyset(&sa_mittenti.sa_mask); // using 'sigemptyset' let's use an empty mask to block no signal
  sigaction(SIGTERM, &sa_terminatione, NULL);
  sigaction(SIGINT, &sa_terminatione, NULL);

  while (true)
  {
    // We want the program to never end becuase we will terminate it by sending a signal SIGTERM or SIGINT
  }

  return EXIT_SUCCESS;
}

/*
Tree Process

Creare un albero di processi dinamico, che consente di creare/eliminare processi in vari livelli dell'albero

*/