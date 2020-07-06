CFLAGS= -g -O3 -Wall
CC = g++ -std=c++17 $(CFLAGS)


all: brandes

brandes: build/main.o build/Graph.o build/utils.o
	$(CC) -o brandes build/main.o build/Graph.o build/utils.o -lpthread

build/main.o: src/main.cpp
	$(CC) -c src/main.cpp -o build/main.o

build/Graph.o: src/Graph.h src/Graph.cpp
	$(CC) -c src/Graph.cpp -o build/Graph.o -lpthread

build/utils.o: src/utils.h src/utils.cpp
	$(CC) -c src/utils.cpp -o build/utils.o

clean:
	rm -f brandes
	cd build; rm -f *.o
	find * -name \*~ -delete

