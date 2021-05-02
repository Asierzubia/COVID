#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#define size_world 10 // un lado
#define iterations 1000 

int idx_iter;
enum list {INFECTED, NOT_INFECTED, VACCINED};

typedef struct index {
    int id; // index
    enum list l; 
} index_t;

index_t *world;

void init_world();
void move(person_t *person);

#endif // ENVIRONMENT_H