#ifndef POPULATION_H
#define POPULATION_H

//--------Nuevo---------//
#define max_speed 3 //Falta inicializarlo con un valor aleatorio en populations.c
#define max_direction 8 //Falta inicializarlo con un valor aleatorio en populations.c
// Pon que el max speed sea 3
// Pon que el max direcction sea 8. Arriba,abajo,.... ya sabes
//----------------------//

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