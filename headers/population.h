#ifndef POPULATION_H
#define POPULATION_H

// Variables globales

int size;
float age_mean;
int radius;
float prob_infection;
float recovery_period;
float prob_direction;
float prob_speed;
person_t *personList;
int i,j;

void init_lista();
void calculate_probs();
float calculate_ageMean();
void create_population();
#endif // POPULATION_H