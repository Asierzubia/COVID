#ifndef DEFINITIONS_H
#define DEFINITIONS_H


#define size_world 5
#define percent 0.1
#define MAX_SPEED 2     
#define MAX_DIRECTION 8 
#define MAX_INCUBATION 15 
#define MAX_RECOVERY 10
#define MAX_INFECTION 0.001 //0.6
#define MAX_DEATH 200
#define population_size 10


int iter;
int posX;
int posY;
float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002 , 0.004, 0.013, 0.036, 0.08, 0.148};
int i, j, k,position;
int seed;
int mu;
int alfa;
int beta;
gsl_rng *r;
int num_persons_to_vaccine;
int group_to_vaccine;
int person_vaccinned;
float age_mean;
int radius;
float prob_infection;
float recovery_period;
float prob_direction;
float prob_speed;
int id_contVaccined;
int idx_iter;

int id_contI;
int id_contNotI;
enum list {INFECTED, NOT_INFECTED, VACCINED};

typedef struct index {
    int id; // index
    enum list l; 
} index_t;

typedef struct person {
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
void per_cicle();
void change_state(person_t *person);
void propagate(person_t *person);
void init_lists();
void init_gsl();
float calculate_ageMean();
void create_population();
void change_move_prob(person_t *person);
void change_infection_prob(person_t *person);
void init_world();
float calculate_prob_death(int edad);
void move(person_t *person);
int vacunate(person_t person,int vaccines);
void calculate_init_position(person_t *person);
int random_number(int min_num, int max_num);
void print_world();

#endif // DEFINITIONS_H
