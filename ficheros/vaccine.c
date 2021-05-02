#include "vaccine.h"
#include "environment.h"

percent = 0.7;
group_to_vaccine = 8;
num_person_to_vaccine = iterations / (size * percent);
person_vaccinned = 0;


int vacunate(person_t *person,int vaccines_left_t){
        
    if(person->age >=group_to_vaccine*10){
        person->state = 4;
        l_vaccined[id_contVaccined] = person;
        id_contVaccined++;
        //TODO:Eliminar de la lista de no infectados a esa persona
        person_vacinned++;
        return vaccines_left_t--;
    }
    
    return vaccines_left_t;
}