#include "device_driver.h"

#define TIM3_TIC         20                  // usec
#define TIM3_FREQ        (1000000./TIM3_TIC) // 50,000 Hz
#define TIM3_1ms_PLS     (TIM3_FREQ/1000.)   // 50 counts per 1ms

void TIM3_Delay(int time_ms){
    Macro_Set_Bit(RCC->APB1ENR, 1);

    // One-pulse mode (OPM: bit 3), Downcounter (DIR: bit 4)
    TIM3->CR1 = (1 << 4) | (1 << 3);
    TIM3->PSC = (unsigned int)(TIMXCLK / TIM3_FREQ + 0.5) - 1;
    TIM3->ARR = (unsigned int)TIM3_1ms_PLS; // 1ms 단위로 카운트 설정

    // 요청한 ms만큼 반복 실행
    for(int i = 0; i < time_ms; i++){
        Macro_Set_Bit(TIM3->EGR, 0);          // UG = 1 (Update Event 생성)
        Macro_Clear_Bit(TIM3->SR, 0);         // UIF 플래그 클리어
        Macro_Set_Bit(TIM3->CR1, 0);          // CEN = 1 (타이머 시작)

        while(Macro_Check_Bit_Clear(TIM3->SR, 0)); // 1ms 대기
        Macro_Clear_Bit(TIM3->CR1, 0);        // CEN = 0
    }
}

