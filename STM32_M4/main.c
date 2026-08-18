#include "device_driver.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* ===================================================================
 * [1] 외부 함수 선언 (extern)
 * =================================================================== */
extern void UART2_Init(uint32_t pclk1_hz, uint32_t baud);
extern void UART2_SendChar(char c);
extern void UART2_SendString(const char *str);
extern bool UART2_IsCommandReady(void);
extern void UART2_GetCommand(char *out_buf);
extern void UART_ParseCommand(char *cmd);
extern void init_servo_motor(void);
extern void Servo_MoveBarrier(void);

/* ===================================================================
 * [2] 전역 상태 및 타이머 변수
 * =================================================================== */
volatile uint8_t  g_slot_occupancy_mask = 0x00; 
volatile uint8_t  g_is_barrier_open = 0;
volatile uint32_t g_barrier_timer = 0;

static volatile uint32_t g_tick_count = 0;

/* ===================================================================
 * [3] SysTick 정밀 타이머
 * =================================================================== */
void SysTick_Init(void) {
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U; // 1ms 주기
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    g_tick_count++;
}

uint32_t GetTick(void) {
    return g_tick_count;
}

/* ===================================================================
 * [4] 사용자 버튼 (PC13 - 차량 감지 가상 센서)
 * =================================================================== */
void Button_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER &= ~(3U << (13 * 2)); // PC13 입력 모드
}

uint8_t Button_IsPressed(void) {
    return !(GPIOC->IDR & (1U << 13));
}

/* ===================================================================
 * [5] 메인 루프
 * =================================================================== */
int Main(void) {
    SystemCoreClockUpdate();
    SysTick_Init();
    Button_Init();
    LED_Init();
    

    // USART2 초기화 (115200bps, 8-N-1)
    UART2_Init(48000000U, 115200U);
    init_servo_motor();
    // [프로토콜 R 전송] 부팅 완료 + 초기화 완료 알림
    UART2_SendString("R\r\n");
    //LED_UpdateFromSlots();

    char cmd_buffer[64];

    while (1) {
        Servo_MoveBarrier();
    
        UART2_SendString("Ready\r\n");
        // 1. UART 수신 명령 파싱 및 처리
        if (UART2_IsCommandReady()) {
            UART2_GetCommand(cmd_buffer);
            UART_ParseCommand(cmd_buffer);
            LED_UpdateFromSlots();
        }
    }
}