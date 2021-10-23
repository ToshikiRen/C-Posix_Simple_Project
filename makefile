CC = gcc
FLAGS = -pthread -lrt

build:
	$(CC) $(FLAGS) $(file).c -o executabil_tema

run:
	./executabil_tema

clean:
	rm -rf executabil_tema