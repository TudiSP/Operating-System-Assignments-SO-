CC = gcc
exec = so-cpp
obj = so-cpp.o
deps = so.cpp.h
CFLAGS = -Wall -std=c89
build: $(obj)
	$(CC) -o $(exec) $< 
%.o: %.c $(deps)
	$(CC) -c -o $@ $< $(CFLAGS)
clean: %.o
	rm $(obj) $(exec)

