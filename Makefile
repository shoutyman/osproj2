mod-v6: mod-v6.c structures.h
	g++ mod-v6.c structures.h -std=c++11
	
mod-v6.o: mod-v6.c

.PHONY: all clean

all: mod-v6

clean:
	rm -f mod-v6 my_v6