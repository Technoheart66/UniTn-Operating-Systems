// Lezione di laboratorio del 19/05/2025

// Threads

/*
Le code di messaggi sono l'ultima tecnica di IPC che vedremo. Molto potenti ed è un concetto semplice.
I thread invece sono un concetto simile a fork().
I thread sono flussi di esecuzione all'interno di un processo.
Tuttavia, rispetto a fork() che clona e genera figli indipendenti, i thread non sono indipendenti.
Il codice si sdoppia logicamente ma esso non è indipendente.

A volte, come ben sappiamo, è necessario eseguire operazioni bloccanti e mentre le eseguiamo può succedere-
  che vogliamo eseguire altre operazioni. Per fare ciò in alcuni scenari è possibile gestire il flusso-
  con fork() e i figli ma ciò non è adatto a tutti gli scenari.
I thread consentono di risolvere questi scenari permettendo di eseguire codice in parallelo.
Ogni thread è identificato da un ID e può essere gestito 'come un processo figlio'.

Attenzione al puntatore void.

I thread vengono creati e distrutti.
La creazione è di predefinita impostata per creare thread 'joinable' ossia li crea in uno stato tale-
  da permettere ad altri thread di aspettare e catturare il valoro di ritorno e le variabili.
Una volta creato, o direttamente alla creazione, è possibile impostare lo stato del thread in 'detached'.
Attenzione che l'inverso non è possibile, ossia da 'detached' non si può passare a 'joinable'.
I thread hanno molti attributi:
  - init (inizializza tutte le variabili al valore di default)
  - destroy
  - set
  - get
  - detachstate
  - etc.


// Thread e segnali

Quando si invia un segnale a un processo non è possibile sapere a prescindere quale thread andrà a gestirlo.
Perciò è importante impostare una maschera dei segnali correttamente a ciascun thread tramite-
  'pthread_attr_setsigmask_np()' tuttavia non è standard.
In alternativa è possibile usare la funzione standard-
  'pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict oset)'


// Mutex e il problema della sincronizzazione

Quando eseguiamo un programma con più thread essi condividono alcune risorse,-
  tra le quali le variabili globali.
Se entrambi i thread accedono ad una sezione di codice condivisa ed hanno la necessità-
  di accedervi in maniera esclusiva allora dobbiamoinstaurare una sincronizzazione.
I risultati, altrimenti, potrebbero essere inaspettati


*/