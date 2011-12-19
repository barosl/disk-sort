CFLAGS = -O3 -Wall -Werror -Wno-unused-result

STRIP = strip

disk-sort: src/main.o src/str.o src/sort.o src/clock.o
	$(CC) $(CFLAGS) -o $@ $+
	$(STRIP) $@

clean:
	rm -f disk-sort src/*.o
