CPPFLAGS= -I libs -Wall -Wextra
CFLAGS= -std=gnu11 -Os -g -ggdb

.PHONY: clean

cchar: src/cchar.o libs/pfasta.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f **/*.o cchar
