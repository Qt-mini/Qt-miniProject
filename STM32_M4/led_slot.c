#include "device_driver.h"

#define EMPTY_FLOOR   0b00000000
#define FULL_FLOOR    0b11111111
#define SLOT_ERROR    -1

extern volatile uint8_t g_slot_occupancy_mask;

int error_input(int floor);

// 가장낮은 층부터 비어있는 층반환 (1~8층 기준)
int Slot_FindLowestEmpty(){
    if (g_slot_occupancy_mask == FULL_FLOOR) {
        return SLOT_ERROR;
    }
    for (int i = 0; i < 8; i++) {
        if ((g_slot_occupancy_mask & (1U << i)) == 0U) {
            return i + 1;
        }
    }
    return SLOT_ERROR;
}
//입차시 해당층 슬롯 1 설정 
void Slot_SetOccupied(unsigned int floor){
    if (error_input((int)floor) || (floor >= 8) || (g_slot_occupancy_mask & (1U << floor))) {
        return;
    }
    g_slot_occupancy_mask |= (1U << floor);
}
//출차시 해당층 슬롯 0설정 
void Slot_SetEmpty(unsigned int floor){
    if (error_input((int)floor) || (floor >= 8) || ((g_slot_occupancy_mask & (1U << floor)) == 0U)) {
        return;
    }
    g_slot_occupancy_mask &= ~(1U << floor);
}
//전체 층 슬롯 반환
unsigned int get_slot(){
    return (unsigned int)g_slot_occupancy_mask;
}

// 올바르지 못한 층수일때 예외처리
int error_input(int floor){
    if(floor >= 8 || floor < 0) return 1;
    return 0;
}

