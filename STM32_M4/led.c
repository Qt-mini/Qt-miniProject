#include "device_driver.h"

extern volatile uint8_t g_slot_occupancy_mask;

void LED_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);

	Macro_Write_Block(GPIOC->MODER, 0xffff, 0x5555, 0);

	Macro_Clear_Bit(GPIOC->ODR, 0);
	Macro_Clear_Bit(GPIOC->ODR, 1);
	Macro_Clear_Bit(GPIOC->ODR, 2);
	Macro_Clear_Bit(GPIOC->ODR, 3);
	Macro_Clear_Bit(GPIOC->ODR, 4);
	Macro_Clear_Bit(GPIOC->ODR, 5);
	Macro_Clear_Bit(GPIOC->ODR, 6);

}

//입차, 출차 led 세팅
void LED_SetFloor(uint8_t floor, uint8_t state){
	if (floor >= 8) {
		return;
	}
	if(state){
		Macro_Set_Bit(GPIOC->ODR, floor);
	}
	else{
		Macro_Clear_Bit(GPIOC->ODR, floor);
	}
}

void LED_UpdateFromSlots(void)
{
	for (uint8_t i = 0; i < 8; i++) {
		uint8_t occupied = (g_slot_occupancy_mask & (1U << i)) ? 1U : 0U;
		LED_SetFloor(i, occupied);
	}
}
