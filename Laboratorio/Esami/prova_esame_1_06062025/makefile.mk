# Creare un makefile che compili il programma solamente se il file passato da terminale 
# FILE=<path∕to∕File.txt> esiste. Se non esiste deve essere restituito un messaggio di errore.
# Qualora il file dovesse esistere, il makefile deve creare una nuova cartella (nella
# directory in cui viene eseguito) e copiarvi dentro il file. Dopodichè, dovrà compilare i sorgenti
# creando un eseguibile sempre all’interno della nuova cartella. Il nome dell’eseguibile e della
# cartella sono dati da terminale con altre 2 variabili: DIR=<path> EXE=<name>.

SHELL:=/bin/bash
FILE?=
DIR?=
EXE?=
SORGENTI:=$(wildcard *.c)
OUT=${DIR}/${EXE}.out# = evaluates recursively only when variable is used

all: check_vars operations ${OUT}
	@echo "all done"

operations:
	mkdir -p "${DIR}"
	cp "${FILE}" "${DIR}/${FILE}"

${OUT}:
	@echo "compiling"
	gcc ${SORGENTI} -o "${OUT}"

check_vars:
	@if [[ -z "${FILE}" ]]; then echo "param FILE is empty"; exit 1; else echo "FILE: ${FILE}"; fi
	@if [[ -z "${DIR}" ]]; then echo "param DIR is empty"; exit 1; else echo "DIR: ${DIR}"; fi
	@if [[ -z "${EXE}" ]]; then echo "param EXE is empty"; exit 1; else echo "EXE: ${EXE}"; fi

