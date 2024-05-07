CFLAGS = -std=c99 -pedantic -Wall -Wextra -O3
DBGFLAGS = -std=c99 -pedantic -Wall -Wextra -g3 -DDEBUG

CC = cc

++ = g++ -std=c++11

all: cleancube cube.o

cleancube:
	rm -rf cube.o

cube.o:
	${CC} ${CFLAGS} -c -o cube.o cube.c

cleandebug:
	rm -rf debugcube.o

debugcube.o:
	${CC} ${DBGFLAGS} -c -o debugcube.o cube.c

clean: cleantest
	rm -rf *.o

test: cleandebug debugcube.o
	CUBETYPE=${CUBETYPE} TEST=${TEST} ./test/test.sh

benchmark: cube.o
	CUBETYPE=${CUBETYPE} ./benchmark/bench.sh

.PHONY: all clean cleancube cleandebug test benchmark

ctest: cleanctest cube.o
	${CC} ${CFLAGS} -o ctest ctest.c cube.o

cpptest: cleancpptest cube.o
	${++}  -o cpptest ctest.cpp cube.o

cleanctest:
	rm -rf ctest

cleancpptest:
	rm -rf cpptest

cleantest: cleanctest cleancpptest


