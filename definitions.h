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
#define POPULATION_SIZE 20
#define BATCH 2
#define CUADRICULA 5
#define ITER 10
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

void create_person(int procesador);
int random_number(int min_num, int max_num);
void print_person(person_t *p, int procesador,char lista);
void calculate_init_position(person_t *person);
person_t *init_lists(int pop);
int vacunate(person_t *person);
void change_move_prob(person_t *person);
void change_state(person_t *person);
void realocate_lists();
coord_t calculate_coord(int x, int y);
void print_lists(int world_rank);
void propagate(person_t *person);
void change_infection_prob(person_t *person);

void move_person(person_t *person, int world_rank);
void move(person_t *person, coord_t *coord);
int is_inside_world(int from, int direction, int to_node);
int search_node(int world_rank, int x, int y);

void free_move_list();
void free_prop_list();
void free_person_move();
void free_index_list();
void free_person_return();

void move_arrived();

void send_visitors(int flag,int flag_return);
void Psend(int to_node, int flag);
void recive(int flag);

void create_data_type_coord(coord_t *coordenadas);
void create_data_type_person(person_t *persona);
void create_data_type_person_move(coord_t *coordenadas, person_t *persona);

void init_world(int size_x, int size_y);
void init_gsl(int seed);
void init_person_parameters(person_t *persona, int state, int id_local);
void init_prop_list(int size_x, int size_y);
void init_move_list(int size_x, int size_y);
char *init_list_archives(int size);
void init_person_move_list(int size_x,int size_y);

void propagate_arrived();
float calculate_prob_death(int edad);

void print_positions();
void save_positions(int world_rank, int iteration);
void print_metrics();
void save_metrics(int world_rank, int iteration);
void calculate_metrics();
char *init_list_archives(int size);

void Psend_return(int to_node);
void recive_return();

void init_index_list(int size_x,int size_y);
void init_person_return(int size_x,int size_y);

void move_returned();
void kill(int id, char *list);

#endif // DEFINITIONS_H