#include "device_driver.h"
#include "ultrasonic.h"
#include <stdint.h>

#define ECHO_WAIT_RISE    0
#define ECHO_WAIT_FALL    1

static void Ultrasonic_Trig_Init(void);
static void TIM5_Trig_Init(void);
static void TIM4_Echo_Init(void);

volatile uint32_t echo_rise = 0;
volatile uint32_t echo_fall = 0;
volatile uint8_t echo_done = 0;
volatile uint8_t echo_state = ECHO_WAIT_RISE;
volatile CarState car_state = CAR_NONE;

uint8_t detect_count = 0;
uint8_t release_count = 0;

void Ultrasonic_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR, 1);                 // GPIOB Clock on

    Ultrasonic_Trig_Init();                         // PB4 TRIG (LOW) 초기화
    TIM5_Trig_Init();                               // TIM5 TRIG 초기화
    TIM4_Echo_Init();                               // TIM4 ECHO 초기화
}

static void Ultrasonic_Trig_Init(void)
{
    Macro_Write_Block(GPIOB->MODER, 0x3, 0x1, 8);         // PB4 Output 설정
    Macro_Clear_Bit(GPIOB->OTYPER, 4);                    // Push-Pull 설정
    Macro_Clear_Bit(GPIOB->ODR, 4);                       // TRIG 초기 LOW
}

static void TIM5_Trig_Init(void)
{
    Macro_Set_Bit(RCC->APB1ENR, 3);                  // TIM5 Clock on

    TIM5->PSC = 15;                                  // 96MHz / 96 = 1MHz
    TIM5->ARR = 99999;                               // 100ms 주기
    TIM5->CCR1 = 10;                                 // 10us 후 Compare

    Macro_Set_Bit(TIM5->DIER, 0);                    // Update Interrupt Enable
    Macro_Set_Bit(TIM5->DIER, 1);                    // CC1 Interrupt Enable

    TIM5->SR = 0;                                    // Interrupt Flag Clear

    NVIC_EnableIRQ(TIM5_IRQn);                       // TIM5 Interrupt Enable

    Macro_Set_Bit(TIM5->CR1, 0);                     // TIM5 Start
}

static void TIM4_Echo_Init(void)
{
    Macro_Set_Bit(RCC->APB1ENR, 2);                  // TIM4 Clock Enable

    Macro_Write_Block(GPIOB->MODER, 0x3, 0x2, 12);  // PB6 Alternate Function
    Macro_Write_Block(GPIOB->AFR[0], 0xF, 0x2, 24); // PB6 AF2 = TIM4_CH1

    TIM4->PSC = 15;                                  // 96MHz / 96 = 1MHz
    TIM4->ARR = 0xFFFF;                              // 최대 Counter
    TIM4->CNT = 0;                                   // Counter 초기화

    Macro_Write_Block(TIM4->CCMR1, 0x3, 0x1, 0);     // CC1S = 01, Input Capture

    Macro_Clear_Bit(TIM4->CCER, 1);                  // Rising Edge 설정
    Macro_Set_Bit(TIM4->CCER, 0);                    // Capture Enable

    Macro_Set_Bit(TIM4->DIER, 1);                    // CC1 Interrupt Enable

    TIM4->SR = 0;                                    // Interrupt Flag Clear

    NVIC_EnableIRQ(TIM4_IRQn);                       // TIM4 Interrupt Enable

    Macro_Set_Bit(TIM4->CR1, 0);                     // TIM4 Start
}


void TIM5_IRQHandler(void)
{
    if (Macro_Check_Bit_Set(TIM5->SR, 0))            // Update Interrupt
    {
        Macro_Clear_Bit(TIM5->SR, 0);                // Update Flag Clear

        echo_done = 0;                               // 측정 완료 상태 초기화
        echo_state = ECHO_WAIT_RISE;                 // Rising 대기

        echo_rise = 0;                               // Rising 값 초기화
        echo_fall = 0;                               // Falling 값 초기화

        Macro_Clear_Bit(TIM4->CCER, 0);              // Capture Disable
        Macro_Clear_Bit(TIM4->CCER, 1);              // Rising Edge 설정
        Macro_Clear_Bit(TIM4->SR, 1);                // Capture Flag Clear

        TIM4->CNT = 0;                               // Timer Counter 초기화

        Macro_Set_Bit(TIM4->CCER, 0);                // Capture Enable

        Macro_Set_Bit(GPIOB->ODR, 4);                // TRIG HIGH

        TIM5->CCR1 = 10;                             // 10us 후 LOW
    }

    if (Macro_Check_Bit_Set(TIM5->SR, 1))            // Compare Interrupt
    {
        Macro_Clear_Bit(TIM5->SR, 1);                // Compare Flag Clear

        Macro_Clear_Bit(GPIOB->ODR, 4);              // TRIG LOW
    }
}

void TIM4_IRQHandler(void)
{
    if (Macro_Check_Bit_Set(TIM4->SR, 1))            // Capture Interrupt
    {
        uint32_t capture = TIM4->CCR1;               // Capture 값 읽기

        if (echo_state == ECHO_WAIT_RISE)            // Rising Edge
        {
            echo_rise = capture;                    // Rising 시간 저장
            echo_state = ECHO_WAIT_FALL;             // Falling 대기

            Macro_Clear_Bit(TIM4->CCER, 0);          // Capture Disable
            Macro_Set_Bit(TIM4->CCER, 1);            // Falling Edge 설정
            Macro_Clear_Bit(TIM4->SR, 1);            // Capture Flag Clear
            Macro_Set_Bit(TIM4->CCER, 0);            // Capture Enable
        }
        else                                         // Falling Edge
        {
            echo_fall = capture;                    // Falling 시간 저장
            echo_done = 1;                           // 측정 완료
            echo_state = ECHO_WAIT_RISE;             // 다음 Rising 대기

            Macro_Clear_Bit(TIM4->CCER, 0);          // Capture Disable
            Macro_Clear_Bit(TIM4->CCER, 1);          // Rising Edge 설정
            Macro_Clear_Bit(TIM4->SR, 1);            // Capture Flag Clear
            Macro_Set_Bit(TIM4->CCER, 0);            // Capture Enable
        }

        Macro_Clear_Bit(TIM4->SR, 1);                // Capture Flag Clear
    }
}

void Ultrasonic_UpdateCarState(void)
{
    uint32_t echo_time;
    int32_t distance;

    if (!echo_done)                                  // 측정 완료 전
    {
        return;                                      // 함수 종료
    }

    if (echo_fall >= echo_rise)                      // Overflow 없음
    {
        echo_time = echo_fall - echo_rise;           // Echo HIGH 시간
    }
    else                                             // Overflow 발생
    {
        echo_time = (0xFFFF - echo_rise) + echo_fall + 1;  // Overflow 보정
    }

    echo_done = 0;                                   // 측정 결과 처리 완료

    distance = (echo_time * 1715) / 10000;            // 거리 계산

    if (distance <= 150)                             // 150mm 이하
    {
        detect_count++;                              // 감지 횟수 증가
        release_count = 0;                           // 해제 횟수 초기화

        if (detect_count >= 3)                       // 3회 연속 감지
        {
            detect_count = 3;                        // 최대값 유지
            car_state = CAR_DETECTED;                // 차량 감지
        }
    }
    else if (distance >= 200)                        // 200mm 이상
    {
        release_count++;                             // 해제 횟수 증가
        detect_count = 0;                            // 감지 횟수 초기화

        if (release_count >= 3)                      // 3회 연속 해제
        {
            release_count = 3;                       // 최대값 유지
            car_state = CAR_NONE;                    // 차량 없음
        }
    }
    else                                             // 150~200mm
    {
        detect_count = 0;                            // 감지 횟수 초기화
        release_count = 0;                           // 해제 횟수 초기화
    }
}

CarState Ultrasonic_GetCarState(void)
{
    return car_state;                                // 현재 차량 상태 반환
}