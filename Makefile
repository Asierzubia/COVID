OBJS    = metrics.c conf.c prob.c lists.c person.o main.o
SOURCE  = mwtrics.o conf.o prob.o lists.o person.c main.c
HEADER  = definitions.h
OUT     = covid.out
CC	 = mpicc
FLAGS    = -g -c -Wall
LFLAGS   = -lm -lgsl

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

metrics.o: metrics.c
	$(CC) $(FLAGS) metrics.c

conf.o: conf.c
	$(CC) $(FLAGS) conf.c

prob.o: prob.c
	$(CC) $(FLAGS) prob.c

lists.o: lists.c
	$(CC) $(FLAGS) lists.c

person.o: person.c
	$(CC) $(FLAGS) person.c

main.o: main.c
	$(CC) $(FLAGS) main.c


clean:
	rm -f $(OBJS) $(OUT)