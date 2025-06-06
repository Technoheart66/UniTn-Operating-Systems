a := tutte le variabili dei makefile sono stringhe!\n
b := one two\n# a is set to the string "one two"
c := 'one two senza a capo' # Not recommended. b is set to the string "'one two'"
d := a capo non nella variabile ma in printf
all:
	printf '$a' #we need quotes '' in printf
	printf '$b'
	printf $c
	printf '$d\n'