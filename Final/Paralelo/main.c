#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "definitions.h"


  float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
  float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
  float mean_death, mean_infected, mean_recovered, mean_healthy, mean_RO;
  int identificador_global;
  int iter, posX, posY, i, j, k, position, seed, mu, alfa, beta;
  int num_persons_to_vaccine, group_to_vaccine, person_vaccinned, radius, vaccines_left;
  int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death;
  int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
  int p_death, p_infected, p_recovered, p_healthy, p_RO;
  index_t **world;
  person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
  person_t *l_person_moved, *l_person_propagate;

  int quadrant_x = 0;
  int quadrant_y = 0;

int main(int argc, char *argv[])
{

  int world_size,world_rank; 

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  //----------FALTA PARAMETRIZACION--------------------------
  //Inicializacion de variables
  int id = 0;
  id_contI = 0;
  id_contNotI = 0;
  id_contVaccined = 0;
  iter = 10;
  srand(world_rank*10); //Falta parametrizarlo para que en cada ejecucion sea diferente
  int population = 0;
  int quadrant = 2;
  int when_change_group = 5; //Para saber cada cuantas iteraciones tengo que cambiar de grupo
  int cont_iterations = 0;
  num_persons_to_vaccine = (int) 70 / (population * percent);
  posX = 0;
  posY = 0;
  bach = 2;
  cont_bach = 1;


  if(world_rank==0){

		quadrant_x = size_world/(int)floor(sqrt(world_size));
		quadrant_y = size_world/(int)ceil(sqrt(world_size));
		population = round(POPULATION_SIZE/world_size);

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
  identificador_global = world_rank * population;

  // Inicializamos el mundo entero --- Hay que hacerlo 1D para que sea dinamico y contiguo
  // Realmente no necesitamos este mundo, hablar con pinas

  init_world(quadrant_x,quadrant_y);

  	
  //Inicializar listas
  l_person_infected = init_lists(population);
  l_person_notinfected = init_lists(population); 
  l_vaccined = init_lists(population);
  // Dos nuevas listas para controlar los que se mueven de cuadrante y a los que hay que infectar de otros cuadrantes
  l_person_moved = init_lists(population);
  l_person_propagate = init_lists(population);

  
  //Crear la poblacion
  int contador = id_contI;
  for (i = 0; i < population; i++)
  {
        create_person(world_rank);
  }
  //Ahora que ya tenemos creado las listas,creadas las personas y el mundo empezamos las iteraciones

  for (k = 0; k < iter; k++) // ITERACCIONES
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
        int id_contIAux = id_contI;
        int id_contNotIAux = id_contNotI;

        for (i = 0; i < id_contIAux; i++) // INFECTED
        {
            if (l_person_infected[i].id != -1)
            {
                //change_state(l_person_infected[i]);
                if (l_person_infected[i].id != -1) // Ha cambiado de estado y no se mueve
                {
                    //change_move_prob(&l_person_infected[i]);
                    //move(&l_person_infected[i]);
                }
            }
        }

        l_person_infected[0].id = -1;
        printf("P%d | Id_contI -> %d\n",world_rank,id_contI);
        realocate_lists();
        printf("P%d | Id_contI -> %d\n",world_rank,id_contI);

        //Comprobamos si hay BATCH
        // if (cont_bach == BATCH)
        // {
        //     cont_bach = 1;
        //     //calculate_metrics();
        //     realocate_lists();
        // }
        // else cont_bach++;
    
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
    printf("Procedador %d Ha creado la persona => ID_Global:%d|ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]\n",
           procesador,p.id_global,p.id, p.state, p.age, p.incubation_period, p.prob_infection, p.recovery, p.coord[0], p.coord[1]);

}

int random_number(int min_num, int max_num)
{
    //srand(time(NULL));
    return rand() % max_num;
}


void create_person(int procesador){

    //Lo he hecho asi porque dentro de las listas ya estan creadas las estructuras
    index_t index;
    int state = random_number(0,2);
    if (state == 0 || state == 3) // SANO
    {        
        calculate_init_position(&l_person_notinfected[id_contNotI]);
        l_person_notinfected[id_contNotI].id = id_contNotI;
        l_person_notinfected[id_contNotI].age = random_number(1, 100);
        l_person_notinfected[id_contNotI].prob_infection = 0.15;
        l_person_notinfected[id_contNotI].state = state;
        l_person_notinfected[id_contNotI].incubation_period = random_number(0, MAX_INCUBATION);
        l_person_notinfected[id_contNotI].recovery = random_number(0, MAX_RECOVERY);
        l_person_notinfected[id_contNotI].id_global = identificador_global;
        identificador_global++;
        index.id = l_person_notinfected[id_contNotI].id;
        index.l = NOT_INFECTED;
        world[l_person_notinfected[id_contNotI].coord[0]][l_person_notinfected[id_contNotI].coord[1]] = index;
        id_contNotI++;
        print_person(l_person_notinfected[id_contNotI-1],procesador);
    }
    else // INFECTADO
    {
        calculate_init_position(&l_person_infected[id_contI]);
     	l_person_infected[id_contI].id = id_contI;
        l_person_infected[id_contI].age = random_number(1, 100);
        l_person_infected[id_contI].prob_infection = 0.15;
        l_person_infected[id_contI].state = state;
        l_person_infected[id_contI].incubation_period = random_number(0, MAX_INCUBATION);
        l_person_infected[id_contI].recovery = random_number(0, MAX_RECOVERY);
        l_person_infected[id_contI].id_global = identificador_global;
        identificador_global++;
        index.id = l_person_infected[id_contI].id;
        index.l = INFECTED;
        world[l_person_infected[id_contI].coord[0]][l_person_infected[id_contI].coord[1]] = index;
        id_contI++;
        print_person(l_person_infected[id_contI-1],procesador);
    }
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
        if ((x + directions[i][0]) < size_world && (y + directions[i][1]) < size_world) // Mantenerse dentro
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