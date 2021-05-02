#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include "gsl/gsl/gsl_randist.h"
#include "gsl/gsl/gsl_rng.h"

#include "definitions.h"
#include "vaccine.h"
#include "person.h"
#include "environment.h"
#include "population.h"
#include "probability.h"

int i, j, k;
int main(int argc, char *argv[])
{
    init_world();
    create_population();
    per_cicle();
}
int when_change_group = iterations / group_to_vaccine; //Para saber cada cuantas iteraciones tengo que cambiar de grupo
int cont_iterations = 0;

void per_cicle()
{
    int id_contIAux, id_contNotIAux, vaccines_left;
    idx_iter = 0;
    for (k = 0; k < iterations; k++) // ITERACCIONES
    {
        //NUEVO ----- VACUNACION-------------------
        if (cont_iterations >= when_change_group)
        {
            group_to_vaccine--;
            cont_iterations = 0;
        }
        else
            cont_iterations++;
        ///-------------END VACUNAICON-------------
        vaccines_left = num_person_to_vaccine;
        id_contIAux = id_contI;
        id_contNotIAux = id_contNotI;

        for (i = 0; i < id_contIAux; i++) // INFECTED
        {
            changeState(&l_person_infected[i]);
            change_move_prob(&l_person_infected[i]);
            move(&l_person_infected[i]);
        }

        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED Y VACCINED
        {
            if (&l_person_notinfected[i].id != -1)
            {
                if (vaccines_left > 0)
                {
                    vaccines_left = vaccine(&l_person_notinfected[i], vaccines_left);
                    change_move_prob(&l_person_notinfected[i]);
                    move(&l_person_notinfected[i]);
                }
            }
            if (i <= id_contVaccined)
            {
                change_move_prob(&l_vaccined[i]);
                move(&l_vaccined[i]);
            }
        }

        idx_iter++;
    }
}

void changeState(person_t *person) // 1 and 2 States
{
    int state = person->state;
    if (person->incubation_period > 0)
    {
        person->incubation_period--;
    }
    else
    {
        if (person->recovery == 0)
        {
            person->state = 3;
            person->incubation_period = random_number(3, 5); // TODO: Cambiar esto
            person->recovery = random_number(3, 5);
        }
        else
        {
            person->recovery--;
            if (person->state == 1)
            {
                propagate(person);
            }
        }
    }
}

void propagate(person_t *person)
{
    int directions[12][2] = {{1, 1}, {2, 1}, {1, 2}, {2, 2}, {-1, -1}, {-1, -2}, {-2, -1}, {-2 - 2}, {0, 1}, {0, 2}, {1, 0}, {2, 0}};
    index_t index;
    int x = person->coord[0];
    int y = person->coord[1];
    person_t person_aux;
    float prob_aux;
    for (i = 0; i < 12; i++)
    {
        index = world[x + directions[i][0], y + directions[i][1]];
        if (index.l == NOT_INFECTED)
        {
            person_aux = l_person_notinfected[index.id];
            change_infection_prob(&person_aux);
            if (person_aux.prob_infection > MAX_INFECTION)
            {
                prob_aux = 1000 * calculate_prob_death(person_aux.age);
                if (random_number(0, MAX_DEATH) <= prob_aux)
                { // MUERE
                    person_aux.state = 5;
                }
                else
                {
                    if (random_number(0, 1) == 0) // INFECCIOSO
                    {
                        person_aux.state = 2;
                    }
                    else
                    {
                        person_aux.state = 1;
                    }
                    l_person_notinfected[person_aux.id].id = -1;
                    id_contI++;
                    person_aux.id = id_contI;
                    l_person_infected[id_contI] = person_aux;
                }
            }
        }
    }
}