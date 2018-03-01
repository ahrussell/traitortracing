CC=g++

CFLAGS=-I. -I./include -I/usr/local/include -L. -L/usr/local/lib

SOURCES= relic_compat.cpp curves.cpp mbm.cpp rtt.cpp main.cpp util.cpp cli.cpp

LIBS=-lrelic -lsodium

riskytraitortracing: 
	$(CC) -std=c++14 -o traitortracing $(SOURCES) $(CFLAGS) $(LIBS) 

.PHONY: clean

clean:
	rm -f *.o traitortracing
