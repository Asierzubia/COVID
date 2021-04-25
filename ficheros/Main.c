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

//-------------DUDAS-----------------------------------------
//Las personas recuperadas se pueden volver a contagiar??????
//Que estado le ponemos a una persona contiagiosa, estado con sintomas o sin sintomas.
//Que sentido tienes que toda la poblacion tenga la misma probabilidad de moverse? No sera que cada persona tiene una probabilidad de moverse? Porque sino habria cilcos en los que se mueven todas las personas, y otros en los que no se moveria ninguna.

void per_cicle(){
    /*---------------IDEA--------------------------------------*/
    /*La idea es meter a las personas en el entorno, pero y 
    si solo miramos las personas de la lista de personas
    de la poblacion en ved de mirar toda la matriz? Solo haria 
    falta un atributo extra dentro de persona que indique en que
    posicion de la matriz se encuentra en cada momento. De manera 
    que el entorno sea algo simbolico pero no necesario. Solo nos 
    interesaria cambiar los atributos de las personas dentro del 
    entorno si lo dibujaramos,pero si no hacemos ese opcional, 
    daria absolutamente igual, y nos ahorrariamos espacio
    y aumentariamos eficiencia. Creo yo vamos.
    ------------------------------------------------------------*/
    for(i = 0; i<size; i++){
        //Mirar a ver si es infectado, y en ese caso mirar a ver si hay alguien en su radio de contagio
        estado = personList[i];
        if(estado == 1 || estado == 2){
            //Lo primero es mirar a ver si se ha recuperado para cambiarle de estado a recuperado.
            if(personList[i].recovery == 0){

                changeEstate(i,3);
                personList[i].recovery = recovery_period;

            }//Si no se ha recuperado, le resto uno el periodo de recuperacion que le queda.
            else if(personList[i].recovery != 0){

                personList[i].recovery -= 1;
            }
            //Miramos la probabilidad de muerte, para saber si darlo por muerto o no. Si la probabilidad es mayor que 0.7 lo damos por muerto.
            //De esta manera lo miramos cada ciclo que siga siendo infectado y siempre va a dar el mismo resultado, como hacer para mirarlo una vez?
            else if(personList[i].prob_death >= 0.7){

                changeEstate(i,5);

            }
            else if(personList[i].periodo_incu != 0){ //Miramos a ver si el infectado es contagioso

                personList[i].periodo_incu -= 1;

            }
            else{
                //miramos la probabilidad de contagio de esta persona. SI es por encima de 0.7 entonces contagia, si no, nada.
                if(prob_infection >=0.7) propagate(i);

            }
        }
        //Ahora lo muevo
        move(i);
        //Una vez que ya lo he movido, calculo las nuevas coordenadas.
        changeCoordenates(i);

    }

}

void changeEstate(int index_person,int new_state){

    personList[i].state = new_state;

}

void changeCoordenates(int index_person){

    //No hacer hasta responder las dudas del principio del docuemento.

}

void move(int index_person){

    //No hacer hasta responder las dudas del principio del documento

}

void propagate(int index_person){

    //Ahora tengo que mirar arriba,abajo,izquierda,derecha. Tengo que tener en cuenta los limites de la matriz
    fila = personList[i] / size_world;
    columna = personList[i] % size_world;
    //En ved de un bucle poner las cuatro posibilidades
    for( j= fila ; j < fila+2;j++){
        for(k=columna;k<columhna + 2;k++){
            if ((columna+1) <= size_world){
            
                if(person_List[(filea*size_world) + columna+1].state == 0 || person_List[(filea*size_world) + columna+1].state == 3){

                    changeEstate(i,contagiado); //No hay un estado de contagiado, que le ponemos estado 1 o estado 2. O lo hacemos random
                
                }
            }
        }
    }
}