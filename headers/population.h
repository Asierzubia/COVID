#ifndef POPULATION_H
#define POPULATION_H

// Variables globales

int population_size;
float age_mean;
int radius;
float prob_infection;
float recovery_period;
float prob_direction;
float prob_speed;
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
// int index_infected,index_notinfected;
int id_contVaccined;

void init_lists();
void calculate_probs();
float calculate_ageMean();
void create_population();
#endif // POPULATION_H