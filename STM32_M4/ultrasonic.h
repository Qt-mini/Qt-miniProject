#ifndef ULTRASONIC_H
#define ULTRASONIC_H

typedef enum
{
    CAR_NONE = 0,
    CAR_DETECTED
} CarState;

void Ultrasonic_Init(void);
void Ultrasonic_UpdateCarState(void);
CarState Ultrasonic_GetCarState(void);

#endif