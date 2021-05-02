#include "environment.h"
#include "person.h"
#include "population.h"

int i, j, position;

void init_world()
{
    world = (index_t *)calloc(size_world * size_world, sizeof(index_t));
    for ( i = 0; i < size_world; i++)
    {
        for (j = 0; j < size_world; j++)
        {
            world[i,j].id = -1;
        }   
    }
    
}
//----------------NUEVO----------------------///
// Ahora ya no tenemos un vector para la direccion y otro para la velocidad
// Ahora lo que tenemos es un unico vector para la direccion y la velocidad

void move(person_t *person)
{
    //Obtenemos las coordenadas
    int x = person->coord[0];
    int y = person->coord[1];
    int speed = person->speed[1];
    if (speed == 0)
    {
        /* code */
    }
    int direction = person->speed[0];
    int directions[12][2] = {{1, 0}, {2, 0}, {1, 1}, {0, 1}, {0, 2}, {-1, 1}, {-1, 0}, {-2, 0}, {-1, -1}, {0, -1}, {0, -2}, {1, -1}};
    if (direction == 2 || direction == 4 || direction == 6 || direction == 8) // DIAGONAL
    {
        x += directions[direction + 1][0];
        y += directions[direction + 1][1];
    }
    else
    {
        x += directions[(direction - 1) + speed][0];
        y += directions[(direction - 1) + speed][1];
    }

    if (x<size_world && y <size_world)
    {
        if (world[x,y].id != -1)
        {
            
        }
    }
    
}