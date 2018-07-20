CC=gcc

all: clean dttool

dttool:
	$(CC) device_tree.c dttool.c -o dttool

clean:
	rm -f dttool
