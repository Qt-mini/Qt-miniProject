#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define UART_RX_BUFFER_SIZE 64

/* ===================================================================
 * [0] main.c 자원 참조 (extern 선언)
 * =================================================================== */
extern volatile uint8_t  g_slot_occupancy_mask;
extern volatile uint8_t  g_is_barrier_open;
extern volatile uint32_t g_barrier_timer;
extern uint32_t GetTick(void);
extern int8_t   Slot_FindLowestEmpty(void);

static char s_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t s_rx_index = 0;
static char s_cmd_buffer[UART_RX_BUFFER_SIZE];
static volatile bool s_cmd_ready = false;

/* ===================================================================
 * [1] USART2 초기화 및 전송 함수
 * =================================================================== */
void UART2_Init(uint32_t pclk1_hz, uint32_t baud) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER &= ~( (3U << (2 * 2)) | (3U << (3 * 2)) );
    GPIOA->MODER |=  ( (2U << (2 * 2)) | (2U << (3 * 2)) );

    GPIOA->AFR[0] &= ~( (0xFU << (2 * 4)) | (0xFU << (3 * 4)) );
    GPIOA->AFR[0] |=  ( (7U << (2 * 4)) | (7U << (3 * 4)) );

    USART2->BRR = (pclk1_hz + (baud / 2U)) / baud;

    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
}

void UART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

void UART2_SendString(const char *str) {
    while (*str) {
        UART2_SendChar(*str++);
    }
}

/* ===================================================================
 * [2] 수신 버퍼 제어 API
 * =================================================================== */
bool UART2_IsCommandReady(void) {
    return s_cmd_ready;
}

void UART2_GetCommand(char *out_buf) {
    if (out_buf != NULL) {
        strcpy(out_buf, s_cmd_buffer);
    }
    s_cmd_ready = false;
}

/* ===================================================================
 * [3] 수신 명령 파싱 및 처리 로직 (최종 프로토콜 규격 반영)
 * =================================================================== */
void UART_ParseCommand(char *cmd) {
    char tx_buf[64];

    // [1] 입차 요청: "I:<car4>" (예: "I:1234") 또는 하위 호환 "I"
    if (strncmp(cmd, "I:", 2) == 0 || strcmp(cmd, "I") == 0) {
        int8_t empty_floor = Slot_FindLowestEmpty();
        if (empty_floor != -1) {
            g_slot_occupancy_mask |= (1U << (empty_floor - 1));

            g_is_barrier_open = 1;
            g_barrier_timer = GetTick();

            snprintf(tx_buf, sizeof(tx_buf), "P:%d\r\n", empty_floor);
            UART2_SendString(tx_buf);
        } else {
            // 만차 시 에러 응답 (E:3)
            UART2_SendString("E:3\r\n");
        }
    }
    // [2] 정산 완료/출차 요청: "O:<floor>" (예: "O:3")
    else if (strncmp(cmd, "O:", 2) == 0) {
        int floor = atoi(cmd + 2);

        // 1) 층 번호 범위 검증 (1~8층 벗어남: 에러 코드 1)
        if (floor < 1 || floor > 8) {
            UART2_SendString("E:1\r\n");
        }
        // 2) 해당 층이 이미 비어 있는 경우 (빈 슬롯 출차 시도: 에러 코드 2)
        else if (!(g_slot_occupancy_mask & (1 << (floor - 1)))) {
            UART2_SendString("E:2\r\n");
        }
        // 3) 정상 출차 완료
        else {
            g_slot_occupancy_mask &= ~(1U << (floor - 1)); // 슬롯 비움

            g_is_barrier_open = 1;                        // 차단기 개방
            g_barrier_timer = GetTick();                  // 자동 닫힘 타이머 시작

            // 출차 완료 단일 응답 "E\r\n"
            UART2_SendString("E\r\n");
        }
    }
    // [3] 슬롯 점유 상태 조회: "?"
    else if (strcmp(cmd, "?") == 0) {
        char bitmask[9];
        // Bit7(8층)부터 Bit0(1층)까지 역순으로 문자열 구성 (예: Bit0=1 -> S:00000001)
        for (int i = 7; i >= 0; i--) {
            bitmask[7 - i] = (g_slot_occupancy_mask & (1 << i)) ? '1' : '0';
        }
        bitmask[8] = '\0';

        snprintf(tx_buf, sizeof(tx_buf), "S:%s\r\n", bitmask);
        UART2_SendString(tx_buf);
    }
    // [4] 알 수 없는 비정상 명령
    else {
        UART2_SendString("E:0\r\n");
    }
}

/* ===================================================================
 * [4] USART2 인터럽트 서비스 루틴 (ISR)
 * =================================================================== */
void USART2_IRQHandler(void) {
    if (USART2->SR & USART_SR_RXNE) {
        char ch = (char)(USART2->DR & 0xFF);

        if (ch == '\n' || ch == '\r') {
            if (s_rx_index > 0) {
                s_rx_buffer[s_rx_index] = '\0';
                strcpy(s_cmd_buffer, s_rx_buffer);
                s_cmd_ready = true;
                s_rx_index = 0;
            }
        } else {
            if (s_rx_index < UART_RX_BUFFER_SIZE - 1) {
                s_rx_buffer[s_rx_index++] = ch;
            }
        }
    }
}