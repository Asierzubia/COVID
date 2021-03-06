#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define SIZE_WORLD 8
#define PERCENT 0.05
#define MAX_SPEED 2
#define MAX_DIRECTION 7
#define MAX_INCUBATION 5
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.3 //0.6
#define MAX_DEATH 200
#define POPULATION_SIZE 60
#define BATCH 2
#define CUADRICULA 5
#define ITER 11
#define SEED 3

/*Estructuras*/

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

typedef struct coord
{
    int x;
    int y;
} coord_t;

typedef struct person
{
    int age;
    int state;
    float prob_infection;
    coord_t coord;
    int speed[2]; // 0 direction 1 speed
    int incubation_period;
    int recovery;
    int id;
    int id_global;
} person_t;

typedef struct person_move
{
    person_t person;
    coord_t coord;
} person_move_t;

/*Funciones*/

void print_world(int worldRank);
void init_world(int size_x, int size_y);
void init_gsl(int seed);
void create_person(int procesador);
int random_number(int min_num, int max_num);
void print_person(person_t *p, int procesador,char lista);
void calculate_init_position(person_t *person);
void init_person_parameters(person_t *persona, int state, int id_local);
person_t *init_lists(int pop);
int vacunate(person_t *person);
void change_move_prob(person_t *person);
void change_state(person_t *person);
void realocate_lists();
coord_t calculate_coord(int x, int y);
void print_lists(int world_rank);

void move_person(person_t *person, int world_rank);
void move(person_t *person, coord_t *coord);
int is_inside_world(int from, int direction, int to_node);
int search_node(int world_rank, int x, int y);

void init_move_list(int size_x, int size_y);
void free_move_list();
#endif // DEFINITIONS_H