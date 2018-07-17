CC=gcc

all: dttool

dttool:
	$(CC) dttool.c -o dttool

clean:
	rm dttool
