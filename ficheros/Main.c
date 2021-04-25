#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int i,j,k;
int main(int argc, char *argv[])
{
    int iterations = 1000;
    size = argv[1];
    create_population();
    create_enviroment();
    for(j =0 ; j<iterations;j++){
        per_cicle;
    }
}
//Las personas recuperadas se pueden volver a contagiar??????
void per_cicle(){

    for(i = 0; i<size; i++){
        //Mirar a ver si es infectado, y en ese caso mirar a ver si hay alguien en su radio de contagio
        estado = personList[i];
        if(estado == 1 || estado == 2){
            //Ahora tengo que mirar arriba,abajo,izquierda,derecha. Tengo que tener en cuenta los limites de la matriz
            fila = personList[i] / size_world;
            columna = personList[i] % size_world;
            //En ved de un bucle poner las cuatro posibilidades
            for( j= fila ; j < fila+2;j++){
                for(k=columna;k<columhna + 2;k++){
                    if ((columna+1) <= size_world){
                        if(world[(filea*size_world) + columna+1] != 0){
                            if(world[(filea*size_world) + columna+1].state == 0 || world[(filea*size_world) + columna+1].state == 3){
                                changeEstate(i,contagiado); //No hay un estado de contagiado, que le ponemos estado 1 o estado 2. O lo hacemos random
                            }
                        }
                    }
                }
            }
        }
        
        move(i);
        changeCoordenates(i);

    }

}

void changeEstates(int index_person){


}

void changeCoordenates(int index_person){

}

void move(int index_person){


}

void propagate(int index_person){

}