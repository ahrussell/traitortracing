CC=g++

CFLAGS=-I. -I/usr/local/include -L. -L/usr/local/lib

LIBS=-lrelic -lsodium

riskytraitortracing: 
	$(CC) -std=c++14 -o riskytt relic_compat.cpp curves.cpp mbm.cpp rtt.cpp main.cpp util.cpp cli.cpp $(CFLAGS) $(LIBS) 

.PHONY: clean

clean:
	rm -f *.o riskytt
