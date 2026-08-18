#include "device_driver.h"
#include <stdio.h>
#include <string.h>

extern void init_servo_motor(void);
extern void Servo_MoveBarrier(void);

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
	init_servo_motor();
}

void Main(){
	Sys_Init(115200);
	printf("ttdddd");
	
    while(1){
		
		Servo_MoveBarrier();
		printf("aaaasdsds");
	}
}
