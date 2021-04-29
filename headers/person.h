#ifndef PERSON_H
#define PERSON_H

typedef struct person {
    int age;
    int state;
    float prob_death;
    int coord[2];
    int speed[2];
    int incubation_period; 
    int recovery;
    int id;
} person_t;

float l_death_prob[9];

int posX = 0;
int posY = 0;
int id_contI, id_contNotI;

person_t create_person();
void calculate_init_position();
int random_number(int min_num, int max_num);

#endif // PERSON_H