files := kiaro file2
some_file: $(files) # pre-requisites (dependencies), is the above variable!
	echo "Look at this variable: " $(files)
	touch some_file

kiaro:
	touch ciao

file2:
	touch bello

clean:
	rm -f ciao bello some_file