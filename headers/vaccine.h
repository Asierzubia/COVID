#ifndef VACCINE_H
#define VACCINE_H

int num_persons_to_vaccine;
int percent;
int group_to_vaccine;
int person_vaccinned;
typedef enum {false, true} bool;
  
bool vacunate(int age);

#endif // VACCINE_H