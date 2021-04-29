#include "environment.h"
#include "person.h"
#include "population.h"

int i,j,position;

void init_world(){
    world = (index_t *)calloc(size_world*size_world,sizeof(index_t));
}

void move(person_t *person)
{
    int x = person->coord[0];
    int y = person->coord[1];
    //prob_direction
}
