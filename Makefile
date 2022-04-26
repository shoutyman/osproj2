mod-v6: mod-v6.cpp structures.h
	g++ -o mod-v6 mod-v6.cpp 
	

all: mod-v6

clean:
	rm -f mod-v6 my-v6