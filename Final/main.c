#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h> // notice this! you need it!
#include <math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include "definitions.h"
///----VAriables-//

int main(int argc, char *argv[])
{
    init_gsl();
    printf("EMPIEZA EL JUEGO DE LA PURGA\n");
    id_contI = 0;
    id_contNotI = 0;
    seed = 3; //CAMBIAR
    mu = 100;
    alfa = 2;
    beta = 5;
    radius = 2;
    group_to_vaccine = 8;
    num_persons_to_vaccine = 70 / (POPULATION_SIZE * percent);
    iter = 10;
    posX = 0;
    posY = 0;
    cont_bach = 1;
    cont_death = 0;
    vaccines_left = 0;
    id_contVaccined = 0;
    init_world();
    create_population();
    per_cicle();
    gsl_rng_free(r);
}

void per_cicle()
{
    int id_contIAux, id_contNotIAux, is_vaccined;
    int when_change_group = 50; //Para saber cada cuantas iteraciones tengo que cambiar de grupo
    int cont_iterations = 0;
    for (k = 0; k < iter; k++) // ITERACCIONES
    {
        printf("--------------------------------ITERACION --> %d--------------------------------\n", k);
        print_world();
        if (cont_iterations >= when_change_group)
        {
            group_to_vaccine--;
            cont_iterations = 0;
        }
        else
        {
            cont_iterations++;
        }
        vaccines_left = num_persons_to_vaccine;
        id_contIAux = id_contI;
        id_contNotIAux = id_contNotI;

        for (i = 0; i < id_contIAux; i++) // INFECTED
        {
            if (l_person_infected[i].id != -1)
            {
                change_state(l_person_infected[i]);
                if (l_person_infected[i].id != -1) // Ha cambiado de estado y no se mueve
                {
                    change_move_prob(&l_person_infected[i]);
                    move(&l_person_infected[i]);
                }
            }
        }

        for (i = 0; i < id_contVaccined; i++) // VACCINED
        {
            change_move_prob(&l_vaccined[i]);
            move(&l_vaccined[i]);
        }
        
        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED 
        {
            if (l_person_notinfected[i].id != -1)
            {
                if (vaccines_left > 0)
                {
                    is_vaccined = vacunate(l_person_notinfected[i]);
                    if (is_vaccined == 0) // NOT-VACCINED
                    {
                        change_move_prob(&l_person_notinfected[i]);
                        move(&l_person_notinfected[i]);
                    }
                }else{
                    change_move_prob(&l_person_notinfected[i]);
                    move(&l_person_notinfected[i]);
                }
            }
        }
        print_world();
        if (cont_bach == BATCH)
        {
            cont_bach = 1;
            //smetrics();
        }
        else
        {
            cont_bach++;
        }
    }
}

void metrics()
{
}

void change_state(person_t person) // 1(INFECCIOSO) and 2(NO-INFECCIOSO) States
{
    if (person.incubation_period > 0)
    {
        l_person_infected[person.id].incubation_period--;
    }
    else
    {
        if (person.recovery == 0)
        {
            l_person_infected[person.id].state = 3;
            l_person_infected[person.id].incubation_period = random_number(3, 5);
            l_person_infected[person.id].recovery = random_number(3, 5);
            l_person_infected[person.id].id = -1;         
            l_vaccined[id_contVaccined] = person; 
            id_contVaccined++;
        }
        else
        {
            l_person_infected[person.id].recovery--;
            if (l_person_infected[person.id].state == 2)
            {
                propagate(&person);
            }
        }
    }
}

void propagate(person_t *person)
{
    int directions[12][2] = {{1, 0}, {2, 0}, {1, 1}, {0, 1}, {0, 2}, {-1, 1}, {-1, 0}, {-2, 0}, {-1, -1}, {0, -1}, {0, -2}, {1, -1}};
    index_t index;
    int x = person->coord[0];
    int y = person->coord[1];
    person_t person_aux;
    float prob_aux;
    //printf(">>>>>INFECTAR\n");
    for (i = 0; i < 12; i++) // Todas las direcciones
    {
        if ((x + directions[i][0]) < size_world && (y + directions[i][1]) < size_world) // Mantenerse dentro
        {
            index = world[x + directions[i][0]][y + directions[i][1]];
            if (index.id != -1 && index.l == NOT_INFECTED) // Persona no infectada y asignada
            {
                person_aux = l_person_notinfected[index.id];
                change_infection_prob(&person_aux);
                if (person_aux.prob_infection > MAX_INFECTION) // Se infecta
                {
                    l_person_notinfected[person_aux.id].id = -1;
                    prob_aux = 1000 * calculate_prob_death(person_aux.age);
                    if (random_number(0, MAX_DEATH) <= prob_aux) // MUERE
                    {
                        printf(">>>>>%d HA MUERTO!\n", person_aux.id);
                        person_aux.state = 5;
                        world[x + directions[i][0]][y + directions[i][1]].id = -1;
                        cont_death++;
                    }
                    else
                    {
                        printf(">>>>>%d SE HA INFECTADO!\n", person_aux.id);
                        if (random_number(0, 1) == 0) // INFECCIOSO
                        {
                            person_aux.state = 2;
                        }
                        else
                        {
                            person_aux.state = 1;
                        }
                        person_aux.id = id_contI;
                        l_person_infected[id_contI] = person_aux;
                        world[x + directions[i][0]][y + directions[i][1]].l = INFECTED;
                        world[x + directions[i][0]][y + directions[i][1]].id = id_contI;
                        id_contI++;
                    }
                }
            }
        }
    }
}

//----------------ENVIRONMENT--------------------//////

void init_world()
{
    //world = malloc(size_world * size_world * sizeof(index_t *));
    for (i = 0; i < size_world; i++)
    {
        for (j = 0; j < size_world; j++)
        {

            world[i][j].id = -1;
        }
    }
}

void move(person_t *person)
{
    int x = person->coord[0];
    int y = person->coord[1];
    int speed = 1;//person->speed[1];
    if (speed != 0)
    {
        int direction = 7;//person->speed[0];
        //printf("%d%d\n",speed,direction);
        int diagonals[4][2] = {{-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
        int directions[8][2] = {{0, 1}, {0, 2}, {-1, 0}, {-2, 0}, {0, -1}, {0, -2}, {1, 0}, {2, 0}};
        //int directions[12][2] = {{1, 0}, {2, 0}, {1, 1}, {0, 1}, {0, 2}, {-1, 1}, {-1, 0}, {-2, 0}, {-1, -1}, {0, -1}, {0, -2}, {1, -1}};
        if (direction == 2 || direction == 4 || direction == 6 || direction == 8) // DIAGONAL
        {
            x += diagonals[(direction / 2) - 1][0];
            y += diagonals[(direction / 2) - 1][1];
        }
        else
        {
            x += directions[direction - 1 + speed - 1][0];
            y += directions[direction - 1 + speed - 1][1];
        }
        if (x < size_world && y < size_world && x >= 0 && y >= 0)
        {
            if (world[x][y].id == -1) // EMPTY POSITION
            {
                world[person->coord[0]][person->coord[1]].id = -1; // Se elimina la anterior pos
                world[x][y].id = person->id;
                if (person->state == 1 || person->state == 2)
                {
                    world[x][y].l = INFECTED;
                }
                else if (person->state == 0)
                {
                    world[x][y].l = NOT_INFECTED;
                }
                else
                {
                    world[x][y].l = VACCINED;
                }
                person->coord[0] = x;
                person->coord[1] = y;
            }
        }
    }
}
///-----------------------PERSON-------------------------////

person_t create_person()
{
    person_t person;
    person.age = random_number(1, 100);
    person.prob_infection = gsl_ran_beta(r, alfa, beta);
    person.state = random_number(0, 2);
    person.incubation_period = random_number(0, MAX_INCUBATION);
    person.recovery = random_number(0, MAX_RECOVERY);
    calculate_init_position(&person);
    index_t index;
    if (person.state == 0 || person.state == 3) // SANO
    {
        person.id = id_contNotI;
        l_person_notinfected[id_contNotI] = person;
        id_contNotI++;
        index.l = NOT_INFECTED;
    }
    else // INFECTADO
    {
        person.id = id_contI;
        l_person_infected[id_contI] = person;
        id_contI++;
        index.l = INFECTED;
    }
    index.id = person.id;
    world[person.coord[0]][person.coord[1]] = index; // INTRODUCIDO EN WORLD
    print_person(person);
    return person;
}

void print_person(person_t p)
{
    printf("ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]\n",
           p.id, p.state, p.age, p.incubation_period, p.prob_infection, p.recovery, p.coord[0], p.coord[1]);
}

float calculate_prob_death(int edad)
{
    for (int i = 0; i < 9; i++)
    {
        if (edad >= 10 * i && edad < 10 * i + 10)
        {
            return l_death_prob[i];
        }
        else if (i == 8)
        {
            return l_death_prob[i];
        }
    }
    return 0.0;
}

int random_number(int min_num, int max_num)
{
    srand(time(NULL));
    return rand() % (max_num + 1) + min_num;
}

void calculate_init_position(person_t *person)
{
    // TODO: Cambiar esto para que sea con el random_number
    person->coord[0] = posX; // coord x
    person->coord[1] = posY; // coord y
    posY++;
    if (posY == size_world)
    {
        posY = 0;
        posX++;
    }
}

///-----------------------POPULATION-------------------------////

void init_gsl()
{
    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);
    seed = time(NULL);
    gsl_rng_set(r, seed);
}

void change_move_prob(person_t *person)
{
    person->speed[0] = random_number(0, MAX_DIRECTION);
    person->speed[1] = random_number(0, MAX_SPEED);
}

void change_infection_prob(person_t *person)
{
    person->prob_infection = gsl_ran_beta(r, alfa, beta);
}

float calculate_ageMean()
{
    return (alfa / (alfa + beta)) * mu;
}

void create_population()
{
    //Inicializo la lista
    init_lists();
    //Calculo las probabilidades y la edad media
    calculate_ageMean();
    //Relleno la lista con personas
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        create_person();
        sleep(1);
    }
}

void init_lists()
{
    l_person_infected = (person_t *)malloc(POPULATION_SIZE * sizeof(person_t));
    l_person_notinfected = (person_t *)malloc(POPULATION_SIZE * sizeof(person_t));
    l_vaccined = (person_t *)malloc(POPULATION_SIZE * sizeof(person_t));
}

////////----------------VACUNATE--------------////////////

int vacunate(person_t person)
{
    if (person.age >= group_to_vaccine * 10)
    {
        printf(">>>>>%d VACUNADO!\n",person.id);
        l_person_notinfected[person.id].id = -1;
        person.state = 4;
        l_vaccined[id_contVaccined] = person;
        world[person.coord[0]][person.coord[1]].l = VACCINED;
        world[person.coord[0]][person.coord[1]].id = id_contVaccined;
        id_contVaccined++;
        return 1;
    }else{
        return 0; // no vacunado
    }

}

void print_world()
{
    index_t index_aux;
    for (i = 0; i < size_world; i++)
    {

        for (j = 0; j < size_world; j++)
        {

            index_aux = world[i][j];
            if (index_aux.id == -1)
            {
                printf("| --- |");
            }
            else
            {
                if (index_aux.l == INFECTED)
                {
                    printf("| I-%d |", index_aux.id);
                }
                else if (index_aux.l == NOT_INFECTED)
                {
                    printf("| N-%d |", index_aux.id);
                }
                else
                {
                    printf("| V-%d |", index_aux.id);
                }
            }
        }
        printf("\n");
    }
    printf("\n\n\n\n");
}
