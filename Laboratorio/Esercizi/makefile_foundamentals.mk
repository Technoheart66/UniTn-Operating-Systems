files := kiaro file2
some_file: $(files) # pre-requisites (dependencies), is the above variable!
	echo "Look at this variable: " ${files}
	@echo "accedi alle variabili usando () e {}"
	touch some_file

kiaro:
	touch ciao

file2:
	touch bello
