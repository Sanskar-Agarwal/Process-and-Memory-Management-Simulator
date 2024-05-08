CC = gcc
CFLAGS = -Wall
EXEC = allocate
SRCS = main.c linkedlist.c 
OBJS = $(SRCS:.c=.o)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) -lm

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)

format:
	clang-format -i *.c *.h