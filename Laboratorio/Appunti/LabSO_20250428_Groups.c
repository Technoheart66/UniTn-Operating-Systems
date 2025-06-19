// Lezione di laboratorio del 24/03/2025

// Titolo: System Calls

/*
Recap precedente

Ricordiamo che è da due settimane che non c'è lezione

Abbiamo visto lo spettro dei 32 segnali standard, cui possiamo definire funzioni custom
Abbiamo visto i segnali real-time min e max, sarebbero da utilizzare con il payload
Payload limitato, segnali real-time gestiti in modo diverso:
  con segnali normali solo l'ultimo segnale viene gestito
  con segnali real-time se arrivano 10 segnali di fila essi vengono inseriti in una coda e gestiti a turno
*/

/*
Errori in C

Sarà capitato di imbattersi in errori che terminano l'esecuzione del programma (ad es. segmentation fault)
Oltre a questi errori ci sono errori diversi, non fatali, come ad esempio la lettura errata di un file
In alcuni casi verrà restituito il puntatore null se l'esito è negativo, ma il programma non termina
Perciò per controllare se tutto è andato bene dobbiamo controllare se il puntatore di ritorno è corretto
Per fare ciò, utilizziamo la variabile 'errno', ma dobbiamo ricordare di controllarla dopo ogni chiamata
  poiché la variabile verrà sovvrascritta dall'esito di una chiamata successiva
Possiaom anche convertire il codice d'errore in una stringa compresibile tramite '*strerror(int errnum)'
Esiste anche la funzione 'void perror(const char *str)' che stamperà il codice standard ed in aggiunta una
  stringa personalizzata definita da noi *esempio codice*
*/

/*
Esempio: errore apertura file
*/

#include <stdio.h> <errno.h> <string.h> //errFile.c

extern int errno; // declare external global variable
int main(void)
{
  FILE *pf;
  pf = fopen("nonExistingFile.boh", "rb"); // Try to open file
  if (pf == NULL)
  { // something went wrong!
    fprintf(stderr, "errno = %d\n", errno);
    perror("Error printed by perror");
    fprintf(stderr, "Strerror: %s\n", strerror(errno));
  }
  else
  {
    fclose(pf);
  }
}

/*
Esempio: errore processo non esistente
*/

#include <stdio.h> <errno.h> <string.h> <signal.h> //errSig.c
extern int errno;                                  // declare external global variable
int main(void)
{
  int SIGUSR1;
  int sys = kill(3443, SIGUSR1); // Send signal to non existing proc
  if (sys == -1)
  { // something went wrong!
    fprintf(stderr, "errno = %d\n", errno);
    perror("Error printed by perror");
    fprintf(stderr, "Strerror: %s\n", strerror(errno));
  }
  else
  {
    printf("Signal sent\n");
  }
}

/*
Process groups

I processi figli ereditano il gruppo del padre, tuttavia se il padre cambia gruppo i figli non lo faranno
La gerarchia padre-figlio permette al padre di cambiare gruppo al figlio specificandolo nella funzione
*/