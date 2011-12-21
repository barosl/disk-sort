CFLAGS = -O3 -Wall -Werror -Wno-unused-result

STRIP = strip

NAME = disk-sort
OBJS = src/main.o src/str.o src/sort.o src/clock.o

.PHONY: c clean

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $+
	$(STRIP) $@

c: clean
clean:
	rm -f $(NAME) src/*.o
