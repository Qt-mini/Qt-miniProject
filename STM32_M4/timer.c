#include "device_driver.h"

#define TIM3_TIC         20                  // usec
#define TIM3_FREQ        (1000000./TIM3_TIC) // 50,000 Hz
#define TIM3_1ms_PLS     (TIM3_FREQ/1000.)   // 50 counts per 1ms
#define TIM_SET           16000000
void TIM3_Delay(int time_ms)
{
    if (time_ms <= 0) return;

    // 1. TIM3 클럭 인가
    Macro_Set_Bit(RCC->APB1ENR, 1);

    // 2. 1ms 주기로 카운트 설정
    TIM3->CR1 = (1 << 4) | (1 << 3); // Downcounter, One-pulse mode
    TIM3->PSC = (unsigned int)(TIMXCLK / TIM3_FREQ + 0.5) - 1;
    TIM3->ARR = (uint16_t)TIM3_1ms_PLS; // 1ms당 50 카운트

    // 3. time_ms 만큼 1ms 지연을 반복 실행
    for (int i = 0; i < time_ms; i++)
    {
        Macro_Set_Bit(TIM3->EGR, 0);     // UG: Update Generation (레지스터 갱신)
        Macro_Clear_Bit(TIM3->SR, 0);    // UIF 플래그 클리어
        Macro_Set_Bit(TIM3->CR1, 0);     // 타이머 시작 (CEN = 1)

        // 1ms 카운트 완료(UIF 플래그 셋) 대기
        while (Macro_Check_Bit_Clear(TIM3->SR, 0));

        Macro_Clear_Bit(TIM3->CR1, 0);   // 타이머 정지 (CEN = 0)
    }
}