//*****************************************************************************
//
// MSP432 main.c template
// Heather Bradfield
// Dylan Hart
// Final Project
//
//****************************************************************************

#include "msp.h"
#include "PWM.h"
#include <math.h>
volatile int32_t last_time;
volatile int32_t front_time;
volatile int32_t back_time;
volatile int32_t forward_time;
int32_t h;
float theta;
float dist;
int32_t target_d = 0x10000;
float k = 100000;
float target_angle;
float gamma;

#define PI 3.14159

float min(float a, float b) {
    return a <= b ? a : b;
}

float max(float a, float b) {
    return a >= b ? a : b;
}

void delay(uint16_t us) {
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = 42*us;
    SysTick->VAL = 0;
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}

int32_t ping(uint16_t trig, uint16_t echo) {
    P5->OUT &= ~trig;
    delay(2);
    P5->OUT |= trig;
    delay(10);
    P5->OUT &= ~trig;

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    while (!(P5->IN & echo));
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    while (P5->IN & echo);
    int32_t time = 0xFFFFFF - SysTick->VAL;
    return time;
}

void main(void)
{
	PWM_Init(0x2000, 0x1999, 0x1999);
    P2->DIR |= 1<<6 | 1<<7;
    P2->OUT |= 1<<6 | 1<<7;

    P5->DIR |= 0x51;
    P5->DIR &= ~0xA2;

    //3.6 3.7
    P3->DIR |= 0xC0;
    P3->OUT |= 0xC0;

    while (1) {
        back_time = ping(0x10,0x20);
        delay(1000);
        front_time = ping(0x40,0x80);
        delay(1000);
        forward_time = ping(1,2);
        delay(1000);

        if (forward_time < 0x15000) {
            PWM_Duty2(0x1999);
            PWM_Duty1(0x00);
            continue;
        }

        h = (back_time + front_time)/2;
        theta = atan2(front_time - back_time, 0x8500);
        dist = cos(theta) * h;
        target_angle = max(-PI/2, min(PI/2, (target_d-dist)/k));
        gamma = (target_angle-theta)/PI;

        if (gamma >= 0) {
            PWM_Duty1((uint32_t) (0x1999*(1-gamma)));
            PWM_Duty2(0x1999);
            P3->OUT = 0x80;
        }
        else {
            PWM_Duty1(0x1999);
            PWM_Duty2((uint32_t) (0x1999*(1+gamma)));
            P3->OUT = 0x40;
        }

    }
}
