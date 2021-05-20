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

gsl_rng *r;

float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
float mean_death, mean_infected, mean_recovered, mean_healthy, mean_RO;

int iter, posX, posY, i, j, k, position, seed, mu, alfa, beta;
int num_persons_to_vaccine, group_to_vaccine, person_vaccinned, radius, vaccines_left;
int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death, c_death_aux, c_healthy_aux, c_recovered_aux, c_infected_aux, RO;
int bach, cont_bach, num_bach;
int p_death, p_infected, p_recovered, p_healthy, p_RO;
int aux_death, aux_infected, aux_recovered, aux_healthy, aux_RO;

FILE *arch_metrics, *arch_positions;


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

index_t world[size_world][size_world];
person_t create_person();
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
char *l_positions, *l_metrics;
//int *l_count_healthy, *l_count_infected, *l_count_recovered, *l_count_death;

void per_cicle();
void change_state(person_t person);
void propagate(person_t *person);
void init_lists();
void init_gsl();
void create_population();
void change_move_prob(person_t *person);
void change_infection_prob(person_t *person);
void init_world();
void move(person_t *person);
void calculate_init_position(person_t *person);
void print_world();
void print_person(person_t person);
void calculate_metrics();
void print_metrics();
float calculate_ageMean();
float calculate_prob_death(int edad);
int vacunate(person_t person);
int random_number(int min_num, int max_num);
int calculate_age();
void free_structs();
void save_positions(int world_rank, int iteration);
#endif // DEFINITIONS_H