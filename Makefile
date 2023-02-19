OBJECTS += third_party/coro/libcoro.o

main: $(OBJECTS)
	$(CC) $(CCFLAGS) -o main main.c $(OBJECTS)

third_party/coro/libcoro.o:
	make -C third_party/coro

.PHONY: clean
clean:
	$(RM) main $(OBJECTS)
