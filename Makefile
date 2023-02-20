OBJECTS += fibre.o queue.o third_party/coro/libcoro.o
CCFLAGS += -Wall -Werror -O3

main: $(OBJECTS)
	$(CC) $(CCFLAGS) -o main main.c $(OBJECTS)

third_party/coro/libcoro.o:
	make -C third_party/coro

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) main $(OBJECTS)
