#include "../headers/entorno.h"

int i,j,position;
void init_world(){

    world = (person_t *)calloc(size_world*size_world,sizeof(person_t));

}

void create_enviroment(){

    //Inicializo el Array 1D que va a simular el mundo
    init_world();
    //Distribuyo las personas en el mundo
    for(i = 0; i < size; i++){
        //Si la posicion esta vacia metemos la persona dentro
        if(world[personList[i].init_position] == 0){
            
            world[personList[i].init_position] = personList[i];

        }
        else{ //Si no le calculamos otra posicion pseudoaleatoria

            j = 1;
            while(j == 1){

                position = random_number(1,size);
                if(world[position] == 0){
                    
                    personList[i].init_position = position;
                    world[position] = personList[i];
                    j = 2;

                }
            }
        }
    }
}