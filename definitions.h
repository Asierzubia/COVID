#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MAX_ARGS 256
#define MAX_FILE_LEN 4096
#define SIZE_WORLD 16
#define PERCENT 0.05
#define MAX_SPEED 2
#define MAX_DIRECTION 7
#define MAX_INCUBATION 5
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.0 //0.6
#define MAX_DEATH 200
#define POPULATION_SIZE 60
#define BATCH 2
#define CUADRICULA 5
#define ITER 1000
#define SEED 3
#define AGEMEAN 28
/*Estructuras*/

typedef struct configuration_t {
    int world_size;
    float percent;
    int population_size;
    int batch;
    int iter;
    int seed;
    int ageMean;
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

/*Variables*/

index_t **world;
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
person_move_t **l_person_moved;
person_move_t **lista_de_persona_mover;
int **lista_de_persona_mover_return;
coord_t **l_person_propagate;
coord_t *l_person_propagate_recive;
int *l_cont_node_move, *l_cont_node_propagate, *l_cont_persona_mover, *l_cont_person_return;
struct configuration_t conf;


int world_size, world_rank;
float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
int identificador_global;
int iter, posX, posY, i, j, k, position, seed, mu, alpha, beta;
int num_persons_to_vaccine, group_to_vaccine, when_change_group, person_vaccinned, radius, vaccines_left;
int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death, cont_healthy, cont_infected, cont_recovered, cont_move_visitor, cont_move_return, cont_propagate_visitor;
int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
int p_death, p_infected, p_recovered, p_healthy, p_RO;
int aux_death, aux_infected, aux_recovered, aux_healthy, aux_RO;
int PX, PY;
int buffer_total;
int buffer_node;
int quadrant_x;
int quadrant_y;
int contador, cont_propagate_recive;
int cont_iter; // ELIMINAR

float iter_healthy, iter_recovered, iter_infected, iter_death, iter_RO;

int **index_return_person;
int *cont_index_return;
float *l_mean_healthy, *l_mean_infected, *l_mean_recovered, *l_mean_death, *l_mean_R0;
float *recv_healthy, *recv_infected, *recv_recovered, *recv_death, *recv_R0;
char *l_positions, *l_positions_aux, *l_metrics_aux, *recv_positions;
float *recv_metrics;

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
void free_recv_metrics();
void free_all_lists();
void free_world();

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
void init_recv_metric_list();
void init_all_lists();

void propagate_arrived();
float calculate_prob_death(int edad);

void print_positions();
void save_positions(int world_rank, int iteration);
void print_metrics();
void save_metrics(int world_rank, int iteration);
void calculate_metrics();
void calculate_metric_mean();
char *init_list_archives(int size);

void Psend_return(int to_node);
void recive_return();

void init_index_list(int size_x,int size_y);
void init_person_return(int size_x,int size_y);

void move_returned();
void kill(int id, char *list);

void getAlphaBeta(int ageMean);
void set_configuration(int argc, char *argv[]);

void metrics();
#endif // DEFINITIONS_H
