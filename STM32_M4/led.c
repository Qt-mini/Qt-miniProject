#include "device_driver.h"

void LED_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0); 

	Macro_Write_Block(GPIOC->MODER, 0x3, 0x1, 10);

	Macro_Clear_Bit(GPIOC->OTYPER, 0);
	Macro_Clear_Bit(GPIOC->OTYPER, 1);
	Macro_Clear_Bit(GPIOC->OTYPER, 2);
	Macro_Clear_Bit(GPIOC->OTYPER, 3);
	Macro_Clear_Bit(GPIOC->OTYPER, 4);
	Macro_Clear_Bit(GPIOC->OTYPER, 5);
	Macro_Clear_Bit(GPIOC->OTYPER, 6);


	Macro_Clear_Bit(GPIOC->ODR, 0);
	Macro_Clear_Bit(GPIOC->ODR, 1);
	Macro_Clear_Bit(GPIOC->ODR, 2);
	Macro_Clear_Bit(GPIOC->ODR, 3);
	Macro_Clear_Bit(GPIOC->ODR, 4);
	Macro_Clear_Bit(GPIOC->ODR, 5);
	Macro_Clear_Bit(GPIOC->ODR, 6);
}

// void LED_UpdateAll(uint8_t mask){
// //??
// }

//입차, 출차 led 세팅
void LED_SetFloor(uint8_t floor, uint8_t state){
	if(state){
		Macro_Clear_Bit(GPIOC->ODR, floor);
	}
	else{
		Macro_Set_Bit(GPIOC->ODR, floor);
	}

}
