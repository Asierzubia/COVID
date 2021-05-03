OBJS	= main.o
SOURCE	= main.c
HEADER	= definitions.h
OUT	= covid.out
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lm -lgsl -lgslcblas

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c 


clean:
	rm -f $(OBJS) $(OUT)
