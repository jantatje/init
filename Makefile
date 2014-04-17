CC=clang
FLAGS=-static

.PHONY: init strip clean

all: init strip

init:
	$(CC) init.c $(FLAGS) -o init

strip:
	strip init

clean:
	rm init
