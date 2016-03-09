CC = g++

CFLAGS = -W -Wall -O4
LFLAGS = 

PROG = gt
SOURCE = gt.cpp

build: $(PROG)
	@echo Done
	
$(PROG): $(SOURCE)
	$(CC) -o $(PROG) $(SOURCE) $(CFLAGS) $(LFLAGS)

clean:
	rm -rf *.o $(PROG)
