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
  int num_persons_to_vaccine, group_to_vaccine,when_change_group, person_vaccinned, radius, vaccines_left;
  int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death;
  int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
  int p_death, p_infected, p_recovered, p_healthy, p_RO;
  index_t **world;
  person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
  person_t *l_person_moved, *l_person_propagate;

  int quadrant_x = 0;
  int quadrant_y = 0;
  gsl_rng *r;

int main(int argc, char *argv[])
{

  int world_size,world_rank; 

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  //----------FALTA PARAMETRIZACION--------------------------
  //Inicializacion de variables
  int id = 0;
  int seed = world_rank*10*SEED;
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

  srand(seed); //Falta parametrizarlo para que en cada ejecucion sea diferente
  init_gsl(seed);

  if(world_rank==0){
		quadrant_x = SIZE_WORLD/(int)floor(sqrt(world_size));
		quadrant_y = SIZE_WORLD/(int)ceil(sqrt(world_size));
		population = round(POPULATION_SIZE/world_size);
        num_persons_to_vaccine = round(POPULATION_SIZE * PERCENT);
  }
  
  //INICIALIZACION DE FICHEROS	posic = MPI_File_open( MPI_COMM_WORLD, "historialposic.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &posiFile);
  FILE *metricas;
  metricas = fopen("metricas.txt", "w+");
  int pos;
  MPI_Offset position_metrics_offset;
  MPI_File metrics_positions;
  MPI_Status positionStatus;
  pos = MPI_File_open( MPI_COMM_WORLD, "PersonPositions.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &metrics_positions);
  //FINALIZACION DE INICIALIZACION DE FICHEROS

  MPI_Bcast (&population, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast (&quadrant_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast (&quadrant_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast (&num_persons_to_vaccine, 1, MPI_INT, 0, MPI_COMM_WORLD);

  identificador_global = world_rank * population;


  init_world(quadrant_x,quadrant_y);

  	
  //Inicializar listas
  l_person_infected = init_lists(population);
  l_person_notinfected = init_lists(population); 
  l_vaccined = init_lists(population);
  // Dos nuevas listas para controlar los que se mueven de cuadrante y a los que hay que infectar de otros cuadrantes
  l_person_moved = init_lists(population);
  l_person_propagate = init_lists(population);

  
  //Crear la poblacion
  for (i = 0; i < population; i++)
  {
        create_person(world_rank);
  }
  //Ahora que ya tenemos creado las listas,creadas las personas y el mundo empezamos las iteraciones
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
                    //move(&l_person_infected[i]);
                }
            }
        }

        for (i = 0; i < id_contVaccined; i++) // VACCINED
        {
            change_move_prob(&l_vaccined[i]);
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
                        //move(&l_person_notinfected[i]);
                    }
                }
                else
                {
                    change_move_prob(&l_person_notinfected[i]);
                    //move(&l_person_notinfected[i]);
                }
            }
        }
        //Comprobamos si hay BATCH
        if (cont_bach == BATCH)
        {
            cont_bach = 1;
            //calculate_metrics();
            //realocate_lists();
        }
        else {
            cont_bach++;
        }
    
  }

  //Solo imprimir las metricas al final de la ejecucion
   if(world_rank == 0){
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

// void save_metrics(){

   
// }

person_t *init_lists(int pop)
{

  person_t *result = malloc(BATCH * pop * sizeof(person_t));
  return result;

}

void print_person(person_t p,int procesador)
{
    printf("Procedador %d Ha creado la persona => ID_Global:%d|ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]|Speed[%d,%d]\n",
           procesador,p.id_global,p.id, p.state, p.age, p.incubation_period, p.prob_infection, p.recovery, p.coord[0], p.coord[1],p.speed[0],p.speed[1]);

}

int random_number(int min_num, int max_num)
{
    //srand(time(NULL));
    return rand() % (max_num + 1) + min_num;
}

void init_person_parameters(person_t *persona,int state){

        persona->id = id_contNotI;
        persona->age = random_number(1, 100);
        persona->prob_infection = gsl_ran_beta(r, alpha, beta);
        persona->state = state;
        persona->incubation_period = random_number(0, MAX_INCUBATION);
        persona->recovery = random_number(0, MAX_RECOVERY);
        persona->id_global = identificador_global;
}

void create_person(int procesador){

    //Lo he hecho asi porque dentro de las listas ya estan creadas las estructuras
    index_t index;
    int state = random_number(0,2);
    if (state == 0 || state == 3) // SANO
    {        
        calculate_init_position(&l_person_notinfected[id_contNotI]);
        init_person_parameters(&l_person_notinfected[id_contNotI],state);
        index.id = l_person_notinfected[id_contNotI].id;
        index.l = NOT_INFECTED;
        world[l_person_notinfected[id_contNotI].coord[0]][l_person_infected[id_contNotI].coord[1]] = index;  
        id_contNotI++;
        print_person(l_person_notinfected[id_contNotI-1],procesador);
    }
    else // INFECTADO
    {
        calculate_init_position(&l_person_infected[id_contI]);
        init_person_parameters(&l_person_infected[id_contI],state);
        index.id = l_person_infected[id_contI].id;
        index.l = INFECTED;
        world[l_person_infected[id_contI].coord[0]][l_person_infected[id_contI].coord[1]] = index;
        id_contI++;
        print_person(l_person_infected[id_contI-1],procesador);
    }
        identificador_global++;

}


void init_world(int size_x,int size_y)
{
    world = malloc(size_x * sizeof(index_t*));
    if(world == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for(i=0; i<size_x; i++)
    {
        world[i] = malloc(size_y*sizeof(index_t));
        if(world[i] == NULL)
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
    int x = person->coord[0];
    int y = person->coord[1];
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
void calculate_init_position(person_t *person)
{
    person->coord[0] = posX; // coord x
    person->coord[1] = posY; // coord y
    posY++;
    if (posY == quadrant_y)
    {
        posY = 0;
        posX++;
    }
}


void realocate_lists(){

    int last_value = 0;
    for (i = 0; i < id_contI; i++){ // INFECTED
        
        if(l_person_infected[i].id == -1){ //Caso en el que la posicion esta vacia
            for(j = i + 1; j < id_contI; j++ ){
                if(l_person_infected[j].id != -1) {
                    last_value = i + 1;
                    break;
                }
            }
            memcpy(&l_person_infected[i],&l_person_infected[j],sizeof(person_t));
            l_person_infected[i].id = i;
            l_person_infected[j].id = -1;
            world[l_person_infected[i].coord[0]][l_person_infected[i].coord[1]].id = i;
        }

    }
    id_contI = last_value; 
    for (i = 0; i < id_contNotI; i++){ // INFECTED
        if(l_person_notinfected[i].id == -1){ //Caso en el que la posicion esta vacia
            for(j = i + 1; id_contNotI; j++ ){
                if(l_person_notinfected[j].id != -1){
                    last_value = i + 1;
                    break;
                } 
            }
            memcpy(&l_person_notinfected[i],&l_person_notinfected[j],sizeof(person_t));
            l_person_notinfected[i].id = i;
            l_person_notinfected[j].id = -1;
            world[l_person_notinfected[i].coord[0]][l_person_notinfected[i].coord[1]].id = i;
        }        
    }
    id_contNotI = last_value;

    for (i = 0; i < id_contVaccined; i++){ // INFECTED
        if(l_vaccined[i].id == -1){ //Caso en el que la posicion esta vacia
            for(j = i + 1; id_contVaccined; j++ ){
                if(l_vaccined[j].id != -1) {
                    last_value = i + 1;
                    break;
                }
            }
            memcpy(&l_vaccined[i],&l_vaccined[j],sizeof(person_t));
            l_vaccined[i].id = i;
            l_vaccined[j].id = -1;
            world[l_vaccined[i].coord[0]][l_vaccined[i].coord[1]].id = i;
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
        world[person.coord[0]][person.coord[1]].l = VACCINED;
        world[person.coord[0]][person.coord[1]].id = id_contVaccined;
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
    person->speed[0] = random_number(0, MAX_DIRECTION);
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
