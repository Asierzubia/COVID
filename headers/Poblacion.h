#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>


#Variables globales

int size;
float age_mean;
int radius;
float prob_infection;
float recovery_period;
float prob_direction;
float prob_speed;
person *personList;
int i,j;