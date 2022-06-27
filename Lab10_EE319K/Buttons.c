#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "Sound.h"
#include "Timer1.h"

// Buttons.c
// Use edge triggered interrupts to read PF0 and PF4
// PF0: Clear button and PF4: Drop button

void Button_Init(void){volatile long delay;                           	  
  SYSCTL_RCGCGPIO_R |= 0x20;           // activate port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
  delay = 100;                  //    allow time to finish activating
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0                              // 2) GPIO Port F needs to be unlocked
  GPIO_PORTF_AMSEL_R &= ~0x11;  // 3) disable analog on PF4,0
                                // 4) configure PF4,0 as GPIO
  GPIO_PORTF_PCTL_R &= ~0x000F000F;
  GPIO_PORTF_DIR_R &= ~0x11;    // 5) make PF4,0 in
  GPIO_PORTF_AFSEL_R &= ~0x11;  // 6) disable alt funct on PF4,0
	GPIO_PORTF_PUR_R |= 0x11;     
  GPIO_PORTF_DEN_R |= 0x11;     // 7) enable digital I/O on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     //    PF4,0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //    PF4,0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;     //    PF4,0 falling edge event (Neg logic)
  GPIO_PORTF_ICR_R = 0x11;      //    clear flag1-0
  GPIO_PORTF_IM_R |= 0x11;      // 8) arm interrupt on PF4,0
                                // 9) GPIOF priority 2
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00400000;
  NVIC_EN0_R = 1<<30;   // 10)enable interrupt 30 in NVIC
}


