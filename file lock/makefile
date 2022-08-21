SHELL = /bin/bash
CC = gcc
CFLAGS = -g -pthread
SRC = $(wildcard *.c)
EXE = $(patsubst %.c, %, $(SRC))

all: ${EXE}

%:	%.c ${wildcard *.db}

	${CC} ${CFLAGS} $@.c -o $@
	@echo -n 3500 > lockf.db
	@echo -n 3500 > flock.db
	chmod g+s lockf.db
	chmod g-x lockf.db

	
clean:
	rm ${EXE} *.db