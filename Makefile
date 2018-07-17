CC=gcc

all: clean dttool

dttool:
	$(CC) dttool.c -o dttool

clean:
	rm -f dttool
