#include "device_driver.h"

#define TIM3_TIC         20                  // usec
#define TIM3_FREQ        (1000000./TIM3_TIC) // 50,000 Hz
#define TIM3_1ms_PLS     (TIM3_FREQ/1000.)   // 50 counts per 1ms

void TIM3_Delay(int time_ms){
    Macro_Set_Bit(RCC->APB1ENR, 1);

	TIM3->CR1 = (1<<4)|(1<<3);
	TIM3->PSC = (unsigned int)(TIMXCLK/TIM3_FREQ + 0.5)-1;
	TIM3->ARR = TIME3_PLS_OF_1ms * time;

	Macro_Set_Bit(TIM3->EGR,0);
	Macro_Clear_Bit(TIM3->SR, 0);
	Macro_Set_Bit(TIM3->CR1, 0);

	while(Macro_Check_Bit_Clear(TIM3->SR, 0));
	Macro_Clear_Bit(TIM3->CR1, 0);
}
