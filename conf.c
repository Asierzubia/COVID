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