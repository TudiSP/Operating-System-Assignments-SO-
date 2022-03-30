CC = gcc
exec = so_stdio.so
deps = ./utils/so_stdio.h
obj = so_stdio.o 
CFLAGS = -fPIC -Wall -std=c89
build: $(obj)
	$(CC) -shared -o $(exec) $< 
		
%.o: %.c $(deps)
	$(CC) -c -o $@ $< $(CFLAGS)
		
clean: %.o
	rm %.o $(exec)

