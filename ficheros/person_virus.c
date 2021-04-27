#include "person.h"
#include "probability.h"
#include "gsl/gsl/gsl_randist.h"
#include "gsl/gsl/gsl_rng.h"

person_t create_person(){
    person_t person;
    init_gsl();    
    person.prob_death = gsl_ran_beta(r, alfa, beta);
    person.age = (int )gsl_ran_beta(r, alfa, beta); //Cambiar
    person.state =(int)gsl_ran_beta(r, alfa, beta); //Cambiar porque tiene que estar entre 1 y 5
    person.periodo_incu = recovery_period; //Nuevo
    person.recovery = recovery_period; //Nuevo
    gsl_rng_free(r);
    calculate_init_position();
    return person;
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