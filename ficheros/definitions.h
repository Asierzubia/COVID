#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define SIZE_WORLD 30
#define PERCENT 0.05
#define MAX_SPEED 2
#define MAX_DIRECTION 8
#define MAX_INCUBATION 5
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.3 //0.6
#define MAX_DEATH 200
#define POPULATION_SIZE 10
#define BATCH 2
#define CUADRICULA 5
#define ITER 50
#define SEED 3


/*Estructura para la parametrizacion*/

typedef struct configuration_t {
    unsigned int world_size;
    unsigned int percent_t;
    unsigned int population_size;
    unsigned int batch;
    unsigned int iter_t;
    unsigned int seed;
} configuration_t;

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
    int id_global;
} person_t;

void create_person(int procesador);
//person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
void create_data_type(person_t *persona,MPI_Datatype *tipo);
void per_cicle();
void change_state(person_t person);
void propagate(person_t *person);
person_t *init_lists(int tamano);
void init_gsl();
void change_move_prob(person_t *person);
void change_infection_prob(person_t *person);
void init_world();
void move_person(person_t *person, int world_rank);
void move(person_t *person, coord *coord);
int move_visitor(person_t *person, coord *coord)
void calculate_init_position(person_t *person);
void print_world();
void print_person(person_t person,int procesador);
void calculate_metrics();
void init_person_parameters(person_t *persona,int estado,int id_local);
float calculate_ageMean();
float calculate_prob_death(int edad);
int vacunate(person_t person);
int random_number(int min_num, int max_num);
void realocate_lists();
int is_inside_world(int from, int direction, int to_node)
int search_node(int world_rank, int x, int y);
void print_person_especial(person_t p,int procesador);

#endif // DEFINITIONS_H