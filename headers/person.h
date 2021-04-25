#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

typedef struct person {
    
    int age;
    int state;
    float prob_death;
    int coord[2];
    int speed[2];
    int init_position;

} person_t;

