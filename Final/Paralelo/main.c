#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "definitions.h"


  float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
  float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
  float mean_death, mean_infected, mean_recovered, mean_healthy, mean_RO;

  int iter, posX, posY, i, j, k, position, seed, mu, alfa, beta;
  int num_persons_to_vaccine, group_to_vaccine, person_vaccinned, radius, vaccines_left;
  int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death;
  int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
  int p_death, p_infected, p_recovered, p_healthy, p_RO;
  index_t world[size_world][size_world];
  person_t *l_person_infected, *l_person_notinfected, *l_vaccined;

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
  int population;
  int when_change_group = 5; //Para saber cada cuantas iteraciones tengo que cambiar de grupo
  int cont_iterations = 0;
  population = (int) POPULATION_SIZE / world_size;
  num_persons_to_vaccine = 70 / (population * percent);

  //faltaria tener en cuenta cuando las personas son impares
  //int quadrant = size_world * size_world / ; 
  MPI_Bcast (&population, 1, MPI_INT, 0, MPI_COMM_WORLD);
  //MPI_Bcast (&quadrant, 1, MPI_INT, 0, MPI_COMM_WORLD);


  if(world_rank == 0){

    for(i = 1;i < world_size; i++){
        id = i * population;
        MPI_Send(&id, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(&id, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }

  else{

       	MPI_Recv(&id_contI, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&id_contNotI, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
       // printf("EL procesador %d se queda a la espera de recibir el mensaje  %d\n", world_rank,id_contI);
   }

  printf("P%d == id_contI %d == id_contNotI %d == population %d \n",world_rank,id_contI,id_contNotI,population);
  
  if(world_rank == 0){
      init_world();
  }
  	
  //sInicializar listas

  l_person_infected = init_lists(population);
  l_person_notinfected = init_lists(population); 
  l_vaccined = init_lists(population);

  printf("P%d idContI => %d, id_cont + population => %d",world_rank,id_contI,id_contI+population);
  
  //Crear la poblacion
  int contador = id_contI;
  for (i = contador; i < (contador + population); i++)
  {
   	    printf("P%d Valor de I => %d\n",world_rank,i);
        create_person(l_person_notinfected,l_person_infected,world_rank);
  }
	/*
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
*/
/*
    vaccines_left = num_persons_to_vaccine;
    id_contIAux = id_contI;
    id_contNotIAux = id_contNotI;

    for (i = 0; i < id_contIAux; i++) // INFECTED
    {
        if (l_person_infected[i].id != -1)
        {
            change_state(l_person_infected[i]);
            if (l_person_infected[i].id != -1) // Ha cambiado de estado y no se mueve
            {
                change_move_prob(&l_person_infected[i]);
                move(&l_person_infected[i]);
            }
        }
    }*/

  //}
  free(l_person_infected);
  free(l_person_notinfected);
  free(l_vaccined);
  printf("Finaliza la simulacion\n");
  MPI_Finalize();
  return 0;
}


person_t *init_lists(int size_population)
{

  person_t *result;
  result=(person_t *)malloc(size_population * sizeof(person_t));;
  return result;

}

void print_person(person_t p,int procesador)
{
    printf("Procedador %d Ha creado la persona => ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]\n",
           procesador,p.id, p.state, p.age, p.incubation_period, p.prob_infection, p.recovery, p.coord[0], p.coord[1]);

}

int random_number(int min_num, int max_num)
{
    srand(time(NULL));
    return rand() % (max_num + 1) + min_num;
}


void create_person(person_t *l_notInfected_p,person_t *l_infected_p,int procesador){

    person_t person;
    person.age = random_number(1, 100);
    person.prob_infection = 0.15;
    person.state = random_number(0, 2);
    person.incubation_period = random_number(0, MAX_INCUBATION);
    person.recovery = random_number(0, MAX_RECOVERY);
    index_t index;
    if (person.state == 0 || person.state == 3) // SANO
    {
     	person.id = id_contNotI;
        printf("%d\n", id_contNotI);
        //l_notInfected_p[id_contNotI] = person;
	    memcpy(&l_notInfected_p[id_contNotI], &person, sizeof(person_t));
        id_contNotI++;
        index.l = NOT_INFECTED;
    }
    else // INFECTADO
    {
     	person.id = id_contI;
        //l_notInfected_p[id_contI] = person;
        printf("%d\n", id_contI);

	    memcpy(&l_infected_p[id_contI], &person, sizeof(person_t));
        id_contI++;
        index.l = INFECTED;
    }
    index.id = person.id;
    print_person(person,procesador);
 

}


void init_world()
{
    for (i = 0; i < size_world; i++)
    {
        for (j = 0; j < size_world; j++)
        {

            world[i][j].id = -1;
        }
    }
}
//Como el world lo tenemos definido en definitions entonces imagino
// que lo inicializaran todos los procesadores, y eso seria un desperdicio
// de memoria, como hago para que solo lo inicialice el P0?

//Preguntar a ver si le tengo que pasar la lista de infectados,no infectados y vacunados
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
