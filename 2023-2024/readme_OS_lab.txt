

   _____ _     _                 _                              _   _       _   _       _                     _             _       
  / ____(_)   | |               (_)                            | | (_)     (_) | |     | |                   | |           (_)      
 | (___  _ ___| |_ ___ _ __ ___  _    ___  _ __   ___ _ __ __ _| |_ ___   ___  | | __ _| |__   ___  _ __ __ _| |_ ___  _ __ _  ___  
  \___ \| / __| __/ _ \ '_ ` _ \| |  / _ \| '_ \ / _ \ '__/ _` | __| \ \ / / | | |/ _` | '_ \ / _ \| '__/ _` | __/ _ \| '__| |/ _ \ 
  ____) | \__ \ ||  __/ | | | | | | | (_) | |_) |  __/ | | (_| | |_| |\ V /| | | | (_| | |_) | (_) | | | (_| | || (_) | |  | | (_) |
 |_____/|_|___/\__\___|_| |_| |_|_|  \___/| .__/ \___|_|  \__,_|\__|_| \_/ |_| |_|\__,_|_.__/ \___/|_|  \__,_|\__\___/|_|  |_|\___/ 
                                          | |                                                                                       
                                          |_|                                                                                       



Text art generator: Font: big, https://patorjk.com/software/taag/#p=testall&f=miniwi&t=gene%40home

====================================================================
 BASH
====================================================================

https://devhints.io/bash
bash cheatsheet carina

https://mywiki.wooledge.org/BashFAQ/031
tabella operatori molto bella sotto spiegazione conditional expressions

//Run file

bash ./example.sh

or we tell te os that the file is executable

chmod +x example.sh		+x means executable
./example.sh

//Variables

DATA="ciao"
echo $DATA

DATA=$(pwd)  assigns the value of the command to DATA
echo $DATA

//Array

Definizione: lista=("a" 1 "b" 2 "c" 3) separati da spazi!
● Output completo: ${lista[@]}
● Accesso singolo: ${lista[x]} (0-based)
● Lista indici: ${!lista[@]}
● Dimensione: ${#lista[@]}
● Set elemento: lista[x]=value
● Append: lista+=(value)
● Sub array: ${lista[@]:s:n} (from index s, length n)

//Subshell
It is possible to launch a subshell, that is a sub-environment, with (...commands...)
and we can capture its output with $(...commands...) i.e. ls $(cat /tmp/tmp.txt)



//Conditional expressions
since many operators are used to express different meanings (i.e. > means greater than but also output redirection) we need to encapsulate expressions

https://www.gnu.org/software/bash/manual/html_node/Bash-Conditional-Expressions.html
GNU official reference of conditional expressions

https://mywiki.wooledge.org/BashFAQ/031
Ottima spiegazione della differenza tra:
● open square bracket command --> [ ]			this is the same as test	
● test command --> test condition				this is the same as [ ]
● bash test construct --> [[ ]]


https://linuxsimply.com/bash-scripting-tutorial/conditional-statements/if-else/if-else-one-line/#:~:text=To%20express%20one%2Dline%20if,code%20block%20will%20be%20executed
Esempi carini di if-then-else con vari operatori
if-then-else: 
deve iniziare con if e terminare con fi
ricordati che dopo ogni clausala devi mettere ; 

Some examples

if test 1 -gt 2; 
then echo "bella"; 
else "cacca"; 
fi

Per delle soluzioni molto più compatte leggere le slide
1. Stampa “T” (per True) o “F” (per False) a seconda che il valore rappresenti un file o una cartella esistente
if [[ -e $DATA ]]; then echo "ciao"; else echo "bello"; fi

2. Stampa “file”, “cartella” o “?” a seconda che il valore rappresenti un file (esistente), una cartella (esistente) o una voce non presente nel file-system
if [[ -f $DATA ]]; then echo "file"; else if [[ -d $DATA ]]; then echo "directory"; else echo "?"; fi fi

3. Stampa il risultato di una semplice operazione aritmetica (es: ‘1 < 2’) contenuta nel file indicato dal valore di DATA, oppure “?” se il file non esiste
if [[ -f $DATA ]]; then echo $(( $(cat $DATA) )); else echo "?"; fi

//chmod +x

This command makes a file executable, we can have the same result by right clicking on the file Properties -> Permissions -> Allow executing file as program
We just need to execute this command once per file

chmod +x filename #if the file is in the same directory
chmod +x ./filename #if the file isn't in the same directory you must specify its path

useful practices: 
chmod +x ./filename && ./filename #this makes it executable and then runs it
chmod +x ./bashpid.sh ; echo $BASHPID ; ./bashpid.sh  #with ';' we can run multiple commands per line, i.e. the different PIDs for each shell and subshell in filename.sh
SCRIPT “bashpid.sh”:
# bashpid.sh
echo $BASHPID #the id of the current bash process
echo $( echo $BASHPID)

//Negli script: elementi particolari e altri costrutti

● $$ : contiene il PID del processo attuale (*)
● $? : contiene il codice di ritorno dell’ultimo comando eseguito
● (( a > 2 )) this is an aritmetic expansions i.e. (( a++ )) (( a = 3<10?1:0 )) and capture the output b=$((c+a))

echo $$, echo $?

Le righe vuote e i commenti (#) sono ignorati
La prima riga può essere un metacommento (detto hash-bang, she-bang e altri nomi simili): #!application [opts]
che identifica un’applicazione cui passare il file stesso come argomento (tipicamente usato per identificare l’interprete da utilizzare)
Sono disponibili variabili speciali in particolare
● $@ : lista completa degli argomenti passati allo script
● $# : numero di argomenti passati allo script
● $0, $1, $2, … : n-th argomento

For loop:
for i in ${!lista[@]}; do
echo ${lista[$i]}
done

While loop:
while [[ $i < 10 ]]; do
echo $i ; (( i++ ))
done

If condition:
if [ $1 -lt 10 ]; then
echo less than 10
elif [ $1 -gt 20 ]; then
echo greater than 20
else
echo between 10 and 20
fi

//Negli script: argomenti e funzioni

SCRIPT “args.sh”:
#!/usr/bin/env bash
nargs=$#
while [[ $1 != "" ]]; do
echo "ARG=$1"
shift #shift all argurments to the left, $3->$2 $2->$1 etc.
done

CLI:
chmod +x ./args.sh	
./args.sh uno
./args.sh uno due tre


esempi
Scrivere uno script che dato un qualunque numero di argomenti li restituisca in
output in ordine inverso

special case of $@ and $*, must be quoted, check the bash cheatsheet

./inverseArgs.sh
nArgs=$#

while [[ $nArgs > 0 ]]; do
echo ${lista[$nArgs]}; (( i-- )) done


====================================================================
 DOCKER
====================================================================

Tecnologia di virtualizzazione a livello SO che consente la creazione, gestione e esecuzione di applicazioni attraverso containers.
I containers sono ambienti leggeri, dinamici ed isolati che vengono eseguiti sopra il kernel di Linux. Essi sono gestiti da Docker.

Un’immagine docker è un insieme di “istruzioni” per la creazione di un container. I container devono essere basato su una immagine.
Essa consente di raggruppare varie applicazioni ed eseguirle, con una certa configurazione, in maniera rapida attraverso un container


====================================================================
 C FOUNDAMENTALS
====================================================================

C è un linguaggio debolmente tippizzato che usa 8 tipi:
● void (0 byte)
● char (1 byte)
● short (2 bytes)
● int (4 bytes)
● float (4 bytes)
● long (8 bytes)
● double (8 bytes)
● long double (8 bytes)

Esiste la librea stint.h che definisce dei tipi standard dalla dimensione più esplicita:
	● int8_t
	● int16_t
	● int32_t
	● int64_t
	● uint8_t
	● uint16_t
	● uint32_t
	● uint64_t
	Questi sono i più utilizzati ma ne definisce molti di più

Operatore sizeof()

Alcune librerie standard:
	● stdio.h: FILE, EOF, stderr, stdin, stdout, fclose(), etc...
	● stdlib.h: atof(), atoi(), malloc(), free(), exit(), system(), rand(), etc...
	● string.h: memset(), memcpy(), strncat(), strcmp(), strlen(), etc...
	● math.h: sin(), cos(), sqrt(), floor(), etc...
	● unistd.h: STDOUT_FILENO, read(), write(), fork(), pipe(), etc...
	● fcntl.h: creat(), open(), etc...
	● signal.h: sigset_t datatype, sigfillset(), sigdelset(), etc... https://stackoverflow.com/questions/71043304/vscode-report-undefined-symbol-which-has-been-actually-included

Flags per open(), mkfifo():
	● O_RDONLY, O_WRONLY, O_RDWR: almeno uno è obbligatorio.
	● O_CREAT: crea il file se non esiste (con O_EXCL la chiamata fallisce se esiste)
	● O_APPEND: apre il file in append-mode (auto lseek con ogni scrittura)
	● O_TRUNC: cancella il contenuto del file (se usato con la modalità scrittura)
  
Permission bits: https://man7.org/linux/man-pages/man2/chmod.2.html
	mode: interi (ORed) per i privilegi da assegnare al nuovo file: S_IRUSR, S_IWUSR, S_IXUSR, S_IRWXU, S_IRGRP, …, S_IROTH
	Macro		Octal	Meaning
	S_IRUSR		0400	Read permission for owner (r--)
	S_IWUSR		0200	Write permission for owner (-w-)
	S_IXUSR		0100	Execute permission for owner (--x) — ignored for FIFO
	S_IRGRP		0040	Read for group
	S_IWGRP		0020	Write for group
	S_IXGRP		0010	Execute for group
	S_IROTH		0004	Read for others
	S_IWOTH		0002	Write for others
	S_IXOTH		0001	Execute for others

	in example: mkfifo("myfifo", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		This means: owner can read & write, group can read & write, others can read & write
		
====================================================================
 CONTINUA, il resto in C:\Users\franc\Desktop\Francesco\University\Semestre 4\Crispo Bruno, Sistemi Operativi\Laboratorio
====================================================================
