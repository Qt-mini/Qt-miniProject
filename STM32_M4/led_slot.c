#include "device_driver.h"

#define EMPTY_FLOOR   0b00000000
#define FULL_FLOOR    0b11111111
#define SLOT_ERROR    -1

static unsigned int g_slot_occupancy_mask = EMPTY_FLOOR;
int error_input(int floor);


int Slot_FindLowestEmpty(){
    if(g_slot_occupancy_mask == FULL_FLOOR){
        return SLOT_ERROR;
    }
    for (int i = 0; i < 8; i++) {
        if (Macro_Check_Bit_Clear(g_slot_occupancy_mask, i)) {
            return i; 
        }
    }
}
void Slot_SetOccupied(unsigned int floor){
    if (error_input(floor) || Macro_Check_Bit_Set(g_slot_occupancy_mask, floor)) {
        return;
    }
    Macro_Set_Bit(g_slot_occupancy_mask, floor);
}
void Slot_SetEmpty(unsigned int floor){
    if (error_input(floor) || Macro_Check_Bit_Clear(g_slot_occupancy_mask, floor)) {
        return;
    }
    Macro_Clear_Bit(g_slot_occupancy_mask, floor);
}

unsigned int get_slot(){
    return g_slot_occupancy_mask;
}


int error_input(int floor){
    if(floor >= 8 || floor < 0) return 1;
    return 0;
}

