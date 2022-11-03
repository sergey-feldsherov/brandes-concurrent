CFLAGS= -g -O3 -W -Wall -Wunused -Wcast-align -pedantic -pedantic-errors -fstack-protector-all -Wfloat-equal -Wpointer-arith
BCFLAGS=-W -Wall -Wunused -Wcast-align -Werror -pedantic -pedantic-errors -fstack-protector-all -Wfloat-equal -Wpointer-arith -Wwrite-strings -Wcast-align -Wno-format -Wno-long-long -std=gnu99 -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wbad-function-cast -Wnested-externs # -Wdeclaration-after-statement
CC = gcc $(BCFLAGS)
#CPPC = g++ -std=c++17 $(CFLAGS)


#all: brandes renumerate reduce backnumerate errCalc

#brandes: build/main.o build/FastGraph.o build/utils.o build/ProgressBar.o
#	$(CPPC) -o brandes build/main.o build/FastGraph.o build/utils.o build/ProgressBar.o -lpthread

#build/main.o: src/main.cpp src/timing.h src/FastGraph.h src/utils.h
#	$(CPPC) -c src/main.cpp -o build/main.o

#build/FastGraph.o: src/FastGraph.h src/FastGraph.cpp src/ProgressBar.h
#	$(CPPC) -c src/FastGraph.cpp -o build/FastGraph.o

#build/utils.o: src/utils.h src/utils.cpp
#	$(CPPC) -c src/utils.cpp -o build/utils.o

#build/ProgressBar.o: src/ProgressBar.h src/ProgressBar.cpp src/timing.h
#	$(CPPC) -c src/ProgressBar.cpp -o build/ProgressBar.o

renumerate: build/renumerate.o build/timing.o
	$(CC) -o renumerate build/renumerate.o build/timing.o

build/renumerate.o: src/renumerate.c src/timing.h
	$(CC) -o build/renumerate.o -c src/renumerate.c

build/timing.o: src/timing.h src/timing.c
	$(CC) -o build/timing.o -c src/timing.c 

#reduce: build/reduce.o
#	$(CPPC) -o reduce build/reduce.o

#build/reduce.o: src/reduce.cpp
#	$(CPPC) -c src/reduce.cpp -o build/reduce.o

#backnumerate: build/backnumerate.o
#	$(CPPC) -o backnumerate build/backnumerate.o

#build/backnumerate.o: src/backnumerate.cpp
#	$(CPPC) -c src/backnumerate.cpp -o build/backnumerate.o

#errCalc: build/errCalc.o
#	$(CPPC) -o errCalc build/errCalc.o

#build/errCalc.o: src/errCalc.cpp
#	$(CPPC) -c src/errCalc.cpp -o build/errCalc.o

clean:
#	rm -f brandes renumerate reduce backnumerate errCalc
	rm -f renumerate
	cd build; rm -f *.o
	find * -name \*~ -delete

