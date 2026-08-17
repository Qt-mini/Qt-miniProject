#include "device_driver.h"

#define first_floor   0b00000001
#define second_floor  0b00000010
#define third_floor   0b00000100
#define fourth_floor  0b00001000
#define fifth_floor   0b00010000
#define sixth_floor   0b00100000
#define seventh_floor 0b01000000
#define eighth_floor  0b10000000
#define empty_floor   0b00000000
#define FULL_FLOOR    0b11111111
#define SLOT_ERROR    -1

static const unsigned int floor_mask[8] = {first_floor, second_floor, third_floor, 
    fourth_floor, fifth_floor, sixth_floor, seventh_floor, eighth_floor};

static unsigned int g_slot_occupancy_mask = 0b00000000;

unsigned int Slot_FindLowestEmpty(){
    if(g_slot_occupancy_mask == FULL_FLOOR){
        return SLOT_ERROR;
    }
    for (int i = 0; i < 8; i++) {
        if ((g_slot_occupancy_mask & floor_mask[i]) == 0) {
            return i+1; 
        }
    }
    return SLOT_ERROR;
}
// void Slot_SetOccupied(unsigned int floor){

// }
// void Slot_SetEmpty(unsigned int floor){

// }

