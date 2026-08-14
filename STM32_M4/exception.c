#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}
extern volatile int TIM2_timeout;

void TIM2_IRQHandler(void)
{
	TIM2_Stopwatch_Stop();
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x2, 14);
	Macro_Clear_Bit(TIM2->SR, 0);
	NVIC_ClearPendingIRQ(28);
	TIM2_timeout = 1;

}

extern volatile int DMA1_STREAM5_DONE;

void DMA1_Stream5_IRQHandler(void)
{
	TIM2_Stopwatch_Start();
	DMA1->HIFCR = 0x3F << 6;
	TIM3->CCR2 = 0;
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
    Macro_Write_Block(GPIOC->MODER, 0x3, 0x1, 14);
    Macro_Clear_Bit(GPIOC->ODR, 7); 
	NVIC_ClearPendingIRQ(16);  
    DMA1_STREAM5_DONE = 1;
}
