#include "pwm.h"
// FCY e 40MHZ
void initPwm(){
 TRISB&=(1<<PIN_SERVO); //setare RB10 ca output 

 P1TCONbits.PTOPS = 0; // Timer base output scale 
 P1TCONbits.PTMOD = 0; // Free running 
 P1TMRbits.PTDIR = 0; // Numara in sus pana cand timerul = perioada 
 P1TMRbits.PTMR = 0; // Baza de timp 

 P1DC3 = 0x7530; 
 
 P1TPER = 0x4E20; 

 PWM1CON1bits.PMOD3 = 1; // Canalele PWM3H si PWM3L sunt independente

 PWM1CON1bits.PEN3H = 1; // Pinul PWM1H setat pe iesire PWM 
 PWM1CON1bits.PEN3L = 0; // Pinul PWM1L setat ca gpio

 PWM1CON2bits.UDIS = 1; // Disable Updates from duty cycle and period buffers 
 PWM1CON2bits.SEVOPS = 0;
 P1TCONbits.PTEN = 1; /* Enable the PWM Module */ 
}