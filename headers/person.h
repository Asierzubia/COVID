#ifndef PERSON_H
#define PERSON_H

typedef struct person {
    
    int age;
    int state;
    float prob_death;
    int coord[2];
    int speed[2];
    int init_position; //Nuevo
    int periodo_incu; //Nuevo
    int recovery;

} person_t;

person_t create_person();
void calculate_init_position();
int random_number(int min_num, int max_num);

#endif // PERSON_H