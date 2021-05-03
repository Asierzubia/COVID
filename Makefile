OBJS	= ficheros/main.o
SOURCE	= ficheros/main.c
HEADER	= headers/population.h headers/environment.h headers/person.h headers/probability.h headers/vaccine.h
OUT	= covid.out
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lm -lgsl

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

ficheros/main.o: ficheros/main.c
	$(CC) $(FLAGS) ficheros/main.c 


clean:
	rm -f $(OBJS) $(OUT)