#ifndef PWM_H
#define PWM_H

#include "p33fxxxx.h"

#define PIN_SERVO 10 //PWM1H3
#define PWM_SERVO_MIN 1230
#define PWM_SERVO_MAX 2460
#define PWM_SERVO_MID (PWM_SERVO_MIN+PWM_SERVO_MAX)/2
void initPwm();

#endif
