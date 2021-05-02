#ifndef POPULATION_H
#define POPULATION_H

//--------Nuevo---------//
#define MAX_SPEED 2     
#define MAX_DIRECTION 8 
#define MAX_INCUBATION 15 
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.6
#define MAX_DEATH 200
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
float calculate_ageMean();
void create_population();
void change_move_prob(person_t *person);
void change_infection_prob(person_t *person);
#endif // POPULATION_H