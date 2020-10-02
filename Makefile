CFLAGS= -g -O3 -Wall
CC = g++ -std=c++17 $(CFLAGS)


all: brandes renumerate

brandes: build/main.o build/Graph.o build/FastGraph.o build/utils.o build/ProgressBar.o
	$(CC) -o brandes build/main.o build/Graph.o build/FastGraph.o build/utils.o build/ProgressBar.o -lpthread

build/main.o: src/main.cpp
	$(CC) -c src/main.cpp -o build/main.o

build/Graph.o: src/Graph.h src/Graph.cpp
	$(CC) -c src/Graph.cpp -o build/Graph.o -lpthread

build/FastGraph.o: src/FastGraph.h src/FastGraph.cpp
	$(CC) -c src/FastGraph.cpp -o build/FastGraph.o -lpthread

build/utils.o: src/utils.h src/utils.cpp
	$(CC) -c src/utils.cpp -o build/utils.o

build/ProgressBar.o: src/ProgressBar.h src/ProgressBar.cpp
	$(CC) -c src/ProgressBar.cpp -o build/ProgressBar.o -lpthread

renumerate: build/renumerate.o
	$(CC) -o renumerate build/renumerate.o

build/renumerate.o: src/renumerate.cpp
	$(CC) -c src/renumerate.cpp -o build/renumerate.o

clean:
	rm -f brandes renumerate
	cd build; rm -f *.o
	find * -name \*~ -delete

