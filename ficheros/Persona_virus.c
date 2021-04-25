#include "../headers/person.h"

person_t create_person(){

    init_gsl();    
    prob_death = gsl_ran_beta(r, alfa, beta);
    age = (int )gsl_ran_beta(r, alfa, beta); //Cambiar
    state =(int)gsl_ran_beta(r, alfa, beta); //Cambiar porque tiene que estar entre 1 y 5
    periodo_incu = recovery_period; //Nuevo
    recovery = recovery_period; //Nuevo
    gsl_rng_free(r);
    calculate_init_position();

}

int random_number(int min_num, int max_num)
    {
        int result = 0, low_num = 0, hi_num = 0;

        if (min_num < max_num)
        {
            low_num = min_num;
            hi_num = max_num + 1; 
        } else {
            low_num = max_num + 1;
            hi_num = min_num;
        }

        srand(time(NULL));
        result = (rand() % (hi_num - low_num)) + low_num;
        return result;
    }

void calculate_init_position(){

    init_position = random_number(1,size);

}