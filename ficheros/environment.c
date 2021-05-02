#include "environment.h"
#include "person.h"
#include "population.h"

int i,j,position;

void init_world(){
    world = (index_t *)calloc(size_world*size_world,sizeof(index_t));
}


//----------------NUEVO----------------------///
// Ahora ya no tenemos un vector para la direccion y otro para la velocidad
// Ahora lo que tenemos es un unico vector para la direccion y la velocidad

void move(person_t *person)
{
    //Obtenemos las coordenadas
    int x = person->coord[0];
    int y = person->coord[1];
    //Valocidad y direccion
    int velocity = person->c_v[1];
    int direcction = person->c_v[0];

    //Hay que tener en cuenta los limites de la matriz ficcticia
    // El limite es el tama;o de la matriz

    
}