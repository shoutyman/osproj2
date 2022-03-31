mod-v6: mod-v6.c structures.h
	g++ -o mod-v6 mod-v6.c structures.h
	
mod-v6.o: mod-v6.c

.PHONY: all clean

all: mod-v6

clean:
	rm -f mod-v6 my_v6