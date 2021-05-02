#include "person.h"
#include "probability.h"
#include "environment.h"
#include "population.h"
#include "gsl/gsl/gsl_randist.h"
#include "gsl/gsl/gsl_rng.h"

id_contI = 0;
id_contNotI = 0;
l_death_prob = {0, 0.002, 0.002, 0.002 , 0.004, 0.013, 0.036, 0.08, 0.148};

person_t create_person()
{
    person_t person;
    init_gsl();
    person.age = (int)gsl_ran_beta(r, alfa, beta); 
    person.prob_infection = gsl_ran_beta(r, alfa, beta);
    person.state = random_number(0,3); 
    person.incubation_period = random_number(0,MAX_INCUBATION);          
    person.recovery = random_number(0,MAX_RECOVERY);

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
    gsl_rng_free(r);
    calculate_init_position(person);
    return person;
}

float calculate_prob_death(int edad) { 
    for (int i = 0; i < 9; i++)
    {   
        if(edad>=10*i && edad<10*i+10) {
            return l_death_prob[i];
        }else if (i==8)
        {
            return l_death_prob[i];
        }
    }
    return 0.0;
}

int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;
    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1;
    }
    else
    {
        low_num = max_num + 1;
        hi_num = min_num;
    }
    srand(time(NULL));
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}


void calculate_init_position(person_t person)
{
    // TODO: Cambiar esto para que sea con el random_number
    person.coord[0] = posX; // coord x
    person.coord[1] = posY; // coord y
    posY++;
    if (posY==size_world)
    {
        posY = 0;
        posX++;
    }
}