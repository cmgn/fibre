OBJECTS += vec.o hashmap.o fibre_io.o fibre.o queue.o third_party/coro/libcoro.a
CCFLAGS += -Wall -Werror

example: $(OBJECTS) example.c
	$(CC) $(CCFLAGS) -o $@ $^

third_party/coro/libcoro.a:
	make -C third_party/coro

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) example $(OBJECTS)
