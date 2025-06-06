all: clean one two three four five

# NOTA BENE. metti clean all'inizio altrimenti make vedrà i file da prime e POI gli eliminerà
# questo comporta che echo in four five non scriva in output echo four four echo five five

one:
	touch one
two:
	touch two
three:
	touch three
	echo $@
four five:
	echo $@
# $@ is an automatic variable that contains the target name

clean:
	rm -f one two three four five