#include "device_driver.h"

#define EMPTY_FLOOR   0b00000000
#define FULL_FLOOR    0b11111111
#define SLOT_ERROR    -1

static unsigned int parking_tower = EMPTY_FLOOR;
int error_input(int floor);

// 가장낮은 층부터 비어있는 층반환
int Slot_FindLowestEmpty(){
    if(parking_tower == FULL_FLOOR){
        return SLOT_ERROR;
    }
    for (int i = 0; i < 8; i++) {
        if (Macro_Check_Bit_Clear(parking_tower, i)) {
            return i; 
        }
    }
}
//입차시 해당층 슬롯 1 설정 
void Slot_SetOccupied(unsigned int floor){
    if (error_input(floor) || Macro_Check_Bit_Set(parking_tower, floor)) {
        return;
    }
    Macro_Set_Bit(parking_tower, floor);
}
//출차시 해당층 슬롯 0설정 
void Slot_SetEmpty(unsigned int floor){
    if (error_input(floor) || Macro_Check_Bit_Clear(parking_tower, floor)) {
        return;
    }
    Macro_Clear_Bit(parking_tower, floor);
}
//전체 층 슬롯 반환
unsigned int get_slot(){
    return parking_tower;
}

// 올바르지 못한 층수일때 예외처리
int error_input(int floor){
    if(floor >= 8 || floor < 0) return 1;
    return 0;
}

