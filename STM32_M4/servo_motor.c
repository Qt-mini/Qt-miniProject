#include "device_driver.h"

void init_servo_motor(void){
    // 1. GPIOA 및 TIM2 클럭 인가
    Macro_Set_Bit(RCC->AHB1ENR, 0);     // GPIOA Clock Enable
    Macro_Set_Bit(RCC->APB1ENR, 0);     // TIM2 Clock Enable

    // 2. PA0 핀을 Alternate Function 모드로 설정
    Macro_Write_Block(GPIOA->MODER, 0x3, 2, 0);     // PA0 -> Mode 2 (AF)
    Macro_Write_Block(GPIOA->AFR[0], 0xF, 1, 0);    // PA0 -> AF1 (TIM2_CH1, 4비트 폭)

    // 3. 50Hz (20ms) 주기 설정 (96MHz 기준)
    TIM2->PSC = 95;
    TIM2->ARR = 19999;
    TIM2->CCR1 = 1000;                              // 초기 펄스폭: 1ms (0도)

    // 4. PWM 모드 1 설정 및 채널 1 출력 켜기
    // OC1M = 110 (PWM Mode 1, 비트 4~6), OC1PE = 1 (Preload Enable, 비트 3)
    TIM2->CCMR1 |= (6 << 4) | (1 << 3); 
    TIM2->CCER  |= (1 << 0);                        // CC1E = 1 (Channel 1 Output Enable)

    // 5. 타이머 카운터 활성화 (카운트 시작)
    TIM2->CR1   |= (1 << 0);                        // CEN = 1
}

// 차단기 동작
void Servo_MoveBarrier(void){
    TIM2->CCR1 = 1500;  // 1.5ms (90도)
    TIM3_Delay(3000);
    TIM2->CCR1 = 1000;  // 1.0ms (0도)
}