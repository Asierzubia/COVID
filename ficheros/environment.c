#include "environment.h"
#include "person.h"
#include "population.h"

int i, j, position;

void init_world()
{
    world = (index_t *)calloc(size_world, sizeof(index_t));
    for (i = 0; i < size_world; i++)
    {
        world[i] = (index_t *)calloc(size_world, sizeof(index_t));
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
    int speed = person->speed[1];
    if (speed != 0)
    {
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
        if (x < size_world && y < size_world)
        {
            if (world[x][y].id != -1) // POS VACIA
            {
                world[person->coord[0]][person->coord[1]].id = -1;
                world[x][y].id = person->id;
                if (person->state == 1 || person->speed == 2)
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
            }
        }
    }
}