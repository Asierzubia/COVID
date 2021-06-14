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

FILE *arch_metrics, *arch_positions;
float l_metrics[5] = {0.0, 0.0, 0.0, 0.0, 0.0};


// void calculate_metrics()
// {
//     l_metrics[0] = l_metrics[0] + cont_healthy;
//     l_metrics[1] = l_metrics[1] + cont_recovered;
//     l_metrics[2] = l_metrics[2] + cont_infected;
//     l_metrics[3] = l_metrics[3] + cont_death;
//     l_metrics[4] = l_metrics[4] + cont_R0 ;

//     cont_healthy = 0;
//     cont_recovered = 0;
//     cont_infected = 0;
//     cont_death = 0;
//     cont_R0 = 0;
// }

void calculate_metric_mean()
{
    printf("[METRICAS]: Antes de calcular media por nodo -> %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf \n", l_metrics[0], l_metrics[1], l_metrics[2], l_metrics[3], l_metrics[4]);
    int num_batch = conf.iter / conf.batch;
    l_metrics[0] = l_metrics[0] / num_batch;
    l_metrics[1] = l_metrics[1] / num_batch;
    l_metrics[2] = l_metrics[2] / num_batch;
    l_metrics[3] = l_metrics[3] / num_batch;
    l_metrics[4] = l_metrics[4] / num_batch;
}

void save_metrics(int world_rank, int iteration)
{
    l_metrics[0] = l_metrics[0] + (float) cont_healthy;
    l_metrics[1] = l_metrics[1] + (float) cont_recovered;
    l_metrics[2] = l_metrics[2] + (float) cont_infected;
    l_metrics[3] = l_metrics[3] + (float) cont_death;
    l_metrics[4] = l_metrics[3] + (float) cont_R0; ;

    cont_healthy = 0;
    cont_recovered = 0;
    cont_infected = 0;
    cont_death = 0;
    cont_R0 = 0;
}

void print_metrics()
{
    printf("[METRICAS]: Imprimiendo metrics...\n");
    //printf("[METRICAS]: %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf\n", recv_metrics[14], recv_metrics[15], recv_metrics[16], recv_metrics[17], recv_metrics[18], recv_metrics[21]);
    //printf("[METRICAS]: %6.4lf - %6.4lf - %6.4lf - %6.4lf - %6.4lf \n", recv_metrics[0][0], recv_metrics[0][1], recv_metrics[0][2], recv_metrics[0][3], recv_metrics[0][4]);
    arch_metrics = fopen("METRICAS.metricas", "w");
    if (arch_metrics == NULL)
    {
        printf("El fichero arch_metrics no se ha podido abrir para escritura.\n");
    }

    float mean_values[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    char str_m[1000];
    char str_aux_m[1000];
    int i, j, k, pos;
    k = 0;
    pos = 0;
    for (i = 0; i < 5; i++)
    {
        pos = i;
        for (j = 0; j < world_size; j++)
        {
            mean_values[i] = mean_values[i] + recv_metrics[pos];
            //printf("%d - %6.4lf - %6.4lf - %d \n", pos, recv_metrics[pos], mean_values[i], i);
            pos += 5;
        }
    }

    printf("\nPersonas sanas : %6.4lf \nPersonas contagiadas : %6.4lf \nPersonas recuperadas : %6.4lf \nPersonas fallecidas : %6.4lf \nR0 : %6.4lf \n", mean_values[0], mean_values[1], mean_values[2], mean_values[3], mean_values[4]);
    snprintf(str_aux_m, sizeof(str_aux_m), "Personas sanas : %6.4lf \nPersonas contagiadas : %6.4lf \nPersonas recuperadas : %6.4lf \nPersonas fallecidas : %6.4lf \nR0 : %6.4lf \n", mean_values[0], mean_values[1], mean_values[2], mean_values[3], mean_values[4]);
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
    snprintf(str_aux, sizeof(str_aux), "P%d | ITERACION: %d | {INFECTED - ", world_rank, iteration);
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
    strcpy(l_positions, str);
}

void metrics(){

    calculate_metric_mean();
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
}
