CFLAGS= -g -O3 -W -Wall -Wunused -Wcast-align -pedantic -pedantic-errors -fstack-protector-all -Wfloat-equal -Wpointer-arith
CC = g++ -std=c++17 $(CFLAGS)


all: brandes renumerate reduce backnumerate errCalc

brandes: build/main.o build/FastGraph.o build/utils.o build/ProgressBar.o
	$(CC) -o brandes build/main.o build/FastGraph.o build/utils.o build/ProgressBar.o -lpthread

build/main.o: src/main.cpp
	$(CC) -c src/main.cpp -o build/main.o

build/FastGraph.o: src/FastGraph.h src/FastGraph.cpp
	$(CC) -c src/FastGraph.cpp -o build/FastGraph.o

build/utils.o: src/utils.h src/utils.cpp
	$(CC) -c src/utils.cpp -o build/utils.o

build/ProgressBar.o: src/ProgressBar.h src/ProgressBar.cpp
	$(CC) -c src/ProgressBar.cpp -o build/ProgressBar.o

renumerate: build/renumerate.o
	$(CC) -o renumerate build/renumerate.o

build/renumerate.o: src/renumerate.cpp
	$(CC) -c src/renumerate.cpp -o build/renumerate.o

reduce: build/reduce.o
	$(CC) -o reduce build/reduce.o

build/reduce.o: src/reduce.cpp
	$(CC) -c src/reduce.cpp -o build/reduce.o

backnumerate: build/backnumerate.o
	$(CC) -o backnumerate build/backnumerate.o

build/backnumerate.o: src/backnumerate.cpp
	$(CC) -c src/backnumerate.cpp -o build/backnumerate.o

errCalc: build/errCalc.o
	$(CC) -o errCalc build/errCalc.o

build/errCalc.o: src/errCalc.cpp
	$(CC) -c src/errCalc.cpp -o build/errCalc.o

clean:
	rm -f brandes renumerate reduce backnumerate errCalc
	cd build; rm -f *.o
	find * -name \*~ -delete

