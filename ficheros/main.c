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

//-------------DUDAS-----------------------------------------
//Las personas recuperadas se pueden volver a contagiar??????
//Que estado le ponemos a una persona contiagiosa, estado con sintomas o sin sintomas.
//Que sentido tienes que toda la poblacion tenga la misma probabilidad de moverse? No sera que cada persona tiene una probabilidad de moverse? Porque sino habria cilcos en los que se mueven todas las personas, y otros en los que no se moveria ninguna.

void per_cicle()
{
    int id_contIAux, id_contNotIAux, vaccines_left;
    for (k = 0; k < iterations; k++) // ITERACCIONES
    {
        id_contIAux = id_contI;
        id_contNotIAux = id_contNotI;
        vaccines_left = num_persons_to_vaccine;
        for (i = 0; i < id_contIAux; i++) // INFECTED
        {
            changeState(&l_person_infected[i]);
            move(&l_person_infected[i]);
        }
        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED Y VACCINED
        {
            if (vaccines_left > 0)
            {
                vaccine(&l_person_notinfected[i]);
            }
            move(&l_person_notinfected[i]);
            if (i <= id_contVaccined)
            {
                move(&l_vaccined[i]);
            }
        }
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
    index_t index_aux;
    int x = person->coord[0];
    int y = person->coord[1];
    int posX, posY;
    person_t person_target;
    for (i = 0; i < 12; i++)
    {
        posX = x + directions[i][0];
        posY = y + directions[i][1];
        if (posX <= size_world && posY <= size_world)
        {
            index_aux = world[posX, posY];
            // TODO: MIRAR SI ES VACIO
            if (index_aux.l=NOT_INFECTED)
            {
                l_person_notinfected[index_aux.id];
            }
        }
    }
}