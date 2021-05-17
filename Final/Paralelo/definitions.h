#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define size_world 10
#define percent 0.1
#define MAX_SPEED 2
#define MAX_DIRECTION 8
#define MAX_INCUBATION 5
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.3 //0.6
#define MAX_DEATH 200
#define POPULATION_SIZE 10
#define BATCH 2
#define CUADRICULA 5

//gsl_rng *r;

enum list
{
    INFECTED,
    NOT_INFECTED,
    VACCINED
};

typedef struct index
{
    int id; // index
    enum list l;
} index_t;

typedef struct person
{
    int age;
    int state;
    float prob_infection;
    int coord[2];
    int speed[2]; // 0 direction 1 speed
    int incubation_period;
    int recovery;
    int id;
} person_t;

void create_person(person_t *l_notInfected_p,person_t *l_infected_p, int procesador);
//person_t *l_person_infected, *l_person_notinfected, *l_vaccined;

void per_cicle();
void change_state(person_t person);
void propagate(person_t *person);
person_t *init_lists(int tamano);
void init_gsl();
void create_population(person_t *l_notInfected_p,person_t *l_infected_p, int procesador);
void change_move_prob(person_t *person);
void change_infection_prob(person_t *person);
void init_world();
void move(person_t *person);
void calculate_init_position(person_t *person);
void print_world();
void print_person(person_t person,int procesador);
void calculate_metrics();

float calculate_ageMean();
float calculate_prob_death(int edad);
int vacunate(person_t person);
int random_number(int min_num, int max_num);

#endif // DEFINITIONS_H
