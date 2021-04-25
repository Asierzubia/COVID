#include "../headers/Poblacion.h"
#include "../headers/gsl.h"

seed = 1;
mu = 100;
alfa = 2;
beta = 5;
size = 1000;
//Funciones
radius = 2; //Es algo que hemos decidido
//Inicializar la librería, el generador de números pseudoaleatorios y la semilla.

void init_gsl(){

    gsl_rng_env_setup();
    r = gsl_rng_alloc (gsl_rng_default);
    gsl_rng_set(r, seed);

}

void init_lista(){

    personList = (person *)malloc(size * sizeof(person));

}

void calculate_probs(){

    init_gsl();    
    prob_infection = gsl_ran_beta(r, alfa, beta);
    recovery_period = random_number(3, 5); //El periodo de contagio va a estar entre 3 y 5.
    prob_direction = gsl_ran_beta(r, alfa, beta);
    prob_speed = gsl_ran_beta(r, alfa, beta);
    gsl_rng_free(r);

}

float calculate_ageMean(){

    return (alfa / (alfa + beta)) * mu;

}

void create_population(){

    //Inicializo la lista
    init_lista();
    //Calculo las probabilidades y la edad media
    calculate_ageMean();
    calculate_probs();
    //Relleno la lista con personas
    for(i = 0; i < size; i++ ){
        person = createPerson();
        personList[i] = person;
    }
    

}

