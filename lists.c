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

void free_all_lists(){

    free_world();
    free(l_person_infected);
    free(l_person_notinfected);
    free(l_vaccined);
    free(l_cont_node_move);
    free(l_cont_node_propagate);
    free(l_cont_persona_mover);
    free(l_cont_person_return);
    free(l_positions);
    free(recv_metrics);
    free(cont_index_return);
    free(l_person_propagate_recive);
    free_move_list();
    free_prop_list();
    free_person_move();
    free_index_list();
    free_person_return();
}

void init_all_lists(){
    
    recv_metrics = malloc((5*world_size) * sizeof(float));
    init_world(quadrant_x, quadrant_y);
    l_person_infected = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_person_notinfected = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_vaccined = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_cont_node_move = malloc(world_size * sizeof(int));
    l_cont_node_propagate = malloc(world_size * sizeof(int));
    l_cont_persona_mover = malloc(world_size * sizeof(int));
    l_cont_person_return = malloc(world_size * sizeof(int));
    l_positions = malloc(buffer_node * sizeof(char)); //init_list_archives(1024);
    recv_metrics = malloc((5*world_size) * sizeof(float));
    cont_index_return = malloc(world_size * sizeof(int));
    l_person_propagate_recive = malloc(conf.world_size * conf.world_size * sizeof(coord_t));
    
    init_move_list(world_size, quadrant_x * quadrant_y);
    init_prop_list(world_size, quadrant_x * quadrant_y);
    init_person_move_list(world_size, quadrant_x * quadrant_y);
    init_index_list(world_size, quadrant_y * quadrant_x);
    init_person_return(world_size, quadrant_x * quadrant_y);

}

void init_world(int size_x, int size_y)
{
    int i;
    world = malloc(size_x * sizeof(index_t *));
    if (world == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < size_x; i++)
    {
        world[i] = malloc(size_y * sizeof(index_t));
        if (world[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            world[i][j].id = -1;
        }
    }
}

char *init_list_archives(int size)
{
    char *result = malloc(size * sizeof(char));
    return result;
}

person_t *init_lists(int pop)
{
    person_t *result = malloc( pop * sizeof(person_t));
    return result;
}

void init_index_list(int size_x, int size_y)
{
    int i;
    index_return_person = malloc(size_x * sizeof(int *));
    if (index_return_person == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < size_x; i++)
    {
        index_return_person[i] = malloc(size_y * sizeof(int));
        if (index_return_person[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            index_return_person[i][j] = -1;
            index_return_person[i][j] = -1;
        }
    }
}

void init_person_return(int size_x, int size_y)
{

    lista_de_persona_mover_return = malloc(size_x * sizeof(int *));
    if (lista_de_persona_mover_return == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        lista_de_persona_mover_return[i] = malloc(size_y * sizeof(int));
        if (lista_de_persona_mover_return[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            lista_de_persona_mover_return[i][j] = -1;
            lista_de_persona_mover_return[i][j] = -1;
        }
    }
}

void init_prop_list(int size_x, int size_y)
{
    l_person_propagate = malloc(size_x * sizeof(coord_t *));
    if (l_person_propagate == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        l_person_propagate[i] = malloc(size_y * sizeof(coord_t));
        if (l_person_propagate[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            l_person_propagate[i][j].x = -1;
            l_person_propagate[i][j].y = -1;
        }
    }
}

void init_person_move_list(int size_x, int size_y)
{

    lista_de_persona_mover = malloc(size_x * sizeof(person_move_t *));
    if (lista_de_persona_mover == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int l, n;
    for (l = 0; l < size_x; l++)
    {
        lista_de_persona_mover[l] = malloc(size_y * sizeof(person_move_t));
        if (lista_de_persona_mover[l] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (n = 0; n < size_y; n++)
        {
            lista_de_persona_mover[l][n].person.id = -1;
        }
    }
}

void init_move_list(int size_x, int size_y)
{
    l_person_moved = malloc(size_x * sizeof(person_move_t *));
    if (l_person_moved == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int l, n;
    for (l = 0; l < size_x; l++)
    {
        l_person_moved[l] = malloc(size_y * sizeof(person_move_t));
        if (l_person_moved[l] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (n = 0; n < size_y; n++)
        {
            l_person_moved[l][n].person.id = -1;
        }
    }
}

void free_person_move()
{

    int o;
    for (o = 0; o < world_size; o++)
    {
        free(lista_de_persona_mover[o]);
    }
    free(lista_de_persona_mover);
}
void free_world()
{
    int i;
    for (i = 0; i < quadrant_x; i++)
    {
        free(world[i]);
    }
    free(world);
}

void free_index_list()
{
    int o;
    for (o = 0; o < world_size; o++)
    {
        free(index_return_person[o]);
    }
    free(index_return_person);
}

void free_person_return()
{
    int o;
    for (o = 0; o < world_size; o++)
    {
        free(lista_de_persona_mover_return[o]);
    }
    free(lista_de_persona_mover_return);
}


void free_move_list()
{
    int i;
    for (i = 0; i < world_size; i++)
    {
        free(l_person_moved[i]);
    }
    free(l_person_moved);
}

void free_prop_list()
{

    int i;
    for (i = 0; i < world_size; i++)
    {
        free(l_person_propagate[i]);
    }
    free(l_person_propagate);
}

void realocate_lists()
{
    int i;
    int last_value = id_contI;
    for (i = 0; i < id_contI; i++)
    { // INFECTED

        if (l_person_infected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contI; j++)
            {
                if (l_person_infected[j].id != -1)
                {
                    last_value = i + 1;
                    memcpy(&l_person_infected[i], &l_person_infected[j], sizeof(person_t));
                    l_person_infected[i].id = i;
                    l_person_infected[j].id = -1;
                    world[l_person_infected[i].coord.x][l_person_infected[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
    id_contI = last_value;
    last_value = id_contNotI;
    for (i = 0; i < id_contNotI; i++)
    { // INFECTED
        if (l_person_notinfected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contNotI; j++)
            {
                if (l_person_notinfected[j].id != -1)
                {
                    last_value = i + 1;
                    memcpy(&l_person_notinfected[i], &l_person_notinfected[j], sizeof(person_t));
                    l_person_notinfected[i].id = i;
                    l_person_notinfected[j].id = -1;
                    world[l_person_notinfected[i].coord.x][l_person_notinfected[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
    id_contNotI = last_value;
    last_value = id_contVaccined;
    for (i = 0; i < id_contVaccined; i++)
    { // INFECTED
        if (l_vaccined[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contVaccined; j++)
            {
                if (l_vaccined[j].id != -1)
                {
                    last_value = i + 1;
                    memcpy(&l_vaccined[i], &l_vaccined[j], sizeof(person_t));
                    l_vaccined[i].id = i;
                    l_vaccined[j].id = -1;
                    world[l_vaccined[i].coord.x][l_vaccined[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
    id_contVaccined = last_value;
}

void print_lists(int world_rank)
{

    //printf("---INFECTADOS-----\n");
    for (i = 0; i < id_contI; i++)
    {
        print_person(&l_person_infected[i], world_rank, 'i');
    }
    //printf("---NO INFECTADOS-----\n");
    for (i = 0; i < id_contNotI; i++)
    {
        print_person(&l_person_notinfected[i], world_rank, 'n');
    }
    //printf("---VACUNADOS-----\n");
    for (i = 0; i < id_contVaccined; i++)
    {
        print_person(&l_vaccined[i], world_rank, 'v');
    }
}

void print_world(int worldRank)
{

    for (i = 0; i < quadrant_x; i++)
    {
        for (j = 0; j < quadrant_y; j++)
        {
            printf("El P%d inicializada el mundo, posicion | X->%d  | Y->%d  |\n", worldRank, i, j);
        }
    }
}
