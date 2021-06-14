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
#include <getopt.h>
#include "definitions.h"

/*Variables globales*/
index_t **world;
person_t *l_person_infected, *l_person_notinfected, *l_vaccined;
person_move_t **l_person_moved;
person_move_t **lista_de_persona_mover;
int **lista_de_persona_mover_return;
coord_t **l_person_propagate;
coord_t *l_person_propagate_recive;
int *l_cont_node_move, *l_cont_node_propagate, *l_cont_persona_mover, *l_cont_person_return;
gsl_rng *r;
struct configuration_t conf;


int world_size, world_rank;
float l_death_prob[9] = {0.0, 0.002, 0.002, 0.002, 0.004, 0.013, 0.036, 0.08, 0.148};
float age_mean, prob_infection, recovery_period, prob_direction, prob_speed;
int identificador_global;
int iter, posX, posY, i, j, k, position, seed, mu, alpha, beta;
int num_persons_to_vaccine, group_to_vaccine, when_change_group, person_vaccinned, radius, vaccines_left;
int id_contVaccined, idx_iter, cont_bach, id_contI, id_contNotI, cont_death, cont_healthy, cont_infected, cont_recovered, cont_move_visitor, cont_move_return, cont_propagate_visitor;
int bach, cont_bach, sanas, contagiadas, fallecidas, recuperadas, RO, num_bach;
int p_death, p_infected, p_recovered, p_healthy, p_RO;
int aux_death, aux_infected, aux_recovered, aux_healthy, aux_RO;
int PX, PY;
int buffer_total;
int buffer_node;
int quadrant_x;
int quadrant_y;
int contador, cont_propagate_recive;
int cont_iter; // ELIMINAR

float iter_healthy, iter_recovered, iter_infected, iter_death, iter_RO;

int **index_return_person;
int *cont_index_return;
float *l_mean_healthy, *l_mean_infected, *l_mean_recovered, *l_mean_death, *l_mean_R0;
float *recv_healthy, *recv_infected, *recv_recovered, *recv_death, *recv_R0;
FILE *arch_metrics, *arch_positions;
char *l_positions, *l_positions_aux, *l_metrics_aux, *recv_positions;
float *recv_metrics;
float l_metrics[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

MPI_Datatype coord_type;
MPI_Datatype person_type;
MPI_Datatype person_move;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_rank == 0){
        set_configuration(argc, argv);
    }
    MPI_Bcast(&conf.world_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.percent, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.population_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.batch, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.iter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&conf.ageMean, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int id_contIAux;
    int id_contNotIAux;
    int is_vaccined;
    int seed = world_rank * 10 * conf.seed;
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
    cont_move_visitor = 0;
    cont_move_return = 0;
    cont_propagate_visitor = 0;
    num_bach = 0;
    cont_iter = 0;
    age_mean = conf.ageMean;
    buffer_node = conf.world_size * conf.world_size * (conf.iter/conf.batch) * 10;
    buffer_total = buffer_node * world_size;
    cont_healthy = 0;
    cont_death = 0;
    cont_infected = 0;
    cont_recovered = 0;

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


    if (world_rank == 0)
    {
        quadrant_x = conf.world_size / (int)floor(sqrt(world_size));
        quadrant_y = conf.world_size / (int)ceil(sqrt(world_size));
        population = round(conf.population_size / world_size);
        num_persons_to_vaccine = round(conf.population_size * conf.percent);
        float *recv_healthy, *recv_infected, *recv_recovered, *recv_death, *recv_R0;
        recv_positions = malloc(buffer_total * sizeof(char));
        getAlphaBeta(age_mean);
    }
    MPI_Bcast(&population, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&quadrant_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_persons_to_vaccine, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alpha, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&beta, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(quadrant_x != quadrant_y){
        printf("LAS MATRICES DEL MUNDO DEBEN SER CUADRADAS, INTRODUCE UN NUMERO DE PROCESADORES CON EL CUAL SEAN CUADRADAS LAS MATRICES DEL MUNDO");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    PX = quadrant_x * (world_rank % (int)round(sqrt(world_size)));
    PY = quadrant_y * (world_rank / (int)round(sqrt(world_size)));

    identificador_global = world_rank * population;

    init_all_lists();

    for (i = 0; i < world_size; i++)
    {
        l_cont_node_move[i] = 0;
        l_cont_persona_mover[i] = 0;
        l_cont_person_return[i] = 0;
        l_cont_node_propagate[i] = 0;
        cont_index_return[i] = 0;
    }

    for (i = 0; i < population; i++)
    {
        create_person(world_rank);
    }

    for (k = 0; k < conf.iter; k++) // ITERACCIONES
    {
        cont_propagate_recive = 0;
        
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
            if (l_vaccined[i].id != -1)
            {
                change_move_prob(&l_vaccined[i]);
                move_person(&l_vaccined[i], world_rank);
            }
        }

        for (i = 0; i < id_contNotIAux; i++) // NOT-INFECTED
        {
            if (l_person_notinfected[i].id != -1)
            {
                if (vaccines_left > 0)
                {
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

        send_visitors(0, 0);
        send_visitors(1, 0);
        move_arrived();
        propagate_arrived();
        send_visitors(0, 1);
        move_returned();


        cont_move_visitor = 0;
        cont_move_return = 0;
        cont_propagate_visitor = 0;


        for (i = 0; i < world_size; i++)
        {
            l_cont_node_move[i] = 0;
            l_cont_node_propagate[i] = 0;
            l_cont_persona_mover[i] = 0;
            l_cont_person_return[i] = 0;
            cont_index_return[i] = 0;
        }

        if (cont_bach == conf.batch)
        {
            cont_bach = 1;
            save_metrics(world_rank, k);
            save_positions(world_rank, k);
            realocate_lists();
            num_bach++;
        }
        else
        {
            cont_bach++;
        }
        cont_iter++; 
    }

    calculate_metric_mean();

    //printf("[METRICAS]: P%d %6.4lf\n", world_rank, l_metrics[2]);
    MPI_Gather(l_positions, buffer_node, MPI_CHAR, recv_positions, buffer_node, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    MPI_Gather(l_metrics, 5, MPI_FLOAT, recv_metrics, 5, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0)
    {
        
        if ((l_positions_aux = malloc(buffer_total * world_size * sizeof(char))) == NULL)
        {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        sprintf(l_positions_aux, "%s", recv_positions);
        for (i = 1; i < world_size; i++)
        {
            strcat(l_positions_aux, &recv_positions[buffer_node * i]);
        }
        print_metrics();
        print_positions();
        free(l_positions_aux);
    }

    printf("[MAIN]: P%d finaliza la simulacion\n", world_rank);

    //free_all_lists();
    MPI_Finalize();
    exit(0);
    return (0);
}

void free_all_lists(){

    free_world();
    free(l_person_infected);
    free(l_person_notinfected);
    free(l_vaccined);
    free(l_cont_node_move);
    free(l_cont_node_propagate);
    free(l_cont_persona_mover);
    free(l_cont_person_return);
    free(l_positions);
    free(recv_metrics);
    free(cont_index_return);
    free(l_person_propagate_recive);
    free(l_mean_healthy); 
    free(l_mean_infected);
    free(l_mean_recovered);
    free(l_mean_death);
    free(l_mean_R0);
    free_move_list();
    free_prop_list();
    free_person_move();
    free_index_list();
    free_person_return();
    gsl_rng_free(r);
}

void init_all_lists(){
    
    recv_metrics = malloc((5*world_size) * sizeof(float));
    init_world(quadrant_x, quadrant_y);
    l_person_infected = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_person_notinfected = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_vaccined = init_lists((quadrant_x * quadrant_y) * 10 * conf.batch);
    l_cont_node_move = malloc(world_size * sizeof(int));
    l_cont_node_propagate = malloc(world_size * sizeof(int));
    l_cont_persona_mover = malloc(world_size * sizeof(int));
    l_cont_person_return = malloc(world_size * sizeof(int));
    l_positions = malloc(buffer_node * sizeof(char)); //init_list_archives(1024);
    recv_metrics = malloc((5*world_size) * sizeof(float));
    cont_index_return = malloc(world_size * sizeof(int));
    l_person_propagate_recive = malloc(conf.world_size * conf.world_size * sizeof(coord_t));
    
    init_move_list(world_size, quadrant_x * quadrant_y);
    init_prop_list(world_size, quadrant_x * quadrant_y);
    init_person_move_list(world_size, quadrant_x * quadrant_y);
    init_index_list(world_size, quadrant_y * quadrant_x);
    init_person_return(world_size, quadrant_x * quadrant_y);

}

void set_configuration(int argc, char *argv[]){
    
    int opt, long_index;
    char *num;
    char *unidad;
    static struct option long_options[] = {
        {"world_size    ",      optional_argument, 0,  's' },
        {"help",                no_argument,       0,  'h' },
        {"percent",             optional_argument, 0,  'p' },
        {"population_size",     optional_argument, 0,  'g' },
        {"batch",               optional_argument, 0,  'b' },
        {"iter",                optional_argument, 0,  'i' },
        {"seed",                optional_argument, 0,  'd' },
        {"ageMean",             optional_argument, 0,  'm' },

        {0,                     0,                 0,   0  }
    };
    conf.world_size             = SIZE_WORLD;
    conf.percent                = PERCENT;
    conf.population_size        = POPULATION_SIZE;
    conf.batch                  = BATCH;
    conf.iter                   = ITER;
    conf.seed                   = SEED;
    conf.ageMean                = AGEMEAN;

    long_index =0;

        //Descomentar si se quiere utilizar parametrizacion por linea de comandos
        //if(argc<=1){

            // while ((opt = getopt_long(argc, argv,":s:hc:p:g:b:i:d:m:", 
            //                     long_options, &long_index )) != -1) {
            //     switch(opt) {
            //         case 's':   /* -s or --world_size */
            //             conf.world_size = atoi(optarg);
            //             break; 
            //         case 'h':   /* -h or --help */
            //         case '?':
            //             printf ("Uso %s [OPTIONS]\n", argv[0]);
            //             printf ("  -s  --world_size=NNN\t"
            //                 "Lado de la matriz [%d]\n", SIZE_WORLD);
            //             printf ("  -h, --help\t\t"
            //                 "Ayuda\n");
            //             printf ("  -p or --percent=NNN\t"
            //                 "Porcentaje de la poblacion a vacunar por iteracion [%f]\n", PERCENT);
            //             printf ("  -g or --population_size=NNN\t"
            //                 "Tamano de poblacion [%d]\n", POPULATION_SIZE);
            //             printf ("  -b or --batch=NNN\t"
            //                 "Numero de iteraciones por Batch [%d]\n", BATCH);
            //             printf ("  -i or --iter=NNN\t"
            //                 "Numero de iteraciones [%d]\n", ITER);
            //             printf ("  -d or --seed=NNN\t"
            //                 "Numero que actua como semilla [%d]\n", SEED);
            //             printf ("  -m or --ageMean=NNN\t"
            //                 "Edad media de la poblacion [%d]\n", AGEMEAN);
            //             printf ("Ejemplo de ejecucion por parametros en linea de comando:\n");
            //             printf ("  ./run.sh \"covid.out -s8 -p0.2 -g60 -b2 -i10 -d2 -m20\" 4 \n ");
            //             printf ("Ejemplo de ejecucion por parametros mediante fichero:\n");
            //             printf ("  ./run.sh \"covid.out parametros.txt\" 4 \n ");
            //             exit (2);
            //         case 'p':   /* -p or --percent */ 
            //             conf.percent = atof(optarg);
            //             break;
            //         case 'g':   /* -g or --population_size */ 
            //             conf.population_size = atoi(optarg);
            //             break; 
            //         case 'b':   /* -b or --batch */ 
            //             conf.batch = atoi(optarg);
            //             break;
            //         case 'i':   /* -i or --iter */ 
            //             conf.iter = atoi(optarg);
            //             break;
            //         case 'd':   /* -d or --seed */ 
            //             conf.seed = atoi(optarg);
            //             break;
            //         case 'm':   /* -m or --ageMean */ 
            //             conf.ageMean = atoi(optarg);
            //             break;
            //         default:
            //             printf("Unknown argument option"); 
            //     } 
            // }
        // else
        // {
        //     printf("No se ha introducido ningun parametro, por lo que se utilizaran los valores por defecto\n");
        // }
    if(argc>1){

    
        FILE *f = fopen(argv[1], "r");
        if( f != 0 )
        {
            char fargs[MAX_FILE_LEN] = "" ;
            fread( fargs, 1, MAX_FILE_LEN, f ) ;

            char* fargv[MAX_ARGS] ;
            int fargc = 0 ;

            fargv[fargc] = strtok( fargs, " \n\r" );

            while( fargc < MAX_ARGS && fargv[fargc] != 0 )
            {
                fargc++ ;
                fargv[fargc] = strtok( 0, "\n\r" );
            }
            char c;
            while( (c = getopt( fargc, fargv, ":s:hc:p:g:b:i:d:m:")) != -1 )
            {
                switch( c )
                {
                case 's':   /* -s or --world_size */
                        printf("[ENTRA S]\n");
                        sscanf(optarg, "%i", &conf.world_size); 
                        break; 
                case 'h':   /* -h or --help */
                case '?':
                    printf ("Uso %s [OPTIONS]\n", argv[0]);
                    printf ("  -s  --world_size=NNN\t"
                        "Lado de la matriz [%d]\n", SIZE_WORLD);
                    printf ("  -h, --help\t\t"
                        "Ayuda\n");
                    printf ("  -p or --percent=NNN\t"
                        "Porcentaje de la poblacion a vacunar por iteracion [%f]\n", PERCENT);
                    printf ("  -g or --population_size=NNN\t"
                        "Tamano de poblacion [%d]\n", POPULATION_SIZE);
                    printf ("  -b or --batch=NNN\t"
                        "Numero de iteraciones por Batch [%d]\n", BATCH);
                    printf ("  -i or --iter=NNN\t"
                        "Numero de iteraciones [%d]\n", ITER);
                    printf ("  -d or --seed=NNN\t"
                        "Numero que actua como semilla [%d]\n", SEED);
                    printf ("  -m or --ageMean=NNN\t"
                        "Edad media de la poblacion [%d]\n", AGEMEAN);
                    printf ("Ejemplo de ejecucion por parametros en linea de comando:\n");
                    printf ("  ./run.sh \"covid.out -s8 -p0.2 -g60 -b2 -i10 -d2 -m20\" 4 \n ");
                    printf ("Ejemplo de ejecucion por parametros mediante fichero:\n");
                    printf ("  ./run.sh \"covid.out parametros.txt\" 4 \n ");
                    exit (2);
                case 'p':   /* -p or --percent */
                                        printf("[ENTRA P]\n");

                    sscanf(optarg, "%f", &conf.percent);
                    break;
                case 'g':   /* -g or --population_size */
                                        printf("[ENTRA G]\n");

                    sscanf(optarg, "%i", &conf.population_size);
                    break; 
                case 'b':   /* -b or --batch */ 
                                        printf("[ENTRA B]\n");

                    sscanf(optarg, "%i", &conf.batch);
                    break;
                case 'i':   /* -i or --iter */
                                        printf("[ENTRA I]\n");

                    sscanf(optarg, "%i", &conf.iter);
                    break;
                case 'd':   /* -d or --seed */ 
                                        printf("[ENTRA D]\n");

                    sscanf(optarg, "%i", &conf.seed);
                    break;
                case 'm':   /* -m or --ageMean */
                                        printf("[ENTRA M]\n");

                    sscanf(optarg, "%i", &conf.ageMean);
                    break;
                default:
                    printf("Unknown argument option"); 
                }
            }
        }
    }
    else
    {
        printf("No se ha introducido ningun fichero, por lo que se utilizaran los valores por defecto\n");
    }
}

void move_returned()
{
    //printf("[RETURNED]: %d moviendo returned\n",world_rank);
    int index;
    int i;
    for (i = 0; i < world_size; i++)
    {
        for (j = 0; j < l_cont_person_return[i]; j++)
        {
            if (world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].l == INFECTED)
            {
                printf("[RETURNED]: {INFECTED} P%d | Persona ID_Global : %d, ID_Local : %d \n", world_rank, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id_global, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id);
                world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].id = -1;
                l_person_infected[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id].id = -1;
            }
            else if (world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].l == NOT_INFECTED)
            {
                printf("[RETURNED]: {NOT-INFECTED} P%d  | Persona ID_Global : %d, ID_Local : %d \n", world_rank, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id_global, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id);
                world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].id = -1;
                l_person_notinfected[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id].id = -1;
            }
            else if (world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].l == VACCINED)
            {
                printf("[RETURNED]: {VACCINED} P%d  | Persona ID_Global : %d, ID_Local : %d \n", world_rank, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id_global, l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id);
                world[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.x][l_person_moved[i][lista_de_persona_mover_return[i][j]].person.coord.y].id = -1;
                l_vaccined[l_person_moved[i][lista_de_persona_mover_return[i][j]].person.id].id = -1;
            }
            else
            {
                printf("[RETURNED]: {MAL} --> P%d en la Pos : %d ¡MAL!\n", world_rank, j);
            }
        }
    }
}
void propagate_arrived()
{
    printf("[ARRIVED]: P%d Iter : %d | Cont : %d \n", world_rank, cont_iter, cont_propagate_recive);
    int i;
    for (i = 0; i < cont_propagate_recive; i++)
    {
        printf("[ARRIVED]: P%d Iter : %d | Origen : P%d | {MIRAR} Pos : [%d,%d] \n", world_rank, cont_iter, i, l_person_propagate_recive[i].x, l_person_propagate_recive[i].y);
        if (world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].id != -1 && world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].l == NOT_INFECTED)
        {
            int id = world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].id;
            printf("[ARRIVED]: P%d Iter : %d | Origen : P%d | {PROPAGATE} Pos : [%d,%d] \n", world_rank, cont_iter, i, l_person_propagate_recive[i].x, l_person_propagate_recive[i].y);
            change_infection_prob(&l_person_notinfected[id]);
            if (l_person_notinfected[id].prob_infection > MAX_INFECTION) // Se infecta
            {
                int prob_aux = 1000 * calculate_prob_death(l_person_notinfected[id].age);
                if (random_number(0, MAX_DEATH) <= prob_aux) // MUERE
                {
                    printf("[ARRIVED]: P%d Iter : %d | Origen : P%d | {MUERTE->%d} Pos : [%d,%d] \n", world_rank, cont_iter, i, l_person_notinfected[id].id_global, l_person_propagate_recive[i].x, l_person_propagate_recive[i].y);
                    l_person_notinfected[id].state = 5;
                    world[l_person_notinfected[id].coord.x][l_person_notinfected[id].coord.y].id = -1;
                    cont_death++;
                }
                else
                {
                    printf("[ARRIVED]: P%d Iter : %d | Origen : P%d | {INFECTADO->%d} Pos : [%d,%d] \n", world_rank, cont_iter, i, l_person_notinfected[id].id_global, l_person_propagate_recive[i].x, l_person_propagate_recive[i].y);
                    if (random_number(0, 1) == 0) // INFECCIOSO
                    {
                        l_person_notinfected[id].state = 2;
                    }
                    else
                    {
                        l_person_notinfected[id].state = 1;
                    }
                    l_person_notinfected[id].id = id_contI;
                    memcpy(&l_person_infected[id_contI], &l_person_notinfected[id], sizeof(person_t));
                    world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].l = INFECTED;
                    world[l_person_propagate_recive[i].x][l_person_propagate_recive[i].y].id = id_contI;
                    id_contI++;
                    cont_infected++;
                }
                l_person_notinfected[id].id = -1;
            }
        }
    }
}

float calculate_prob_death(int edad)
{
    int i;
    for (i = 0; i < 9; i++)
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

void send_visitors(int flag, int flag_return)
{

    int nodo_a_enviar;
    // 1-> Derecha
    nodo_a_enviar = world_rank + 1;
    if (PX + quadrant_x != conf.world_size)
    { //Se pasa de la derecha
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PX != 0)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 2-> Arriba Derecha
    nodo_a_enviar = world_rank - ((conf.world_size / quadrant_x) - 1);
    if (PX + quadrant_x != conf.world_size && PY - quadrant_y >= 0)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PX != 0 && PY + quadrant_y != conf.world_size)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //3 -> Arriba
    nodo_a_enviar = world_rank - (conf.world_size / quadrant_x);
    if (PY - quadrant_y >= 0)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PY + quadrant_y != conf.world_size)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //4 -> Arriba Izquierda
    nodo_a_enviar = world_rank - ((conf.world_size / quadrant_x) + 1);
    if (PY - quadrant_y >= 0 && PX - quadrant_x >= 0)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PX + quadrant_x != conf.world_size && PY + quadrant_y != conf.world_size)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //5 -> IZQUIERDA
    nodo_a_enviar = world_rank - 1;
    if (PX != 0)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PX + quadrant_x != conf.world_size)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //6 -> ABAJO IZQUIERDA
    nodo_a_enviar = world_rank + (conf.world_size / quadrant_x) - 1;
    if (PY + quadrant_y != conf.world_size && PX != 0)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PY != 0 && PX + quadrant_x != conf.world_size)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //7 -> ABAJO
    nodo_a_enviar = world_rank + (conf.world_size / quadrant_x);
    if (PY + quadrant_y != conf.world_size)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PY != 0)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // 8 -> ABAJO DERECHA
    nodo_a_enviar = world_rank + (conf.world_size / quadrant_x) + 1;
    if (PX + quadrant_x != conf.world_size && PY + quadrant_y != conf.world_size)
    {
        if (flag_return == 0)
        {
            Psend(nodo_a_enviar, flag);
        }
        else
        {
            Psend_return(nodo_a_enviar);
        }
    }
    if (PY != 0 && PX != 0)
    {
        if (flag_return == 0)
        {
            recive(flag);
        }
        else
        {
            recive_return();
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
}

void Psend_return(int to_node)
{

    MPI_Send(&cont_index_return[to_node], 1, MPI_INT, to_node, 0, MPI_COMM_WORLD);
    if (cont_index_return[to_node] > 0)
    {
        MPI_Send(index_return_person[to_node], cont_index_return[to_node], MPI_INT, to_node, 0, MPI_COMM_WORLD);
        //printf("Contador enviado dos -> %d\n",l_cont_node_move[to_node]);
    }
}

void Psend(int to_node, int flag)
{

    if (flag == 0)
    {
        MPI_Send(&l_cont_node_move[to_node], 1, MPI_INT, to_node, 0, MPI_COMM_WORLD);
        if (l_cont_node_move[to_node] > 0)
        {
            MPI_Send(l_person_moved[to_node], l_cont_node_move[to_node], person_move, to_node, 0, MPI_COMM_WORLD);
            //printf("Contador enviado -> %d\n",l_cont_node_move[to_node]);
        }
    }
    else
    {
        //printf("Contador enviado COORD-> %d\n",l_cont_node_move[to_node]);
        MPI_Send(&l_cont_node_propagate[to_node], 1, MPI_INT, to_node, 0, MPI_COMM_WORLD);
        if (l_cont_node_propagate[to_node] > 0)
        {
            MPI_Send(l_person_propagate[to_node], l_cont_node_propagate[to_node], coord_type, to_node, 0, MPI_COMM_WORLD);
        }
    }
}

void recive_return()
{

    MPI_Status status_person;
    MPI_Recv(&cont_move_return, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status_person);
    //printf("Lo recivo del DOS P%d\n",status_person.MPI_SOURCE);
    if (cont_move_return > 0)
    {
        MPI_Recv(&lista_de_persona_mover_return[status_person.MPI_SOURCE][0], cont_move_return, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        l_cont_person_return[status_person.MPI_SOURCE] = cont_move_return;
    }
}

void recive(int flag)
{

    if (flag == 0)
    {

        MPI_Status status_person;
        MPI_Recv(&cont_move_visitor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status_person);
        if (cont_move_visitor > 0)
        {
            MPI_Recv(&lista_de_persona_mover[status_person.MPI_SOURCE][0], cont_move_visitor, person_move, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            l_cont_persona_mover[status_person.MPI_SOURCE] = cont_move_visitor;
        }
    }
    else
    {
        int cont_p_visitor;
        MPI_Recv(&cont_p_visitor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (cont_p_visitor > 0)
        {
            MPI_Recv(&l_person_propagate_recive[cont_propagate_recive], cont_p_visitor, coord_type, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("LO HA RECIVIDO DEL PROCESADOR COORD----> %d\n",status_coor.MPI_SOURCE);
            cont_propagate_recive = cont_propagate_recive + cont_p_visitor;
        }
        //printf("[RECIVE]: {PROPAGATE} P%d Cont : %d Cont Visitor: %d\n", world_rank, cont_propagate_recive, cont_p_visitor);
    }
}

void move_arrived()
{

    int l;
    int i;
    for (i = 0; i < world_size; i++)
    {
        for (l = 0; l < l_cont_persona_mover[i]; l++)
        {
            if (world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].id == -1)
            { //Esta libre el sitio
                printf("[ARRIVED]: P%d Iter : %d | {MOVER} ID_Global : %d | Pos : [%d,%d] \n", world_rank, cont_iter, lista_de_persona_mover[i][l].person.id_global, lista_de_persona_mover[i][l].coord.x, lista_de_persona_mover[i][l].coord.y);
                //Lo meto en la lista correspondiente en base a si es infectado vacunado o no infectado
                if (lista_de_persona_mover[i][l].person.state == 0)
                { //No esta infectado
                    memcpy(&l_person_notinfected[id_contNotI], &lista_de_persona_mover[i][l].person, sizeof(person_t));
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].l = NOT_INFECTED;
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].id = id_contNotI;
                    id_contNotI++;
                }
                else if (lista_de_persona_mover[i][l].person.state == 1 || lista_de_persona_mover[i][l].person.state == 2)
                { //Esta infectado
                    memcpy(&l_person_infected[id_contI], &lista_de_persona_mover[i][l].person, sizeof(person_t));
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].l = INFECTED;
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].id = id_contI;
                    id_contI++;
                    cont_infected++;
                }
                else
                { //Esta Vacunado
                    memcpy(&l_vaccined[id_contVaccined], &lista_de_persona_mover[i][l].person, sizeof(person_t));
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].l = VACCINED;
                    world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].id = id_contVaccined;
                    id_contVaccined++;
                }
                //Devuelvo los que se han movido
                index_return_person[i][cont_index_return[i]] = l;
                cont_index_return[i] += 1;
            }
            else
            {
                printf("[ARRIVED]: P%d Iter : %d | {NO-MOVER} ID_Global : %d | Pos : [%d,%d] | ID_Local residente : %d \n", world_rank, cont_iter, lista_de_persona_mover[i][l].person.id_global, lista_de_persona_mover[i][l].coord.x, lista_de_persona_mover[i][l].coord.y, world[lista_de_persona_mover[i][l].coord.x][lista_de_persona_mover[i][l].coord.y].id);
            }
        }
    }
}

void print_lists(int world_rank)
{

    //printf("---INFECTADOS-----\n");
    for (i = 0; i < id_contI; i++)
    {
        print_person(&l_person_infected[i], world_rank, 'i');
    }
    //printf("---NO INFECTADOS-----\n");
    for (i = 0; i < id_contNotI; i++)
    {
        print_person(&l_person_notinfected[i], world_rank, 'n');
    }
    //printf("---VACUNADOS-----\n");
    for (i = 0; i < id_contVaccined; i++)
    {
        print_person(&l_vaccined[i], world_rank, 'v');
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
        to_node = (world_rank + 1) + (conf.world_size / quadrant_y);
        direction = 8;
    }
    else if (y >= quadrant_y && x < 0) // DIAGONAL IZQUIERDA ARRIBA
    {
        to_node = (world_rank - 1) + (conf.world_size / quadrant_y);
        direction = 6;
    }
    else if (y >= quadrant_y && x < quadrant_x && x >= 0) // ARRIBA (+ DIAGONAL SIN CAMBIAR)
    {
        to_node = world_rank + (conf.world_size / quadrant_y);
        direction = 7;
    }
    else if (x >= quadrant_x && y < 0) // DIAGONAL DERECHA ABAJO
    {
        to_node = (world_rank + 1) - (conf.world_size / quadrant_y);
        direction = 2;
    }
    else if (y < 0 && x < 0) // DIAGONAL IZQUIERDA ABAJO
    {
        to_node = (world_rank - 1) - (conf.world_size / quadrant_y);
        direction = 4;
    }
    else if (y < 0 && x < quadrant_x && x >= 0) // ABAJO (+ DIAGONAL SIN CAMBIAR)
    {
        to_node = world_rank - (conf.world_size / quadrant_y);
        direction = 3;
    }

    // Comprobar que no se sale
    int sol = is_inside_world(world_rank, direction, to_node);
    if (sol == 0)
    {
        return -1;
    }
    else
    {
        return to_node;
    }
}

int is_inside_world(int from, int direction, int to_node)
{
    switch (direction)
    {
    case 1:
        if (from % (conf.world_size / quadrant_x) == ((conf.world_size / quadrant_x) - 1))
        {
            return 0;
        }
        break;
    case 2:
        if (from % (conf.world_size / quadrant_x) == ((conf.world_size / quadrant_x) - 1) || from < (conf.world_size / quadrant_x))
        {
            return 0;
        }
        break;
    case 3:
        if (from < (conf.world_size / quadrant_x))
        {
            return 0;
        }
        break;
    case 4:
        if (from % (conf.world_size / quadrant_x) == 0 || from > quadrant_x)
        {
            return 0;
        }
        break;
    case 5:
        if (from % (conf.world_size / quadrant_x) == 0)
        {
            return 0;
        }
        break;
    case 6:
        if (from % (conf.world_size / quadrant_x) == 0 || to_node >= world_size)
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
        if (from % (conf.world_size / quadrant_x) == ((conf.world_size / quadrant_x) - 1) || to_node >= world_size)
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

coord_t calculate_coord(int x, int y)
{
    coord_t coord;
    coord.x = x;
    coord.y = y;
    if (x < 0)
    {
        coord.x = quadrant_x + x;
    }
    else if (x >= quadrant_x)
    {
        coord.x = x - quadrant_x;
    }

    if (y < 0)
    {
        coord.y = quadrant_y + y;
    }
    else if (y >= quadrant_y)
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
        int directions[8][2] = {{1, 0}, {2, 0}, {0, -1}, {0, -2}, {-1, 0}, {-2, 0}, {0, 1}, {0, 2}};

        if (direction == 2 || direction == 4 || direction == 6 || direction == 8) // DIAGONAL
        {
            x += diagonals[(direction / 2) - 1][0];
            y += diagonals[(direction / 2) - 1][1];
        }
        else
        {
            x += directions[direction - 1 + speed - 1][0];
            y += directions[direction - 1 + speed - 1][1];
        }
        coord_t coord;
        coord.x = x;
        coord.y = y;
        if (x < quadrant_x && y < quadrant_y) // MISMO QUADRANTE
        {
            if (x >= 0 && y >= 0)
            {
                //printf("X -> %d | Y -> %d\n",x,y);
                if (world[x][y].id == -1) // EMPTY POSITION
                {
                    //printf("Mismo cuadrante X-> %d, Y->%d\n",x,y);
                    world[person->coord.x][person->coord.y].id = -1; // Se elimina la anterior pos
                    printf("[MOVE]: P%d Iter : %d | {LOCAL} | ID : %d | ID_Global : %d | Actual : [%d,%d] | Nueva [%d,%d] | Speed [%d,%d] | State : %d\n",
                           world_rank, cont_iter, person->id, person->id_global, person->coord.x, person->coord.y, x, y, person->speed[0], person->speed[1], person->state);
                    move(person, &coord);
                }
            }
        }
        else
        {
            int to_node = search_node(world_rank, x, y);
            if (to_node != -1) // SE PUEDE
            {
                coord = calculate_coord(x, y);
                if (to_node < world_size && to_node >= 0)
                {
                    printf("[MOVE]: P%d Iter : %d | {VISITANTE->P%d} | ID : %d | ID_Global : %d | Actual : [%d,%d] | Nueva [%d,%d] | Speed [%d,%d] | State : %d\n",
                           world_rank, cont_iter, to_node, person->id, person->id_global, person->coord.x, person->coord.y, x, y, person->speed[0], person->speed[1], person->state);
                    memcpy(&l_person_moved[to_node][l_cont_node_move[to_node]].person, person, sizeof(person_t));
                    memcpy(&l_person_moved[to_node][l_cont_node_move[to_node]].coord, &coord, sizeof(coord_t));
                    l_cont_node_move[to_node] = l_cont_node_move[to_node] + 1;
                }
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
        if (person->recovery == 0) // INMUNIZADO
        {
            printf("[STATE]: P%d | ID_Global %d ha cambiado de estado\n", world_rank, person->id_global);
            person_t *person_ptr, person_aux;
            int pId = person->id;
            person->state = 3;
            person->incubation_period = random_number(3, 5);
            person->recovery = random_number(3, 5);
            person->id = id_contVaccined;
            world[person->coord.x][person->coord.y].l = VACCINED;
            world[person->coord.x][person->coord.y].id = id_contVaccined;
            memcpy(&l_vaccined[id_contVaccined], person, sizeof(person_t));
            l_person_infected[pId].id = -1;
            id_contVaccined++;
            cont_recovered++;
            cont_healthy++;
        }
        else // PROPAGA
        {
            person->recovery--;
            if (person->state == 2)
            {
                propagate(person);
            }
        }
    }
}

void propagate(person_t *person)
{
    int directions[12][2] = {{1, 0}, {2, 0}, {1, 1}, {0, 1}, {0, 2}, {-1, 1}, {-1, 0}, {-2, 0}, {-1, -1}, {0, -1}, {0, -2}, {1, -1}};
    index_t index;
    int x = person->coord.x;
    int y = person->coord.y;
    person_t *person_ptr, person_aux;
    float prob_aux;
    int n;
    for (n = 0; n < 12; n++) // Todas las direcciones
    {
        if ((x + directions[n][0]) < quadrant_x && (y + directions[n][1]) < quadrant_y && (y + directions[n][1]) >= 0 && (x + directions[n][0]) >= 0 && x >= 0 && y >= 0) // Mantenerse dentro
        {
            memcpy(&index, &world[x + directions[n][0]][y + directions[n][1]], sizeof(index_t));
            if (index.id != -1 && index.l == NOT_INFECTED) // Persona no infectada y asignada
            {
                // Se guarda la persona y se calculan los datos
                printf("[PROPAGATE]: {INDEX} P%d Iter : %d | ID_Global : %d | Mirar ID_Local : %d | Mirar Pos [%d,%d] \n", world_rank, cont_iter, person->id_global, index.id, x + directions[n][0], y + directions[n][1]);
                memcpy(&person_aux, &l_person_notinfected[index.id], sizeof(person_t));
                //person_aux.prob_infection = 0.9;
                change_infection_prob(&person_aux);
                if (person_aux.prob_infection > MAX_INFECTION) // SE INFECTA
                {
                    l_person_notinfected[index.id].id = -1;
                    prob_aux = 1000 * calculate_prob_death(person_aux.age);
                    if (random_number(0, MAX_DEATH) <= prob_aux) // MUERE
                    {
                        printf("[PROPAGATE]: P%d Iter : %d | {MUERTE -> %d} | Atacado por --> ID_Global : %d | ID_Local : %d | Pos : [%d,%d] \n", world_rank, cont_iter, l_person_notinfected[index.id].id_global, person->id_global, person->id, x + directions[n][0], y + directions[n][1]);
                        person_aux.state = 5;
                        world[x + directions[n][0]][y + directions[n][1]].id = -1;
                        cont_death++;
                    }
                    else
                    {
                        printf("[PROPAGATE]: P%d Iter : %d | {INFECTADO -> %d} | Infectado por --> ID_Global : %d | ID_Local : %d | Pos : [%d,%d] | ID_contI : %d \n", world_rank, cont_iter, l_person_notinfected[index.id].id_global, person->id_global, person->id, x + directions[n][0], y + directions[n][1], id_contI);
                        if (random_number(0, 1) == 0) // INFECCIOSO
                        {
                            person_aux.state = 2;
                        }
                        else
                        {
                            person_aux.state = 1;
                        }
                        person_aux.id = id_contI;
                        memcpy(&l_person_infected[id_contI], &person_aux, sizeof(person_t));
                        world[x + directions[n][0]][y + directions[n][1]].l = INFECTED;
                        world[x + directions[n][0]][y + directions[n][1]].id = id_contI;
                        id_contI++;
                        cont_infected++;
                    }
                }
            }
        }
        else // VISITANTE
        {
            int to_node = search_node(world_rank, x + directions[n][0], y + directions[n][1]);
            if (to_node != -1)
            {
                printf("[PROPAGATE]: P%d Iter : %d | {VISITANTE->P%d} ID_Global : %d | ID_Local : %d | Pos : [%d,%d] \n", world_rank, cont_iter, to_node, person->id_global, person->id, x + directions[n][0], y + directions[n][1]);
                coord_t coord = calculate_coord(x + directions[n][0], y + directions[n][1]);
                memcpy(&l_person_propagate[to_node][l_cont_node_propagate[to_node]], &coord, sizeof(coord_t));
                l_cont_node_propagate[to_node]++;
            }
        }
    }
}


void change_infection_prob(person_t *person)
{
    person->prob_infection = gsl_ran_beta(r, alpha, beta);
}

int vacunate(person_t *person)
{
    if (person->age >= group_to_vaccine * 10)
    {
        printf("[VACUNAR]: %d\n", person->id_global);
        int pId = person->id;
        person->state = 4;
        person->id = id_contVaccined;
        world[person->coord.x][person->coord.y].l = VACCINED;
        world[person->coord.x][person->coord.y].id = id_contVaccined;
        memcpy(&l_vaccined[id_contVaccined], person, sizeof(person_t));
        l_person_notinfected[pId].id = -1;
        id_contVaccined++;
        cont_healthy++;
        return 1;
    }
    else
    {
        return 0; // no vacunado
    }
}

person_t *init_lists(int pop)
{
    person_t *result = malloc( pop * sizeof(person_t));
    return result;
}

void init_world(int size_x, int size_y)
{
    int i;
    world = malloc(size_x * sizeof(index_t *));
    if (world == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
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

char *init_list_archives(int size)
{
    char *result = malloc(size * sizeof(char));
    return result;
}

void print_person(person_t *p, int procesador, char lista)
{
    printf("[PERSON]: P%d --> ID_Global:%d|ID:%d|State:%d|Age:%d|Incubation:%d|InfectionProb:%6.4lf|Recovery:%d|Coord:[%d,%d]|Speed[%d,%d]\n",
           procesador, p->id_global, p->id, p->state, p->age, p->incubation_period, p->prob_infection, p->recovery, p->coord.x, p->coord.y, p->speed[0], p->speed[1]);
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
    persona->age = roundf(mu * gsl_ran_beta(r, alpha, beta));
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

void print_world(int worldRank)
{

    for (i = 0; i < quadrant_x; i++)
    {
        for (j = 0; j < quadrant_y; j++)
        {
            printf("El P%d inicializada el mundo, posicion | X->%d  | Y->%d  |\n", worldRank, i, j);
        }
    }
}

void realocate_lists()
{
    int i;
    int last_value = id_contI;
    for (i = 0; i < id_contI; i++)
    { // INFECTED

        if (l_person_infected[i].id == -1)
        { //Caso en el que la posicion esta vacia
            for (j = i + 1; j < id_contI; j++)
            {
                if (l_person_infected[j].id != -1)
                {
                    last_value = i + 1;
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
                if (l_person_notinfected[j].id != -1)
                {
                    last_value = i + 1;
                    memcpy(&l_person_notinfected[i], &l_person_notinfected[j], sizeof(person_t));
                    l_person_notinfected[i].id = i;
                    l_person_notinfected[j].id = -1;
                    world[l_person_notinfected[i].coord.x][l_person_notinfected[i].coord.y].id = i;
                    break;
                }
            }
        }
    }
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
                    last_value = i + 1;
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


void init_person_move_list(int size_x, int size_y)
{

    lista_de_persona_mover = malloc(size_x * sizeof(person_move_t *));
    if (lista_de_persona_mover == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int l, n;
    for (l = 0; l < size_x; l++)
    {
        lista_de_persona_mover[l] = malloc(size_y * sizeof(person_move_t));
        if (lista_de_persona_mover[l] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (n = 0; n < size_y; n++)
        {
            lista_de_persona_mover[l][n].person.id = -1;
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
    int l, n;
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

void create_data_type_person_move(coord_t *coordenadas, person_t *persona)
{

    MPI_Datatype types[2] = {coord_type, person_type};

    int lengths[2] = {1, 1};
    MPI_Aint displacements[2], dir1, dir2;
    MPI_Aint base_address;
    displacements[0] = 0;
    MPI_Get_address(&coordenadas, &dir1);
    MPI_Get_address(&persona, &dir2);
    displacements[1] = dir2 - dir1;

    MPI_Type_create_struct(2, lengths, displacements, types, &person_move);
    MPI_Type_commit(&person_move);
}

void init_index_list(int size_x, int size_y)
{
    int i;
    index_return_person = malloc(size_x * sizeof(int *));
    if (index_return_person == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < size_x; i++)
    {
        index_return_person[i] = malloc(size_y * sizeof(int));
        if (index_return_person[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            index_return_person[i][j] = -1;
            index_return_person[i][j] = -1;
        }
    }
}

void init_person_return(int size_x, int size_y)
{

    lista_de_persona_mover_return = malloc(size_x * sizeof(int *));
    if (lista_de_persona_mover_return == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        lista_de_persona_mover_return[i] = malloc(size_y * sizeof(int));
        if (lista_de_persona_mover_return[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            lista_de_persona_mover_return[i][j] = -1;
            lista_de_persona_mover_return[i][j] = -1;
        }
    }
}

void init_prop_list(int size_x, int size_y)
{
    l_person_propagate = malloc(size_x * sizeof(coord_t *));
    if (l_person_propagate == NULL)
    {
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < size_x; i++)
    {
        l_person_propagate[i] = malloc(size_y * sizeof(coord_t));
        if (l_person_propagate[i] == NULL)
        {
            fprintf(stderr, "Memory Allocation Failed\n");
            exit(1);
        }
        for (j = 0; j < size_y; j++)
        {
            l_person_propagate[i][j].x = -1;
            l_person_propagate[i][j].y = -1;
        }
    }
}



void free_move_list()
{
    int i;
    for (i = 0; i < world_size; i++)
    {
        free(l_person_moved[i]);
    }
    free(l_person_moved);
}

void free_prop_list()
{

    int i;
    for (i = 0; i < world_size; i++)
    {
        free(l_person_propagate[i]);
    }
    free(l_person_propagate);
}

void calculate_metrics()
{
    l_metrics[0] = l_metrics[0] + cont_healthy;
    l_metrics[1] = l_metrics[1] + cont_recovered;
    l_metrics[2] = l_metrics[2] + cont_infected;
    l_metrics[3] = l_metrics[3] + cont_death;
    //l_metrics[4] = l_metrics[4] + cont_ ;

    cont_healthy = 0;
    cont_recovered = 0;
    cont_infected = 0;
    cont_death = 0;
}

void calculate_metric_mean()
{
    printf("[METRICAS]: Antes de calcular media por nodo -> %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf \n", l_metrics[0], l_metrics[1], l_metrics[2], l_metrics[3], l_metrics[4]);
    int num_batch = conf.iter / conf.batch;
    l_metrics[0] = l_metrics[0] / num_batch;
    l_metrics[1] = l_metrics[1] / num_batch;
    l_metrics[2] = l_metrics[2] / num_batch;
    l_metrics[3] = l_metrics[3] / num_batch;
}

void save_metrics(int world_rank, int iteration)
{
    l_metrics[0] = l_metrics[0] + (float) cont_healthy;
    l_metrics[1] = l_metrics[1] + (float) cont_recovered;
    l_metrics[2] = l_metrics[2] + (float) cont_infected;
    l_metrics[3] = l_metrics[3] + (float) cont_death;
    l_metrics[4] = 0.0 ;

    cont_healthy = 0;
    cont_recovered = 0;
    cont_infected = 0;
    cont_death = 0;
}

void print_metrics()
{
    printf("[METRICAS]: Imprimiendo metrics...\n");
    printf("[METRICAS]: %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf\n", recv_metrics[0], recv_metrics[1], recv_metrics[2], recv_metrics[3], recv_metrics[4], recv_metrics[5]);
    //printf("\nPersonas sanas : %6.4lf \nPersonas contagiadas : %6.4lf \nPersonas recuperadas : %6.4lf \nPersonas fallecidas : %6.4lf \nR0 : %6.4lf \n", mean_values[0], mean_values[1], mean_values[2], mean_values[3], mean_values[4]);
    //printf("[METRICAS]: %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf \n", recv_metrics[0][0], recv_metrics[0][1], recv_metrics[0][2], recv_metrics[0][3], recv_metrics[0][4]);
    arch_metrics = fopen("METRICAS.metricas", "w");
    if (arch_metrics == NULL)
    {
        printf("El fichero arch_metrics no se ha podido abrir para escritura.\n");
    }

    float mean_values[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    char str_m[1000];
    char str_aux_m[1000];
    int i, j, pos;
    for (i = 0; i < world_size; i+=5)
    {
        for (j = 0; j < 5; j++)
        {
            pos = i+j;
            mean_values[j] = mean_values[j] + recv_metrics[pos];
        }
        
    }

    printf("%6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf ", mean_values[0], mean_values[1], mean_values[2], mean_values[3], mean_values[4]);
    snprintf(str_aux_m, sizeof(str_aux_m), "%6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf ", mean_values[0], mean_values[1], mean_values[2], mean_values[3], mean_values[4]);
    //strcat(str_m, str_aux_m);
    fprintf(arch_metrics, str_aux_m);

    if (fclose(arch_metrics) != 0)
    {
        printf("No se ha podido cerrar el arch_metrics.\n");
    }
}

void print_positions()
{
    printf("[METRICAS]: Imprimiendo positions\n");
    arch_positions = fopen("POSITIONS.pos", "w");
    if (arch_positions == NULL)
    {
        printf("El fichero arch_positions no se ha podido abrir para escritura.\n");
    }
    fprintf(arch_positions, l_positions_aux);
    if (fclose(arch_positions) != 0)
    {
        printf("No se ha podido cerrar el arch_positions.\n");
    }
}

void save_positions(int world_rank, int iteration)
{
    char str[buffer_node];
    char str_aux[buffer_node];
    snprintf(str_aux, sizeof(str), "P%d | ITERACCION: %d | {INFECTED - ", world_rank, iteration);
    strcat(str, str_aux);
    int i;
    for (i = 0; i < id_contI; i++) // INFECTED
    {
        if (l_person_infected[i].id != -1)
        {
            snprintf(str_aux, sizeof(str_aux), "| %d[%d,%d]", l_person_infected[i].id_global, l_person_infected[i].coord.x, l_person_infected[i].coord.y);
            strcat(str, str_aux);
        }
    }
    strcat(str, "} {NOT-INFECTED - ");
    for (i = 0; i < id_contNotI; i++) // NOT-INFECTED
    {
        if (l_person_notinfected[i].id != -1)
        {
            snprintf(str_aux, sizeof(str_aux), "| %d[%d,%d]", l_person_notinfected[i].id_global, l_person_notinfected[i].coord.x, l_person_notinfected[i].coord.y);
            strcat(str, str_aux);
        }
    }
    strcat(str, "} {VACCINED - ");
    //printf("Print 2 %s\n",str);
    for (i = 0; i < id_contVaccined; i++) // VACCINED
    {
        if (l_vaccined[i].id != -1)
        {
            snprintf(str_aux, sizeof(str_aux), "| %d[%d,%d]", l_vaccined[i].id_global, l_vaccined[i].coord.x, l_vaccined[i].coord.y);
            strcat(str, str_aux);
        }
    }
    strcat(str, "}\n");
    //printf(">>>PRINT %s\n",str); (BATCH/ITER)-1
    strcpy(l_positions, str);
}

void free_person_move()
{

    int o;
    for (o = 0; o < world_size; o++)
    {
        free(lista_de_persona_mover[o]);
    }
    free(lista_de_persona_mover);
}
void free_world()
{
    int i;
    for (i = 0; i < quadrant_x; i++)
    {
        free(world[i]);
    }
    free(world);
}

void free_index_list()
{
    int o;
    for (o = 0; o < world_size; o++)
    {
        free(index_return_person[o]);
    }
    free(index_return_person);
}

void free_person_return()
{
    int o;
    for (o = 0; o < world_size; o++)
    {
        free(lista_de_persona_mover_return[o]);
    }
    free(lista_de_persona_mover_return);
}

void getAlphaBeta(int ageMean){

	float normalized_age = ageMean / 100.0;
    float variance = pow((16 / 100.0),2);

    alpha = roundf( ((1 - normalized_age) * pow(normalized_age,2) - (variance * normalized_age)) / variance );
    beta = roundf( (alpha * ( 1 - normalized_age ))  / normalized_age );
 
}
