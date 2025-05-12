// Lezione di laboratorio del 12/05/2025

// Code di messaggi o Message queues

/*
Tendenzialmente alle code viene assegnato come ID un intero positivo incrementale gestisto dal S.O.
Due processi per poter comunicare attraverso le code devono avere accesso al medesimo ID della coda.
Una volta creata la coda vi si può accedere in modalità asincrona, qundi niente meccanismi di-
  sincronizzazione come open, close, etc. di fifo e pipe.
Se leggiamo ma la coda è vuota allora si rimane in attesa, ma ciò è il comportamente voluto ed è-
  generalmente lo standard.

Come identifichiamo la tra processi diversi?
Attraverso il suo ID chiamato Queue Identifier, ma non solo.
Infatti oltre al suo queue ID è presente un altro parametro Key.

Come creare la coda? Sarà il kernel a conferirci la coda restituendo queue ID e key ed è il kernel a gestirla
int msgget(key_t key, int msgflg)

Come argomenti accetta
  ● la chiave key, di tipo key_t che si traduce in long
  ● un flag tra
    A) IPC_CREAT: crea una coda se non esiste già, altrimenti restituisce l’identificativo di quella già esistente;
    B) IPC_EXCL: (da usare con il precedente) fallisce se coda già esistente;
    C) 0xxx: permessi per accedere alla coda, analogo a quello che si può usare nel file system. In alternativa si possono usare S_IRUSR etc.
*/
// msgget.c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

int coda()
{
  key_t queueKey = 56; // Unique key
  int queueId = msgget(queueKey, 0666 | IPC_CREAT | IPC_EXCL);
  return queueId;
}

/*
Spiegazione codice sopra

Il kernel non sa quale coda cercare in specifico se gli passiamo i flag, per capire ciò utilizza la chiave key
  e controllerà l'associazione tra key e queue ID nell sua tabella.

Ottenere chiave univoca
key_t ftok(const char *path, int id)

La variabile precedente key_t key delle code in realtà vengono create tramite questa system call.
Questa funzione crea una chiave univoca, perciò garantisce che la chiave sia sempre diversa.
La sua utilità è che come chiave possiamo utilizzare il riferimento di un file specifico.

Quello che succede è che abbiamo più processi, stesso file e stesso numero, poi la tupla (coppia)-
  genererà una chiave che verrà poi usata nella mssget dal kernel.
Perciò tutti i programmi che avranno accesso alla chiave key_t key generata potranno accedere alla coda.
*/

// esempio: ftok.c

// esempio creazione: ipcCreation.c

#include <sys/types.h> //ipcCreation.c
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <fcntl.h>
void creazione()
{
  remove("/tmp/unique");                                    // Remove file
  key_t queue1Key = ftok("/tmp/unique", 1);                 // Get unique key → fail
  creat("/tmp/unique", 0666);                               // Create file
  queue1Key = ftok("/tmp/unique", 1);                       // Get unique key → ok
  int queueId = msgget(queue1Key, 0666 | IPC_CREAT);        // Create queue → ok
  queueId = msgget(queue1Key, 0);                           // Get queue (no perm. needed) → ok
  msgctl(queue1Key, IPC_RMID, NULL);                        // Remove non existing queue → fail
  msgctl(queueId, IPC_RMID, NULL);                          // Remove queue → ok
  queueId = msgget(queue1Key, 0666);                        // Get non existing queue → fail
  queueId = msgget(queue1Key, 0666 | IPC_CREAT);            // Create queue → ok
  queueId = msgget(queue1Key, 0666 | IPC_CREAT);            // Get queue → ok
  queueId = msgget(queue1Key, 0666 | IPC_CREAT | IPC_EXCL); /* Create
 already existing queue -> fail */
}