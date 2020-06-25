CC=gcc
CFLAGS=-lsoundio -lncurses -lm
DEPS=wave.h
OBJ = main.o wave.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(DEPS)