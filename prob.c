#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <getopt.h>
#include "definitions.h"

gsl_rng *r;

void init_gsl(int seed)
{
    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);
    gsl_rng_set(r, seed);
}


int random_number(int min_num, int max_num)
{
    return rand() % (max_num + 1) + min_num;
}

void change_move_prob(person_t *person)
{
    person->speed[0] = random_number(1, MAX_DIRECTION);
    person->speed[1] = random_number(0, MAX_SPEED);
}

void change_infection_prob(person_t *person)
{
    float alpha_local, beta_local;
    float normalized_death = 50 / 100.0;
    float variance = pow((16 / 100.0),2);

    alpha_local = roundf( ((1 - normalized_death) * pow(normalized_death,2) - (variance * normalized_death)) / variance );
    beta_local = roundf( (alpha_local * ( 1 - normalized_death ))  / normalized_death );
    person->prob_infection = gsl_ran_beta(r, alpha_local, beta_local);
}

void init_person_parameters(person_t *persona, int state, int id_local)
{
    persona->id = id_local;
    persona->age = roundf(mu * gsl_ran_beta(r, alpha, beta));
    persona->prob_infection = gsl_ran_beta(r, alpha, beta);
    persona->state = state;
    persona->incubation_period = random_number(0, MAX_INCUBATION);
    persona->recovery = random_number(0, MAX_RECOVERY);
    persona->id_global = identificador_global;
    persona->speed[0] = random_number(1, MAX_DIRECTION);
    persona->speed[1] = random_number(0, MAX_SPEED);
    world[persona->coord.x][persona->coord.y].id = id_local;
}

void getAlphaBeta(int ageMean){

	float normalized_age = ageMean / 100.0;
    float variance = pow((16 / 100.0),2);

    alpha = roundf( ((1 - normalized_age) * pow(normalized_age,2) - (variance * normalized_age)) / variance );
    beta = roundf( (alpha * ( 1 - normalized_age ))  / normalized_age );
}