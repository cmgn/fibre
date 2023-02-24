OBJECTS += vec.o hashmap.o fibre_io.o fibre.o queue.o third_party/coro/libcoro.a
CCFLAGS += -Wall -Werror

libfibre.a: $(OBJECTS)
	ld -r $^ -o $@

example: libfibre.a example.c
	$(CC) $(CCFLAGS) -o $@ $^

third_party/coro/libcoro.a:
	make -C third_party/coro

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) libfibre.a example $(OBJECTS)
