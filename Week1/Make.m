CC = mpicc
CFLAGS = -Wall -O3  #common warnings and optimizations

all: prog2a prog2b  #build prog2a and prog2b

prog2a: prog2a.c
	$(CC) $(CFLAGS) -o prog2a prog2a.c #compiles prog2a

prog2b: prog2b.c
	$(CC) $(CFLAGS) -o prog2b prog2b.c #compiles prog2b 

clean:
	rm -f prog2a prog2b #removes compiled executables files 