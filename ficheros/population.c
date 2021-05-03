#include "population.h"
#include "probability.h"
#include "person.h"
#include "../gsl/gsl/gsl_randist.h"
#include "../gsl/gsl/gsl_rng.h"

seed = 1;
mu = 100;
alfa = 2;
beta = 5;
size = 1000;
//Funciones
radius = 2; //Es algo que hemos decidido
//Inicializar la librería, el generador de números pseudoaleatorios y la semilla.

void init_gsl()
{
    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);
    gsl_rng_set(r, seed);
}

void init_lists()
{
    l_person_infected = (person_t *)malloc(population_size * sizeof(person_t));
    l_person_notinfected = (person_t *)malloc(population_size * sizeof(person_t));
    l_vaccined = (person_t *)malloc(population_size * sizeof(person_t));
}

// void calculate_probs()
// {
//     init_gsl();
//     prob_infection = gsl_ran_beta(r, alfa, beta);
//     recovery_period = random_number(1, MAX_RECOVERY);
//     prob_direction = gsl_ran_beta(r, alfa, beta);
//     prob_speed = gsl_ran_beta(r, alfa, beta);
//     gsl_rng_free(r);
// }

void change_move_prob(person_t *person) {
    person->speed[0] = random_number(0,MAX_DIRECTION);
    person->speed[1] = random_number(0,MAX_SPEED);
}

void change_infection_prob(person_t *person) {
    init_gsl();
    person->prob_infection = gsl_ran_beta(r,alfa,beta);
    gsl_rng_free(r);
}

float calculate_ageMean()
{
    return (alfa / (alfa + beta)) * mu;
}

void create_population()
{
    //Inicializo la lista
    init_lista();
    //Calculo las probabilidades y la edad media
    calculate_ageMean();
    //Relleno la lista con personas
    for (int i = 0; i < population_size; i++)
    {
        person_t person = create_person();
    }
}