#Questo esercizio proviene dalla slide Bash
#Scrivere delle sequenze di comandi (singola riga da eseguire tutta in blocco) che utilizzano come “input” il valore della variabile DATA per:
#1. Stampa “T” (per True) o “F” (per False) a seconda che il valore rappresenti unfile o una cartella esistente
#2. Stampa “file”, “cartella” o “?” a seconda che il valore rappresenti un file (esistente), una cartella (esistente) o una voce non presente nel file-system
#3. Stampa il risultato di una semplice operazione aritmetica (es: ‘1 < 2’) contenuta nel file indicato dal valore di DATA, oppure “?” se il file non esiste
#NB: usate le subshell se necessario!

#DATA=$(pwd)

#1
if [[ -e $DATA ]]; then echo "T"; else echo "F"; fi

#soluzione del prof 1. [ -e $DATA ] && echo "T" || echo "F"

#2
([[ -f $DATA ]] && echo "file") || ([[ -e $DATA ]] && echo "cartella") || echo "?"
#soluzione del prof 2. [ -f $DATA ] && echo "file" || ( [ -d $DATA ] && echo "cartella" || echo "?" 

#3
DATA="test.txt"
([[ -f $DATA ]] && echo $(( $(cat $DATA) )) || (echo "?"))
#soluzione del prof. 3. [ ! -f $DATA ] && echo "?" || echo $(( $(cat $DATA) ))

#notiamo come il PID sia differente siccome nel secondo echo chiamiamo una subshell
echo "current PID: $BASHPID" #the id of the current bash process
echo "subshell PID: $( echo $BASHPID)"