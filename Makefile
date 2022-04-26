mod-v6: mod-v6.cpp structures.h
	g++ mod-v6.cpp -std=c++11 -o mod-v6
	

all: mod-v6

clean:
	rm -f mod-v6 my_v6