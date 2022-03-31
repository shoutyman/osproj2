mod-v6: mod-v6.c structures.h
	g++ -o mod-v6 mod-v6.c
	

all: mod-v6

clean:
	rm -f mod-v6 my_v6