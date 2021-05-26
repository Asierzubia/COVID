#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include "definitions.h"


/*Variables globales*/
index_t **world;
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
person_move_t **l_person_moved;
int *l_cont_node_move;

gsl_rng *r;

int world_size, world_rank;
float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
float mean_death, mean_infected, mean_recovered, mean_healthy, mean_RO;
int identificador_global;
int iter, posX, posY, i, j, k, position, seed, mu, alpha, beta;
int num_persons_to_vaccine, group_to_vaccine, when_change_group, person_vaccinned, radius, vaccines_left;
int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death, cont_move_visitor, cont_propagate_visitor;
int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
int p_death, p_infected, p_recovered, p_healthy, p_RO;
int aux_death, aux_infected, aux_recovered, aux_healthy, aux_RO;
int PX, PY;
int quadrant_x;
int quadrant_y;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /*Variables locales e inicializaciones*/

    int id_contIAux;
    int id_contNotIAux;
    int is_vaccined;
    int seed = world_rank * 10 * SEED;
    int population = 0;
    int cont_iterations = 0;
    cont_bach = 1;
    when_change_group = 5; 
    group_to_vaccine = 8;
    alpha = 2;
    beta = 5;
    mu = 100;
    id_contI = 0;
    id_contNotI = 0;
    id_contVaccined = 0;
    srand(seed);
    init_gsl(seed);

    if (world_rank == 0)
    {
        quadrant_x = SIZE_WORLD / (int)floor(sqrt(world_size));
        quadrant_y = SIZE_WORLD / (int)ceil(sqrt(world_size));
        population = round(POPULATION_SIZE / world_size);
        num_persons_to_vaccine = round(POPULATION_SIZE * PERCENT);
    }

    MPI_Bcast(&population, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_persons_to_vaccine, 1, MPI_INT, 0, MPI_COMM_WORLD);

    PX=quadrant_x*(world_rank%(int)round(sqrt(world_size)));
	PY=quadrant_y*(world_rank/(int)round(sqrt(world_size)));

    printf("P%d | quadrant X -> %d, quadrant y -> %d | Population %d\n",world_rank,quadrant_x,quadrant_y,population);

    identificador_global = world_rank * population;

    init_world(quadrant_x, quadrant_y);
    l_person_infected = init_lists((quadrant_x*quadrant_y)*10*BATCH);
    l_person_notinfected = init_lists((quadrant_x*quadrant_y)*10*BATCH);
    l_vaccined = init_lists((quadrant_x*quadrant_y)*10*BATCH);
    l_cont_node_move = malloc(world_size*sizeof(int));
    for(i = 0; i < world_size;i++) {
        l_cont_node_move[i] = 0;
    }

    int to = population;
    if(population > (quadrant_y*quadrant_x)) {
        printf("Las personas asignadas a cada nodo superar el limite de huecos del mundo, por lo que se va a generar tantas personas como hueco tenga el mundo, en este caso -> %d",quadrant_y*quadrant_x);
        to = quadrant_y*quadrant_x;
    }
    for (i = 0; i < to; i++) {
        printf("P%d Valor i -> %d\n",world_rank,i);
        create_person(world_rank);
    }
    

    for (k = 0; k < ITER; k++) // ITERACCIONES
    {
        init_move_list(world_size,quadrant_x*quadrant_y);
        if (cont_iterations >= when_change_group)
        {
            group_to_vaccine--;
            cont_iterations = 0;
        }
        else
        {
            cont_iterations++;
        }

        vaccines_left = num_persons_to_vaccine;
        id_contIAux = id_contI;
        id_contNotIAux = id_contNotI;

        for (i = 0; i < id_contIAux; i++) // INFECTED
        {
            if (l_person_infected[i].id != -1)
            {
                change_state(&l_person_infected[i]);
                if (l_person_infected[i].id != -1) // Ha cambiado de estado y no se mueve
                {
                    change_move_prob(&l_person_infected[i]);
                    move_person(&l_person_infected[i], world_rank);
                }
            }
        }

        for (i = 0; i < id_contVaccined; i++) // VACCINED
        {
            change_move_prob(&l_vaccined[i]);
            move_person(&l_vaccined[i], world_rank);
        }

        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED
        {
            if (l_person_notinfected[i].id != -1)
            {
                if (vaccines_left > 0)
                {
                    if(l_person_notinfected[i].id != i) printf("El id y la posicion en la que se encuentra en la lista no es la misma\n");
                    is_vaccined = vacunate(&l_person_notinfected[i]);
                    if (is_vaccined == 0) // NOT-VACCINED
                    {
                        change_move_prob(&l_person_notinfected[i]);
                        move_person(&l_person_notinfected[i], world_rank);
                    }
                }
                else
                {
                    change_move_prob(&l_person_notinfected[i]);
                    move_person(&l_person_notinfected[i], world_rank);
                }
            }
        }

        free_move_list();

        for(i = 0; i < world_size;i++) l_cont_node_move[i] = 0;
        
        if (cont_bach == BATCH)
        {
            cont_bach = 1;
            //save_metrics(world_rank, k);
            //save_positions(world_rank, k);
            realocate_lists();

        }
        else
        {
            cont_bach++;
        }
    }

    

    free(l_person_infected);
    free(l_person_notinfected);
    free(l_vaccined);

    printf("Finaliza la simulacion\n");

    MPI_Finalize();
    exit(0);
    return(0);
}

void print_lists(int world_rank){

    printf("---INFECTADOS-----\n");
    for(i = 0;i<id_contI;i++){
        print_person(&l_person_infected[i],world_rank,'i');
    }
    printf("---NO INFECTADOS-----\n");
    for(i = 0;i<id_contNotI;i++){
        print_person(&l_person_notinfected[i],world_rank,'n');
    }
    printf("---VACUNADOS-----\n");
    for(i = 0;i<id_contVaccined;i++){
        print_person(&l_vaccined[i],world_rank,'v');
    }
    
}


int search_node(int world_rank, int x, int y)
{
    int to_node = -1;
    int direction = -1;
    if (x >= quadrant_x && y < quadrant_y && y >= 0) // DERECHA
    {
        to_node = world_rank + 1;
        direction = 1;
    }
    else if (x < 0 && y < quadrant_y && y >= 0) // IZQUIERDA
    {
        to_node = world_rank - 1;
        direction = 5;
    }
    else if (x >= quadrant_x && y >= quadrant_y) // DIAGONAL DERECHA ARRIBA
    {
        to_node = (world_rank + 1) + (SIZE_WORLD / quadrant_y);
        direction = 8;
    }
    else if (y >= quadrant_y && x < 0) // DIAGONAL IZQUIERDA ARRIBA
    {
        to_node = (world_rank - 1) + (SIZE_WORLD / quadrant_y);
        direction = 6;
    }
    else if (y >= quadrant_y && x < quadrant_x && x >= 0) // ARRIBA (+ DIAGONAL SIN CAMBIAR)
    {
        to_node = world_rank + (SIZE_WORLD / quadrant_y);
        direction = 7;
    }
    else if (x >= quadrant_x && y < 0) // DIAGONAL DERECHA ABAJO
    {
        to_node = (world_rank + 1) - (SIZE_WORLD / quadrant_y);
        direction = 2;
    }
    else if (y < 0 && x < 0) // DIAGONAL IZQUIERDA ABAJO
    {
        to_node = (world_rank - 1) - (SIZE_WORLD / quadrant_y);
        direction = 4;
    }
    else if (y < 0 && x < quadrant_x && x >= 0) // ABAJO (+ DIAGONAL SIN CAMBIAR)
    {
        to_node = world_rank - (SIZE_WORLD / quadrant_y);
        direction = 3;
    }

    // Comprobar que no se sale
    int sol = is_inside_world(world_rank,direction,to_node);
    if(sol==0) {
        return -1;
    }else{
        return to_node;
    }

}

int is_inside_world(int from, int direction, int to_node)
{
    switch (direction)
    {
    case 1:
        if (from % (SIZE_WORLD / quadrant_x) == ((SIZE_WORLD / quadrant_x) - 1))
        {
            return 0;
        }
        break;
    case 2:
        if (from % (SIZE_WORLD / quadrant_x) == ((SIZE_WORLD / quadrant_x) - 1) || from < (SIZE_WORLD / quadrant_x))
        {
            return 0;
        }
        break;
    case 3:
        if (from < (SIZE_WORLD / quadrant_x))
        {
            return 0;
        }
        break;
    case 4:
        if (from % (SIZE_WORLD / quadrant_x) == 0 || from > quadrant_x)
        {
            return 0;
        }
        break;
    case 5:
        if (from % (SIZE_WORLD / quadrant_x) == 0)
        {
            return 0;
        }
        break;
    case 6:
        if (from % (SIZE_WORLD / quadrant_x) == 0 || to_node >= world_size)
        {
            return 0;
        }
        break;
    case 7:
        if (to_node >= world_size)
        {
            return 0;
        }
        break;
    case 8:
        if (from % (SIZE_WORLD / quadrant_x) == ((SIZE_WORLD / quadrant_x) - 1) || to_node >= world_size)
        {
            return 0;
        }
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

coord_t calculate_coord(int x, int y) {
    coord_t coord;
    coord.x = x;
    coord.y = y;
    if (x<0)
    {
        coord.x = quadrant_x + x;
    }else if (x>=quadrant_x)
    {
        coord.x = x - quadrant_x;
    }

    if (y<0)
    {
        coord.y = quadrant_y + y;
    }else if (y>=quadrant_y)
    {
        coord.y = y - quadrant_y;
    }

    return coord;
}

void move(person_t *person, coord_t *coord)
{
    world[coord->x][coord->y].id = person->id;
    if (person->state == 1 || person->state == 2)
    {
        world[coord->x][coord->y].l = INFECTED;
    }
    else if (person->state == 0)
    {
        world[coord->x][coord->y].l = NOT_INFECTED;
    }
    else
    {
        world[coord->x][coord->y].l = VACCINED;
    }
    person->coord.x = coord->x;
    person->coord.y = coord->y;
}

void move_person(person_t *person, int world_rank)
{
    int x = person->coord.x;
    int y = person->coord.y;
    int nx, ny;
    int speed = person->speed[1];
    if (speed != 0)
    {
        int direction = person->speed[0];
        int diagonals[4][2] = {{1, -1}, {-1, -1}, {-1, 1}, {1, 1}};
        int directions[8][2] = {{1,0},{2,0},{0,-1},{0,-2},{-1,0},{-2,0},{0,1},{0,2}};

        if (direction == 2 || direction == 4 || direction == 6 || direction == 8) // DIAGONAL
        {
            x += diagonals[(direction / 2)-1][0];
            y += diagonals[(direction / 2)-1][1];
        }
        else
        {
            x += directions[direction - 1 + speed - 1][0];
            y += directions[direction - 1 + speed - 1][1];
        }
        coord_t coord;
        coord.x = x;
        coord.y = y;
        if (x < quadrant_x && y < quadrant_y )  // MISMO QUADRANTE
        {
            if(x >= 0 && y >= 0){
                //printf("X -> %d | Y -> %d\n",x,y);
                if (world[x][y].id == -1) // EMPTY POSITION
                {
                    //printf("Mismo cuadrante X-> %d, Y->%d\n",x,y);
                    world[person->coord.x][person->coord.y].id = -1; // Se elimina la anterior pos
                    move(person, &coord);
                }
            }
        }
        else
        {
            int to_node = search_node(world_rank, x, y);
            if(to_node != -1) // SE PUEDE
            {
                coord = calculate_coord(x,y);
                printf("To Node -> %d\n",to_node);
                if(to_node < world_size && to_node>=0){
                    printf("ID move list -> %d\n",l_person_moved[to_node][l_cont_node_move[to_node]].person.id);
                }
                //memcpy(&l_person_moved[to_node][l_cont_node_move[to_node]].person,person,sizeof(person_t));
                //memcpy(&l_person_moved[to_node][l_cont_node_move[to_node]].coord,&coord,sizeof(coord_t));
                // l_cont_node_move[to_node] = l_cont_node_move[to_node] + 1;
            }
        }
    }
}


void change_move_prob(person_t *person)
{
    person->speed[0] = random_number(1, MAX_DIRECTION);
    person->speed[1] = random_number(0, MAX_SPEED);
}

void change_state(person_t *person) // 1(INFECCIOSO) and 2(NO-INFECCIOSO) States
{
    if (person->incubation_period > 0)
    {
        person->incubation_period--;
    }
    else
    {
        if (person->recovery == 0)
        {
            //printf("La persona con id-global %d ha cambiado de estado\n",person->id_global);
            person->state = 3;
            person->incubation_period = random_number(3, 5);
            person->recovery = random_number(3, 5);
            person->id = id_contVaccined;
            memcpy(&l_vaccined[id_contVaccined],person,sizeof(person_t));
            world[person->coord.x][person->coord.y].l = VACCINED;
            world[person->coord.x][person->coord.y].id = id_contVaccined;
            person->id = -1;
            id_contVaccined++;
        }
        else
        {
            person->recovery--;
            if (person->state == 2)
            {
                //propagate(&person);
            }
        }
    }
}

int vacunate(person_t *person)
{
    if (person->age >= group_to_vaccine * 10)
    {
        printf(">>>>>%d VACUNADO!\n", person->id_global);
        person->state = 4;
        person->id = id_contVaccined;
        memcpy(&l_vaccined[id_contVaccined],person,sizeof(person_t));
        person->id = -1;
        world[person->coord.x][person->coord.y].l = VACCINED;
        world[person->coord.x][person->coord.y].id = id_contVaccined;
        id_contVaccined++;
        return 1;
    }
    else
    {
        return 0; // no vacunado
    }
}

person_t *init_lists(int pop)
{
    person_t *result = malloc(BATCH * pop * sizeof(person_t));
    return result;
}

void init_world(int size_x, int size_y)
{
    world = malloc(size_x * sizeof(index_t *));
    if (world == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        world[i] = malloc(size_y * sizeof(index_t));
        if (world[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            world[i][j].id = -1;
        }
    }
}

void print_person(person_t *p, int procesador,char lista)
{
    printf("LISTA -> %c | Procedador %d Ha creado la persona => ID_Global:%d|ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]|Speed[%d,%d]\n",
           lista,procesador, p->id_global, p->id, p->state, p->age, p->incubation_period, p->prob_infection, p->recovery, p->coord.x, p->coord.y, p->speed[0], p->speed[1]);
}

void init_gsl(int seed)
{
    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);
    gsl_rng_set(r, seed);
}

int random_number(int min_num, int max_num)
{
    return rand() % (max_num + 1) + min_num;
}

void calculate_init_position(person_t *person)
{
    person->coord.x = posX; // coord x
    person->coord.y = posY; // coord y
    posY++;
    if (posY == quadrant_y)
    {
        posY = 0;
        posX++;
    }
}

void init_person_parameters(person_t *persona, int state, int id_local)
{
    persona->id = id_local;
    persona->age = random_number(1, 100);
    persona->prob_infection = gsl_ran_beta(r, alpha, beta);
    persona->state = state;
    persona->incubation_period = random_number(0, MAX_INCUBATION);
    persona->recovery = random_number(0, MAX_RECOVERY);
    persona->id_global = identificador_global;
    persona->speed[0] = random_number(1, MAX_DIRECTION);
    persona->speed[1] = random_number(0, MAX_SPEED);
    world[persona->coord.x][persona->coord.y].id = id_local;
}

void create_person(int procesador)
{

    int state = random_number(0, 2);
    if (state == 0 || state == 3) // SANO
    {
        calculate_init_position(&l_person_notinfected[id_contNotI]);
        init_person_parameters(&l_person_notinfected[id_contNotI], state, id_contNotI);
        world[l_person_notinfected[id_contNotI].coord.x][l_person_notinfected[id_contNotI].coord.y].l = NOT_INFECTED;
        id_contNotI++;
    }
    else // INFECTADO
    {
        calculate_init_position(&l_person_infected[id_contI]);
        init_person_parameters(&l_person_infected[id_contI], state, id_contI);
        world[l_person_infected[id_contI].coord.x][l_person_infected[id_contI].coord.y].l = INFECTED;
        id_contI++;
    }

    identificador_global++;
}

void print_world(int worldRank){

    for(i = 0; i < quadrant_x; i++){
        for(j = 0; j < quadrant_y; j++){
            printf("El P%d inicializada el mundo, posicion | X->%d  | Y->%d  |\n",worldRank,i,j);
        }
    }

}


void realocate_lists()
{

    int last_value = id_contI;
    for (i = 0; i < id_contI; i++)
    { // INFECTED

        if (l_person_infected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contI; j++)
            {
                if (l_person_infected[j].id != -1)
                {
                    last_value = i+1;
                    memcpy(&l_person_infected[i], &l_person_infected[j], sizeof(person_t));
                    l_person_infected[i].id = i;
                    l_person_infected[j].id = -1;
                    world[l_person_infected[i].coord.x][l_person_infected[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
    id_contI = last_value;
    last_value = id_contNotI;
    for (i = 0; i < id_contNotI; i++)
    { // INFECTED
        if (l_person_notinfected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contNotI; j++)
            {
                if (l_person_notinfected[j].id != -1 && j < id_contNotI)
                {
                    last_value = i+1;
                    memcpy(&l_person_notinfected[i], &l_person_notinfected[j], sizeof(person_t));
                    l_person_notinfected[i].id = i;
                    l_person_notinfected[j].id = -1;
                    world[l_person_notinfected[i].coord.x][l_person_notinfected[i].coord.y].id = i;
                    break;
                }
            }
            
        }
    }
    //printf("ID CONT NOT I -> %d\n",last_value);
    id_contNotI = last_value;
    last_value = id_contVaccined;
    for (i = 0; i < id_contVaccined; i++)
    { // INFECTED
        if (l_vaccined[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contVaccined; j++)
            {
                if (l_vaccined[j].id != -1)
                {
                    last_value = i+1;
                    memcpy(&l_vaccined[i], &l_vaccined[j], sizeof(person_t));
                    l_vaccined[i].id = i;
                    l_vaccined[j].id = -1;
                    world[l_vaccined[i].coord.x][l_vaccined[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
    id_contVaccined = last_value;
}

void free_move_list(){

    int o;
    for(o = 0;o<world_size;o++) {
        free(l_person_moved[o]);
    }
    free(l_person_moved);

}
void init_move_list(int size_x, int size_y)
{
    l_person_moved = malloc(size_x * sizeof(person_move_t *));
    if (l_person_moved == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int l,n;
    for (l = 0; l < size_x; l++)
    {
        l_person_moved[l] = malloc(size_y * sizeof(person_move_t));
        if (l_person_moved[l] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (n = 0; n < size_y; n++)
        {
            l_person_moved[l][n].person.id = -1;
        }
    }
    
}
