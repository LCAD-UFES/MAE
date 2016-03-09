build: 
	gcc -g -Wall -o data_conversion main.c -lm
	
run: primeira_matriz.txt segunda_matriz.txt
	rm -rf CNAE_TAB DOCS_DB
	mkdir CNAE_TAB DOCS_DB
	./data_conversion primeira_matriz.txt segunda_matriz.txt

primeira_matriz.txt segunda_matriz.txt:
	tar xvzf database.tgz

clean: 
	rm -f data_conversion
	rm -f *.o
	rm -f *.txt
	rm -rf CNAE_TAB DOCS_DB
