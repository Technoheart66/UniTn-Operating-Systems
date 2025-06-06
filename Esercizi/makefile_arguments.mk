# è possibile passare variabili come argomento al makefile in questo modo: make FILE=path/to/File.txt
# bisgna usare o almeno dichiarare quei nomi all'interno del makefile
# operatore ?= consente di impostare una variabile solo se non è già stata definita

FILE ?= pan

all:
	@echo "prova a passarmi l'argomento FILE=qualcosa"
	@echo $(FILE)tonde
	@echo ${FILE}grafe