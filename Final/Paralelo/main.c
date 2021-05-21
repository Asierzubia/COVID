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

float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
float mean_death, mean_infected, mean_recovered, mean_healthy, mean_RO;
int identificador_global;
int iter, posX, posY, i, j, k, position, seed, mu, alpha, beta;
int num_persons_to_vaccine, group_to_vaccine, when_change_group, person_vaccinned, radius, vaccines_left;
int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death, cont_move_visitor, cont_propagate_visitor;
int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
int p_death, p_infected, p_recovered, p_healthy, p_RO;
index_t **world;
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
person_move_t **l_person_moved;
coord_t *l_person_propagate;
coord_t *l_person_propagate_recive;
int *l_cont_node;
int PX, PY;
int quadrant_x = 0;
int quadrant_y = 0;
gsl_rng *r;
int world_size, world_rank;
person_move_t *lista_de_persona_mover;


MPI_Datatype coord_type;
MPI_Datatype person_type;
MPI_Datatype person_move;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //----------FALTA PARAMETRIZACION--------------------------
    //Inicializacion de variables
    int id = 0;
    int seed = world_rank * 10 * SEED;
    int population = 0;
    int quadrant = 2;
    when_change_group = 5; //Para saber cada cuantas iteraciones tengo que cambiar de grupo
    int cont_iterations = 0;
    int id_contIAux;
    int id_contNotIAux;
    int is_vaccined;
    posX = 0;
    posY = 0;
    bach = 2;
    cont_bach = 1;
    id_contI = 0;
    id_contNotI = 0;
    id_contVaccined = 0;
    group_to_vaccine = 8;
    alpha = 2;
    beta = 5;
    mu = 100;
    cont_propagate_visitor = 0;
    cont_move_visitor = 0;


    srand(seed); //Falta parametrizarlo para que en cada ejecucion sea diferente
    init_gsl(seed);

    if (world_rank == 0)
    {
        quadrant_x = SIZE_WORLD / (int)floor(sqrt(world_size));
        quadrant_y = SIZE_WORLD / (int)ceil(sqrt(world_size));
        population = round(POPULATION_SIZE / world_size);
        num_persons_to_vaccine = round(POPULATION_SIZE * PERCENT);
    }
    //INICIALIZACION DE FICHEROS	posic = MPI_File_open( MPI_COMM_WORLD, "historialposic.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &posiFile);
    FILE *metricas;
    metricas = fopen("metricas.txt", "w+");
    int pos;
    MPI_Offset position_metrics_offset;
    MPI_File metrics_positions;
    MPI_Status positionStatus;
    pos = MPI_File_open(MPI_COMM_WORLD, "PersonPositions.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &metrics_positions);
    //FINALIZACION DE INICIALIZACION DE FICHEROS

    MPI_Bcast(&population, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_persons_to_vaccine, 1, MPI_INT, 0, MPI_COMM_WORLD);
	PX=quadrant_x*(world_rank%(int)round(sqrt(world_size)));
	PY=quadrant_y*(world_rank/(int)round(sqrt(world_size)));
    printf("quadrant X -> %d, quadrant y -> %d\n",quadrant_x,quadrant_y);

    identificador_global = world_rank * population;
    init_world(quadrant_x, quadrant_y);
    //Creo los DataTypes

    l_cont_node = malloc(world_size*sizeof(int));
    for(i = 0 ; i<world_size;i++) {
        l_cont_node[i] = 0;
    }

    //Ficticio
    person_t persona_virtual;
    persona_virtual.id = id_contNotI;
    persona_virtual.age = random_number(1, 100);
    persona_virtual.prob_infection = gsl_ran_beta(r, alpha, beta);
    persona_virtual.state = NOT_INFECTED;
    persona_virtual.incubation_period = random_number(0, MAX_INCUBATION);
    persona_virtual.recovery = random_number(0, MAX_RECOVERY);
    persona_virtual.id_global = identificador_global;
    persona_virtual.speed[0] = 1;
    persona_virtual.speed[1] = 0;
    coord_t cood_virtual;
    cood_virtual.x = 1;
    cood_virtual.y = 0;
    persona_virtual.coord = cood_virtual;

    create_data_type_coord(&cood_virtual);
    create_data_type_person(&persona_virtual);
    create_data_type_person_move(&cood_virtual, &persona_virtual);
    //create_data_type_person_prop(,&person_prop);

    //Inicializar listas
    l_person_infected = init_lists(population);
    l_person_notinfected = init_lists(population);
    l_vaccined = init_lists(population);

    //Crear la poblacion
    for (i = 0; i < population; i++)
    {
        create_person(world_rank);
    }
    //Ahora que ya tenemos creado las listas,creadas las personas y el mundo empezamos las iteraciones
    init_move_list(world_size,quadrant_x*quadrant_y);

    for (k = 0; k < ITER; k++) // ITERACCIONES
    {
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
                //change_state(l_person_infected[i]);
                if (l_person_infected[i].id != -1) // Ha cambiado de estado y no se mueve
                {
                    change_move_prob(&l_person_infected[i]);
                    move_person(&l_person_infected[i], world_rank);
                    //move(&l_person_infected[i]);
                }
            }
        }

        for (i = 0; i < id_contVaccined; i++) // VACCINED
        {
            change_move_prob(&l_vaccined[i]);
            move_person(&l_vaccined[i], world_rank);
            //move(&l_vaccined[i]);
        }

        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED
        {
            if (l_person_notinfected[i].id != -1)
            {
                if (vaccines_left > 0)
                {
                    is_vaccined = vacunate(l_person_notinfected[i]);
                    if (is_vaccined == 0) // NOT-VACCINED
                    {
                        change_move_prob(&l_person_notinfected[i]);
                        move_person(&l_person_notinfected[i], world_rank);
                        //move(&l_person_notinfected[i]);
                    }
                }
                else
                {
                    change_move_prob(&l_person_notinfected[i]);
                    move_person(&l_person_notinfected[i], world_rank);
                    //move(&l_person_notinfected[i]);
                }
            }
        }

        //MPI_Barrier(MPI_COMM_WORLD);
        //send_visitors(0,&coord_type,&person_move);        
        //free(l_person_propagate_recive);
        // MPI_Barrier(MPI_COMM_WORLD);
        // send_visitors(1,&coord_type,&person_move);
        //move_arrived();
        //propagate_arrived();        
        //free(lista_de_persona_mover);

        //Momento de mirar las personas que se han infectado o movido a cuadrantes de otro procesador
        if (cont_bach == BATCH)
        {
            cont_bach = 1;
            //calculate_metrics();
            //realocate_lists();
        }
        else
        {
            cont_bach++;
        }
    }

    //Solo imprimir las metricas al final de la ejecucion
    if (world_rank == 0)
    {
        //save_metrics();
    }

    //Hacer free de las listas
    free(l_person_infected);
    free(l_person_notinfected);
    free(l_vaccined);
    free(l_person_moved);
    free(l_person_propagate);

    printf("Finaliza la simulacion\n");
    //fclose(metricas);
    MPI_Finalize();
    exit(0);
    return 0;
}

void move_arrived(){

    for(i = 0; i< cont_move_visitor;i++){
        if(world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].id == -1){ //Esta libre el sitio
            //Lo meto en la lista correspondiente en base a si es infectado vacunado o no infectado
            if(lista_de_persona_mover[i].person.state == 0 || lista_de_persona_mover[i].person.state == 3){ //No esta infectado
                memcpy(&l_person_notinfected[id_contNotI],&lista_de_persona_mover[i].person,sizeof(person_t));
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].l = NOT_INFECTED;
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].id = id_contNotI;
                id_contNotI++;
            }
            else if(lista_de_persona_mover[i].person.state == 1 || lista_de_persona_mover[i].person.state == 2){ //Esta infectado
                memcpy(&l_person_infected[id_contI],&lista_de_persona_mover[i].person,sizeof(person_t));
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].l = INFECTED;
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].id = id_contI;
                id_contI++;
            }else{ //Esta Vacunado
                memcpy(&l_vaccined[id_contVaccined],&lista_de_persona_mover[i].person,sizeof(person_t));
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].l = VACCINED;
                world[lista_de_persona_mover[i].coord.x][lista_de_persona_mover[i].coord.y].id = id_contVaccined;
                id_contVaccined++;
            }
        }    
    }
}

void propagate_arrived(){

    for(i = 0;i<cont_propagate_visitor;i++){
        if(world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].x].id != -1){ //Hay alguien, por lo que la infecto
            
            if(world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].x].l == NOT_INFECTED){ //Solo lo recorro si esta en no infectados
                int id = world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].x].id;
                if (l_person_notinfected[id].prob_infection > MAX_INFECTION) // Se infecta
                {
                    int prob_aux = 1000 * calculate_prob_death(l_person_notinfected[id].age);
                    if (random_number(0, MAX_DEATH) <= prob_aux) // MUERE
                    {
                        printf(">>>>>%d HA MUERTO!\n", l_person_notinfected[id].id_global);
                        l_person_notinfected[id].state = 5;
                        world[l_person_notinfected[id].coord.x][l_person_notinfected[id].coord.y].id = -1;
                        cont_death++;
                    }
                    else
                    {
                        printf(">>>>>%d SE HA INFECTADO!\n", l_person_notinfected[id].id_global);
                        if (random_number(0, 1) == 0) // INFECCIOSO
                        {
                            l_person_notinfected[id].state = 2;
                        }
                        else
                        {
                            l_person_notinfected[id].state = 1;
                        }
                        l_person_notinfected[id].id = id_contI;
                        memcpy(&l_person_infected[id_contI],&l_person_notinfected[id],sizeof(person_t));
                        world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].l = INFECTED;
                        world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].id = id_contI;
                        //Falta los realocates
                        l_person_notinfected[id].id = -1;
                        id_contI++;
                    }
                }
            }
            //Lo pongo en la posicion correspondiente del world el valor de INFECTED        
        }
    }
}

void send_visitors(int flag,MPI_Datatype *coord_type, MPI_Datatype *person_move) {
    //printf("Ha entrado en Send_visitors\n");
    int nodo_a_enviar,contador;
    // 1-> Derecha
    nodo_a_enviar= world_rank+1;
    flag = 0;
    if(PX + quadrant_x != SIZE_WORLD){ //Se pasa de la derecha
        //printf("El P%d entrado en DERECHA ENVIAR --> PX : %d \n",world_rank,PX);
        Psend(nodo_a_enviar,flag);
    } 
    if(PX != 0){
        //printf("El P%d entrado en DERECHA RECIVIR\n",world_rank);
        recive(flag);
    }

    // 2-> Arriba Derecha
    nodo_a_enviar = world_rank - ((SIZE_WORLD/quadrant_x)-1);
    if( PX + quadrant_x != SIZE_WORLD && PY - quadrant_y >= 0){
        //printf("El P%d entrado en ARRIBA DERECHA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PX != 0 && PY + quadrant_y != SIZE_WORLD){
        //printf("El P%d entrado en ARRIBA DERECHA RECIVIR\n",world_rank);
        recive(flag);
    }
    //3 -> Arriba
    nodo_a_enviar = world_rank - (SIZE_WORLD/quadrant_x);
    if(PY - quadrant_y >= 0){
        //printf("El P%d entrado en ARRIBA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PY + quadrant_y != SIZE_WORLD){
        //printf("El P%d entrado en ARRIBA RECIVIR\n",world_rank);
        recive(flag);
    }
    //4 -> Arriba Izquierda
    nodo_a_enviar = world_rank - ((SIZE_WORLD/quadrant_x)+1);
    if(PY - quadrant_y >= 0 && PX - quadrant_x >= 0 ){
        //printf("El P%d entrado en ARRIBA IZQUIERDA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PX + quadrant_x != SIZE_WORLD && PY + quadrant_y != SIZE_WORLD ){
        //printf("El P%d entrado en ARRIBA IZQUIERDA RECIVIR\n",world_rank);
        recive(flag);
    }
    //5 -> IZQUIERDA
    nodo_a_enviar = world_rank-1;
    if(PX != 0){
        //printf("El P%d entrado en IZQUIERDA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PX + quadrant_x != SIZE_WORLD){
        //printf("El P%d entrado en IZQUIERDA RECIVIR\n",world_rank);
        recive(flag);
    }
    //6 -> ABAJO IZQUIERDA
    nodo_a_enviar = world_rank+(SIZE_WORLD/quadrant_x) -1;
    if(PY + quadrant_y != SIZE_WORLD && PX != 0){
        //printf("El P%d entrado en ABAJO IZQUIERDA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PY != 0 && PX + quadrant_x != SIZE_WORLD){
        //printf("El P%d entrado en ABAJO IZQUIERDA RECIBIR\n",world_rank);
        recive(flag);
    }
    //7 -> ABAJO
    nodo_a_enviar = world_rank+(SIZE_WORLD/quadrant_x);
    if(PY + quadrant_y != SIZE_WORLD){
        //printf("El P%d entrado en ABAJO ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PY != 0){
        //printf("El P%d entrado en ABAJO RECIBIR\n",world_rank);
        recive(flag);
    }

    // 8 -> ABAJO DERECHA
    nodo_a_enviar = world_rank + (SIZE_WORLD/quadrant_x) + 1;
    if(PX + quadrant_x != SIZE_WORLD && PY+quadrant_y != SIZE_WORLD){
        printf("El P%d entrado en ABAJO DERECHA ENVIAR\n",world_rank);
        Psend(nodo_a_enviar,flag);
    } 
    if(PY != 0 && PX !=0){
        printf("El P%d entrado en ABAJO DERECHA RECIBIR\n",world_rank);
        recive(flag);
    }
}

void Psend(int to_node, int flag){
    
    
    if(flag == 0){
        printf("Nodo ");
        MPI_Send(&l_cont_node[to_node],1,MPI_INT,to_node,0, MPI_COMM_WORLD);
        printf("EL P%d ha enviado %d al P%d\n",world_rank,l_cont_node[to_node],to_node);
        MPI_Send(&l_person_moved[to_node],l_cont_node[to_node],person_move,to_node,0, MPI_COMM_WORLD);
        l_cont_node[to_node] = 0;

    }
    else
    {
        MPI_Send(&cont_propagate_visitor,1,MPI_INT,to_node,0, MPI_COMM_WORLD);
        printf("EL P%d ha enviado %d al P%d\n",world_rank,cont_propagate_visitor,to_node);
        MPI_Send(&l_person_propagate,cont_propagate_visitor,coord_type,to_node,0, MPI_COMM_WORLD);
        cont_propagate_visitor = 0;
    }

}

void recive(int flag){

    if(flag == 0){

        MPI_Recv(&cont_move_visitor, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("EL P%d ha recivido del cont_move_visitor %d\n",world_rank,cont_move_visitor);
        if(cont_move_visitor != 0){
           lista_de_persona_mover = malloc(cont_move_visitor*sizeof(person_move_t));
            MPI_Recv(&lista_de_persona_mover, cont_move_visitor, person_move, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        MPI_Recv(&cont_propagate_visitor, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("EL P%d ha recivido del cont_move_visitor %d\n",world_rank,cont_propagate_visitor);
        if(cont_propagate_visitor != 0){
            l_person_propagate_recive = malloc(cont_propagate_visitor*sizeof(coord_t));
            MPI_Recv(&l_person_propagate_recive, cont_propagate_visitor, coord_type, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
}

person_t *init_lists(int pop)
{
    person_t *result = malloc(BATCH * pop * sizeof(person_t));
    return result;
}

void print_person(person_t p, int procesador)
{
    printf("Procedador %d Ha creado la persona => ID_Global:%d|ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]|Speed[%d,%d]\n",
           procesador, p.id_global, p.id, p.state, p.age, p.incubation_period, p.prob_infection, p.recovery, p.coord.x, p.coord.y, p.speed[0], p.speed[1]);
}

int random_number(int min_num, int max_num)
{
    //srand(time(NULL));
    return rand() % (max_num + 1) + min_num;
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

    //Lo he hecho asi porque dentro de las listas ya estan creadas las estructuras
    int state = random_number(0, 2);
    if (state == 0 || state == 3) // SANO
    {
        calculate_init_position(&l_person_notinfected[id_contNotI]);
        init_person_parameters(&l_person_notinfected[id_contNotI], state, id_contNotI);
        world[l_person_notinfected[id_contNotI].coord.x][l_person_notinfected[id_contNotI].coord.y].l = NOT_INFECTED;
        id_contNotI++;
        //print_person(l_person_notinfected[id_contNotI - 1], procesador);
    }
    else // INFECTADO
    {
        calculate_init_position(&l_person_infected[id_contI]);
        init_person_parameters(&l_person_infected[id_contI], state, id_contI);
        world[l_person_infected[id_contI].coord.x][l_person_infected[id_contI].coord.y].l = INFECTED;
        id_contI++;
        //print_person(l_person_infected[id_contI - 1], procesador);
    }

    identificador_global++;
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

void init_move_list(int size_x, int size_y)
{
    l_person_moved = malloc(size_x * sizeof(person_move_t *));
    if (l_person_moved == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        l_person_moved[i] = malloc(size_y * sizeof(person_move_t));
        if (l_person_moved[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            l_person_moved[i][j].person.id = -1;
        }
    }
    
}
/*
void change_state(person_t person) // 1(INFECCIOSO) and 2(NO-INFECCIOSO) States
{
    if (person.incubation_period > 0)
    {
        l_person_infected[person.id].incubation_period--;
    }
    else
    {
        if (person.recovery == 0)
        {
            l_person_infected[person.id].state = 3;
            l_person_infected[person.id].incubation_period = random_number(3, 5);
            l_person_infected[person.id].recovery = random_number(3, 5);
            l_person_infected[person.id].id = -1;
            l_vaccined[id_contVaccined] = person;
            id_contVaccined++;
        }
        else
        {
            l_person_infected[person.id].recovery--;
            if (l_person_infected[person.id].state == 2)
            {
                propagate(&person);
            }
        }
    }
}
void propagate(person_t *person)
{
    //Tengo que controlar que no se vayan del cuadrado
    int directions[12][2] = {{1, 0}, {2, 0}, {1, 1}, {0, 1}, {0, 2}, {-1, 1}, {-1, 0}, {-2, 0}, {-1, -1}, {0, -1}, {0, -2}, {1, -1}};
    index_t index;
    int x = person->coord.x;
    int y = person->coord.y;
    person_t person_aux;
    float prob_aux;
    for (i = 0; i < 12; i++) // Todas las direcciones
    {
        if ((x + directions[i][0]) < SIZE_WORLD && (y + directions[i][1]) < SIZE_WORLD) // Mantenerse dentro
        {
            index = world[x + directions[i][0]][y + directions[i][1]];
            if (index.id != -1 && index.l == NOT_INFECTED) // Persona no infectada y asignada
            {
                person_aux = l_person_notinfected[index.id];
                change_infection_prob(&person_aux);
                if (person_aux.prob_infection > MAX_INFECTION) // Se infecta
                {
                    l_person_notinfected[person_aux.id].id = -1;
                    prob_aux = 1000 * calculate_prob_death(person_aux.age);
                    if (random_number(0, MAX_DEATH) <= prob_aux) // MUERE
                    {
                        printf(">>>>>%d HA MUERTO!\n", person_aux.id);
                        person_aux.state = 5;
                        world[x + directions[i][0]][y + directions[i][1]].id = -1;
                        cont_death++;
                    }
                    else
                    {
                        printf(">>>>>%d SE HA INFECTADO!\n", person_aux.id);
                        if (random_number(0, 1) == 0) // INFECCIOSO
                        {
                            person_aux.state = 2;
                        }
                        else
                        {
                            person_aux.state = 1;
                        }
                        person_aux.id = id_contI;
                        l_person_infected[id_contI] = person_aux;
                        world[x + directions[i][0]][y + directions[i][1]].l = INFECTED;
                        world[x + directions[i][0]][y + directions[i][1]].id = id_contI;
                        id_contI++;
                    }
                }
            }
        }
    }
}
*/

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
    //printf("Fin Move\n");
}

void move_person(person_t *person, int world_rank)
{
    int x = person->coord.x;
    int y = person->coord.y;
    int nx, ny;
    int speed = person->speed[1];
    if (speed != 0)
    {
        //printf("X -> %d Y -> %d\n",x,y);
        int direction = person->speed[0];
        int diagonals[4][2] = {{1, -1}, {-1, -1}, {-1, 1}, {1, 1}};
        int directions[8][2] = {{1,0},{2,0},{0,-1},{0,-2},{-1,0},{-2,0},{0,1},{0,2}};

        //int directions[8][2] = {{0, 1}, {0, 2}, {-1, 0}, {-2, 0}, {0, -1}, {0, -2}, {1, 0}, {2, 0}};
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
        //printf("Antes del if general | Qx %d Qy %d| Direccion %d | X-> %d, Y->%d\n",quadrant_x,quadrant_y,direction,x,y);
        if (x < quadrant_x && y < quadrant_y)  // MISMO QUADRANTE
        {
            if(x >= 0 && y >= 0){
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
            //printf("Antes de search_node | Qx %d Qy %d| Direccion %d | X-> %d, Y->%d\n",quadrant_x,quadrant_y,direction,x,y);
            int to_node = search_node(world_rank, x, y);
            //printf("Despues de search FROM %d | TO NODE -> %d | Qx %d Qy %d| Direccion %d | F X->%d Y->%d |X-> %d, Y->%d\n",world_rank,to_node,quadrant_x,quadrant_y,direction,person->coord.x,person->coord.y,x,y);
            if(to_node != -1) // SE PUEDE
            {
                printf("To Node -> %d\n",to_node);
                //printf("Visitante FROM %d | TO NODE -> %d | Qx %d Qy %d| Direccion %d | F X->%d Y->%d |X-> %d, Y->%d\n",world_rank,to_node,quadrant_x,quadrant_y,direction,person->coord.x,person->coord.y,x,y);
                memcpy(&l_person_moved[to_node][l_cont_node[to_node]].person,person,sizeof (person_t));
                //l_person_moved[to_node][l_cont_node[to_node]].person = *person;
                memcpy(&l_person_moved[to_node][l_cont_node[to_node]].coord,&coord,sizeof (coord_t));
                //l_person_moved[to_node][l_cont_node[to_node]].coord = *coord;
                //l_cont_node[to_node] = l_cont_node[to_node] + 1;
                printf("Fin Visitante\n");
            }
        }
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
    //printf("Dentro search Node | Direction %d | To node %d \n",direction,to_node);
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
            printf("Caso 1 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 2:
        if (from % (SIZE_WORLD / quadrant_x) == ((SIZE_WORLD / quadrant_x) - 1) || from < (SIZE_WORLD / quadrant_x))
        {
            printf("Caso 2 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 3:
        if (from < (SIZE_WORLD / quadrant_x))
        {
            printf("Caso 3 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 4:
        if (from % (SIZE_WORLD / quadrant_x) == 0 || from > quadrant_x)
        {
            printf("Caso 4 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 5:
        if (from % (SIZE_WORLD / quadrant_x) == 0)
        {
            printf("Caso 5 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 6:
        if (from % (SIZE_WORLD / quadrant_x) == 0 || to_node >= world_size)
        {
            printf("Caso 6 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
            return 0;
        }
        break;
    case 7:
        if (to_node >= world_size)
        {
            printf("Caso 7 | From %d| Direction %d| To Node %d|\n",from,direction,to_node);
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

int move_visitor(person_t *person, coord_t *coord)
{
    if (world[coord->x][coord->y].id == -1) // EMPTY POSITION
    {
        move(person, coord);
        return 1;
    }
    return 0;
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

void realocate_lists()
{

    int last_value = 0;
    for (i = 0; i < id_contI; i++)
    { // INFECTED

        if (l_person_infected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contI; j++)
            {
                if (l_person_infected[j].id != -1)
                {
                    last_value = i + 1;
                    break;
                }
            }
            memcpy(&l_person_infected[i], &l_person_infected[j], sizeof(person_t));
            l_person_infected[i].id = i;
            l_person_infected[j].id = -1;
            world[l_person_infected[i].coord.x][l_person_infected[i].coord.y].id = i;
        }
    }
    id_contI = last_value;
    for (i = 0; i < id_contNotI; i++)
    { // INFECTED
        if (l_person_notinfected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; id_contNotI; j++)
            {
                if (l_person_notinfected[j].id != -1)
                {
                    last_value = i + 1;
                    break;
                }
            }
            memcpy(&l_person_notinfected[i], &l_person_notinfected[j], sizeof(person_t));
            l_person_notinfected[i].id = i;
            l_person_notinfected[j].id = -1;
            world[l_person_notinfected[i].coord.x][l_person_notinfected[i].coord.y].id = i;
        }
    }
    id_contNotI = last_value;

    for (i = 0; i < id_contVaccined; i++)
    { // INFECTED
        if (l_vaccined[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; id_contVaccined; j++)
            {
                if (l_vaccined[j].id != -1)
                {
                    last_value = i + 1;
                    break;
                }
            }
            memcpy(&l_vaccined[i], &l_vaccined[j], sizeof(person_t));
            l_vaccined[i].id = i;
            l_vaccined[j].id = -1;
            world[l_vaccined[i].coord.x][l_vaccined[i].coord.y].id = i;
        }
    }
    id_contVaccined = last_value;
}

int vacunate(person_t person)
{
    if (person.age >= group_to_vaccine * 10)
    {
        printf(">>>>>%d VACUNADO!\n", person.id_global);
        l_person_notinfected[person.id].id = -1;
        person.state = 4;
        l_vaccined[id_contVaccined] = person;
        world[person.coord.x][person.coord.y].l = VACCINED;
        world[person.coord.x][person.coord.y].id = id_contVaccined;
        id_contVaccined++;
        return 1;
    }
    else
    {
        return 0; // no vacunado
    }
}

void change_move_prob(person_t *person)
{
    //printf("ANTES  | Person:%d | Speed:%d | Direction:%d | \n",person->id_global,person->speed[1],person->speed[0]);
    person->speed[0] = random_number(1, MAX_DIRECTION);
    person->speed[1] = random_number(0, MAX_SPEED);
    //printf("DESPUES  | Person:%d | Speed:%d | Direction:%d | \n",person->id_global,person->speed[1],person->speed[0]);
}

void init_gsl(int seed)
{
    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);
    gsl_rng_set(r, seed);
}

void change_infection_prob(person_t *person)
{
    //printf("ANTES  | Person:%d | Infection_Prob:%f | \n",person->id_global,person->prob_infection);
    person->prob_infection = gsl_ran_beta(r, alpha, beta);
    //printf("DESPUES  | Person:%d | Infection_Prob:%f | \n",person->id_global,person->prob_infection);
}

void create_data_type_coord(coord_t *coordenadas)
{

    MPI_Datatype types[2] = {MPI_INT, MPI_INT};

    int lengths[2] = {1, 1};
    MPI_Aint displacements[2], dir1, dir2;
    MPI_Aint base_address;

    MPI_Get_address(&coordenadas, &base_address);
    displacements[0] = 0;
    MPI_Get_address(&coordenadas->x, &dir1);
    MPI_Get_address(&coordenadas->y, &dir2);
    displacements[1] = dir2 - dir1;
    MPI_Type_create_struct(2, lengths, displacements, types, &coord_type);
    MPI_Type_commit(&coord_type);
}

void create_data_type_person(person_t *persona)
{

    MPI_Datatype types[9] = {MPI_INT, MPI_INT, MPI_FLOAT, coord_type, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint displacements[9], dir1, dir2;
    MPI_Aint base_address;

    int lengths[9] = {1, 1, 1, 1, 2, 1, 1, 1, 1};

    MPI_Get_address(&persona, &base_address);
    displacements[0] = 0;
    MPI_Get_address(&persona->age, &dir1);
    MPI_Get_address(&persona->state, &dir2);
    displacements[1] = dir2 - dir1;
    MPI_Get_address(&persona->prob_infection, &dir2);
    displacements[2] = dir2 - dir1;
    MPI_Get_address(&persona->coord, &dir2);
    displacements[3] = dir2 - dir1;
    MPI_Get_address(&persona->speed, &dir2);
    displacements[4] = dir2 - dir1;
    MPI_Get_address(&persona->incubation_period, &dir2);
    displacements[5] = dir2 - dir1;
    MPI_Get_address(&persona->recovery, &dir2);
    displacements[6] = dir2 - dir1;
    MPI_Get_address(&persona->id, &dir2);
    displacements[7] = dir2 - dir1;
    MPI_Get_address(&persona->id_global, &dir2);
    displacements[8] = dir2 - dir1;

    MPI_Type_create_struct(9, lengths, displacements, types, &person_type);
    MPI_Type_commit(&person_type);
}

void create_data_type_person_move(coord_t *coordenadas, person_t *persona){

    MPI_Datatype types[2] = {coord_type,person_type};

    int lengths[2] = {1,1};
    MPI_Aint displacements[2],dir1,dir2; 
    MPI_Aint base_address;
    displacements[0] = 0;
    MPI_Get_address(&coordenadas,               &dir1);
    MPI_Get_address(&persona,                   &dir2);
    displacements[1] = dir2 - dir1;

    MPI_Type_create_struct(2, lengths, displacements, types, &person_move);
    MPI_Type_commit(&person_move);
}

float calculate_prob_death(int edad)
{
    for ( i = 0; i < 9; i++)
    {
        if (edad >= 10 * i && edad < 10 * i + 10)
        {
            return l_death_prob[i];
        }
        else if (i == 8)
        {
            return l_death_prob[i];
        }
    }
    return 0.0;
}