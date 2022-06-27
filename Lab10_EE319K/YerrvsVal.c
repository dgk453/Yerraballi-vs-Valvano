// Yerraballi vs Valvano.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/12/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"
#include "Buttons.h"
#include "Dac.h"
#include <math.h>

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
	
const double pi = 22.0 / 7.0;

uint32_t Data;
uint8_t Ang = 0;
uint8_t Flag = 0;
uint8_t Turn=0;
double Angle;
double Velocity;
uint8_t Player1Count=0;
uint8_t Player2Count=0;
int winnerFlag = 0;
int doneFlag = 0;
int outsideFlag = 0;
int langFlag = 0;



typedef struct{
  int x;       // x position of player
  int y;   // y position of player
}Player_t;

typedef struct{
  int x;       // x position of player
  int y;   // y position of player
	int width;	//width of building
	int height;	//height of building
	uint16_t color;	//color of building
}Building_t;

typedef struct{
  int x;       // x position of object
  int y;   // y position of object
	int dx;		// x velocity of object
	int dy;		// y velocity of object
}Object_t;

Player_t Player1, Player2;
Object_t microCont1, microCont2;
Building_t Buildings[8];
const uint16_t Colors[4] = {0x0000, 0x0008,0x0800, 0x1000};



void Player1Turn(void);
void Player2Turn(void);
void Display (void);

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay1ms(uint32_t n);



// Initliaze Port E and make PE0 and input switch
void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10;      // activate port E
  while((SYSCTL_PRGPIO_R&0x10) != 0x10){};
  GPIO_PORTE_DIR_R &= ~0x03;   // input on PE0
  GPIO_PORTE_DEN_R |=  0x03;   // enable digital I/O on PE0
}

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;      // activate port F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 (built-in LED)
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |=  0x1E;   // enable digital I/O on PF
}


// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(uint32_t period){
 // write this (copy from Lab 8)
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
	NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
	NVIC_ST_CTRL_R |= 0x02;
	NVIC_ST_RELOAD_R = period;   //number of counts to wait
	NVIC_ST_CURRENT_R = 0;       //any value written to CURRENT clears
	while((NVIC_ST_CTRL_R &0x00010000)==0) {}  //wait for the count flag
}

uint32_t ConvertAngle(uint32_t x){
	return ((91*x)/4096);
}

uint32_t ConvertVelocity(uint32_t x){
	return ((101*x)/4096);
}

void BeginningScreen(void){
	ST7735_SetCursor(4,0);
	ST7735_OutString("Choose Language");
	ST7735_SetCursor(1,3);
	ST7735_OutString("Escoje 1 - Espa");
	ST7735_OutChar(0xA4);
	ST7735_OutString("ol");
	ST7735_SetCursor(1,5);
	ST7735_OutString("Press 2 - English");
	while(1){
		if((GPIO_PORTE_DATA_R&0x01)==1){
			ST7735_FillScreen(0x0000);   			// set screen to black
			ST7735_SetCursor(4,2);
			ST7735_OutString("Bienvenidos a\n\n      Yerraballi \n          vs\n        Valvano");
			Delay1ms(5000);
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(1,2);
			ST7735_OutString("La primera persona\n    en golpear al \noponente 3 veces es \n      el GANADOR");
			langFlag = 0;
			break;
		}
		else if((GPIO_PORTE_DATA_R&0x02)==0x02){
			ST7735_FillScreen(0x0000);            // set screen to black
			ST7735_SetCursor(6,2);
			ST7735_OutString("Welcome to\n\n      Yerraballi \n          vs\n        Valvano");
			Delay1ms(5000);
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(1,2);
			ST7735_OutString("The first person to\n    hit opponent 3 \n     times is the \n        WINNER");
			langFlag = 1;
			break;
		}
	}
}	

void Display(void){
	uint32_t display;
	uint8_t leave = 0;
	ST7735_SetCursor(4,0);
	if(Turn==0){
		ST7735_OutString("P1");
	}else{
		ST7735_OutString("P2");
	}	
	ST7735_SetCursor(2,1);
	ST7735_OutString("Ang: ");		
	ST7735_SetCursor(1,2);
	ST7735_OutString("Velo:");
//Process of Displaying the Angle
	while((GPIO_PORTE_DATA_R & 0x01)!=0){}
	while(leave==0){
	while(Flag==0){} // check semaphore		Flag = 0;
		Flag = 0;
		ST7735_SetCursor(7,1);
		display = ConvertAngle(Data);
		LCD_OutDec(display);     //Output to LCD
		if((GPIO_PORTE_DATA_R & 0x01)!=0){
			Angle = display;
			Ang = 1;
			leave = 1;
		}
	}
	leave = 0;
	while((GPIO_PORTE_DATA_R & 0x01)!=0){};
//Process of Displaying the Velocity  
	while(leave==0){    
		while(Flag==0){} // check semaphore
		Flag = 0;
		ST7735_SetCursor(6,2);
    display = ConvertVelocity(Data);
		if(display<100){
			ST7735_OutChar(0x20);
		}
    LCD_OutDec(display);     //Output to LCD
		if((GPIO_PORTE_DATA_R & 0x01)!=0){    
		Velocity = display;
		Ang = 0;
		leave = 1;
		}
	}
}
void buildingFunc(void){
	uint32_t m = 0;
	int n = 0;
	int buildingX = 0;
	for (int buildingI = 0; buildingI < 8; buildingI++){
	m = Random()%40;
	m += 50;
	n = Random()%3;
	Buildings[buildingI].height = 128 - m;
	Buildings[buildingI].x = buildingX;
	Buildings[buildingI].y = m;
	Buildings[buildingI].width = 20;
	buildingX += 20;
	ST7735_FillRect(Buildings[buildingI].x,Buildings[buildingI].y,Buildings[buildingI].width,Buildings[buildingI].height, Colors[n]); /// change colors
	}
	Player1.x = Buildings[0].x;
	Player1.y = Buildings[0].y-1;
	ST7735_DrawBitmap(Player1.x, Player1.y, Yerraballi, 15, 20);
	
	Player2.x = Buildings[7].x;
	Player2.y = Buildings[7].y-1;
	ST7735_DrawBitmap(Player2.x, Player2.y, Valvano, 15, 20);
}

Player_t playerPlacement(Building_t Building[], Player_t player){
	if (Turn == 0){
	player.x = Building[0].x;
	player.y = Building[0].y-1;
	ST7735_DrawBitmap(player.x, player.y, Yerraballi, 15, 20);
	} else if (Turn == 1){
	player.x = Building[7].x;
	player.y = Building[7].y-1;
	ST7735_DrawBitmap(player.x, player.y, Valvano, 15, 20);
	}
	return player;
}

Object_t microContPlacement(Object_t microCont, Building_t Building[], Player_t player){
	player = playerPlacement(Building, player);
	if (Turn == 0){
	microCont.x = player.x + 15;
	microCont.y = player.y - 20;
	} else if (Turn == 1){
	microCont.x = player.x - 8;
	microCont.y = player.y - 20;
	}
	ST7735_DrawBitmap(microCont.x, microCont.y, Micro, 8, 10);
	return microCont;
}


Object_t moveFunc(int velocityX, int velocityY, Object_t microCont){
	ST7735_FillRect(microCont.x, microCont.y-10, 8, 10, 0xFE21);
	microCont.x += velocityX;
	microCont.y -= velocityY;
	ST7735_DrawBitmap(microCont.x, microCont.y, Micro, 8, 10);
	return microCont; 
}

int collisionDetect(int index, Building_t Buildings[], Object_t microCont){
	if ((microCont.x < (Buildings[index].x + Buildings[index].width) && microCont.x > Buildings[index].x) 
			|| ((microCont.x + 8) < (Buildings[index].x + Buildings[index].width) && (microCont.x+8) > Buildings[index].x)){
			if ((microCont.y < (Buildings[index].y + Buildings[index].height) && microCont.y > Buildings[index].y)
				|| ((microCont.y-10) < (Buildings[index].y + Buildings[index].height) && (microCont.y-10) > Buildings[index].y)){
					return 1;
			}	
		}
		return 0;
}

int collisionDetectPlayer(Player_t player, Object_t microCont){
	if ((microCont.x < (player.x + 15/*player width*/) && microCont.x > player.x) 
			|| ((microCont.x + 8) < (player.x + 15/*player width*/) && (microCont.x+8) > player.x)){
			if ((microCont.y > (player.y - 20/*player height*/) && microCont.y < player.y)
				|| ((microCont.y - 10) > (player.y - 20/*player height*/) && (microCont.y-10) < player.y)){
					return 1;
			}	
		}
		return 0;
}



	
int microContMovement(Object_t microCont, Player_t player, Building_t Buildings[]){
	//int tempMicroContX = 0;
	int collisionFlag = 0;
	int buildI = 0;
	int winnerFlag = 0;
	
	//add player1 stuff if not working
	
	player = playerPlacement(Buildings, player);
	if (Turn == 0){
	microCont1 = microContPlacement(microCont, Buildings,player);
	Player1 = playerPlacement(Buildings, player);
	} else if(Turn == 1){
	microCont2 = microContPlacement(microCont, Buildings,player);
	Player2 = playerPlacement(Buildings, player);
	}
	
	microCont = microContPlacement(microCont, Buildings,player);
	
	
	while (microCont.y > 0){
		outsideFlag = 0;
		
		if (Turn == 0){
		microCont = microCont1;						//update microCont
		} else if(Turn == 1){
		microCont = microCont2;
		}
		buildI = microCont.x/20;
		
		
		collisionFlag = collisionDetectPlayer(Player1, microCont);
		if (collisionFlag==1){
			if (Turn == 0){
				winnerFlag = 2;
			}else if (Turn == 1){
				winnerFlag = 2;
			}
			Timer1A_Stop();
			break;
		}
		
		collisionFlag = collisionDetectPlayer(Player2, microCont);
		if (collisionFlag==1){
			if (Turn == 0){
				winnerFlag = 1;
			}else if (Turn == 1){
				winnerFlag = 1;
			}
			Timer1A_Stop();
			break;
		}
		
		collisionFlag = collisionDetect(buildI,Buildings,microCont);
		if (collisionFlag==1 || microCont.x >= 160 || microCont.x <= 0){
			Timer1A_Stop();
			break;
		}
		Timer1A_Start();
  }	
	
	
	while (microCont.y < 0){
		if (collisionFlag==1 || microCont.x >= 160|| microCont.x <= 0){
			Timer1A_Stop();
			break;
		}
		if (Turn == 0){
		microCont = microCont1;						//update microCont
		} else if(Turn == 1){
		microCont = microCont2;
		}
		outsideFlag = 1;
		Timer1A_Start();
		
	
		/*if((tempMicroContX < (microCont.x%20))&& buildI<8){
		buildI++;
		}
		tempMicroContX = microCont.x % 20;*/
	}
	
	while (microCont.y > 0){
		if (collisionFlag==1|| microCont.x >= 160 || microCont.x <= 0){
			Timer1A_Stop();
			break;
		}
		if (Turn == 0){
		microCont = microCont1;						//update microCont
		} else if(Turn == 1){
		microCont = microCont2;
		}
		outsideFlag = 0;
		
		buildI = microCont.x/20;
		

		
		collisionFlag = collisionDetectPlayer(Player1, microCont);
		if (collisionFlag==1){
			if (Turn == 0){
				winnerFlag = 2;
			}else if (Turn == 1){
				winnerFlag = 2;
			}
			Timer1A_Stop();
			break;
		}
		
		collisionFlag = collisionDetectPlayer(Player2, microCont);
		if (collisionFlag==1){
			if (Turn == 0){
				winnerFlag = 1;
			}else if (Turn == 1){
				winnerFlag = 1;
			}
			Timer1A_Stop();
			break;
		}
		
		collisionFlag = collisionDetect(buildI,Buildings,microCont1);
		if (collisionFlag==1 || microCont.x >= 160){
			Timer1A_Stop();
			break;
		}
		
		Timer1A_Start();
		/*if((tempMicroContX < (microCont.x%20))&& buildI<8){
		buildI++;
		}
		tempMicroContX = microCont.x % 20;*/
	}
	
	if (collisionFlag == 1){
		ST7735_DrawBitmap(microCont.x, microCont.y, Explode, 9, 9);
		playsound(Explosion);
		Delay1ms(3000);
		ST7735_DrawCircle(microCont.x, microCont.y-9, 0xfe21);
		doneFlag = 1;
		return winnerFlag;
	}
	doneFlag = 1;
	return winnerFlag;
}


void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
	PF3 ^= 0x08;
	if (Turn == 0){
		if (outsideFlag == 0){
		microCont1 = moveFunc(microCont1.dx,microCont1.dy, microCont1);
		microCont1.dy -= 10;
		Timer1A_Stop();
		}
		else if(outsideFlag == 1){
		microCont1.dy -= 10;
		microCont1.x += microCont1.dx;						// continues to change velocity when outside of screen
		microCont1.y += microCont1.dy;
		Timer1A_Stop();
		}
	}
	else if (Turn == 1){
		if (outsideFlag == 0){
		microCont2 = moveFunc(microCont2.dx,microCont2.dy, microCont2);
		microCont2.dy -= 10;
		}
		else if(outsideFlag == 1){
		microCont2.dy -= 10;
		microCont2.x -= microCont2.dx;						// continues to change velocity when outside of screen
		microCont2.y += microCont2.dy;
		}
	}
}

void GPIOPortF_Handler(void){
	PF2 ^= 0x04;
	if ((GPIO_PORTF_RIS_R & 0x10)==0x10){ //PF4
		ST7735_FillRect(0,0,160, 30, 0xFE21);
		playsound(Grunt);
		if (Turn == 0){
		winnerFlag = microContMovement(microCont1, Player1, Buildings);
		}
		else if (Turn == 1){
		winnerFlag = microContMovement(microCont2, Player2, Buildings);
		}
	}	
  GPIO_PORTF_ICR_R = 0x11;      // acknowledge flag4
}

int main(void){
	DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);
	ADC_Init();    // initialize to sample ADC
  DAC_Init();
	PortF_Init();
	PortE_Init();
	Button_Init();
	Output_Init();
//Enable SysTick Interrupt by calling SysTick_Init()
	SysTick_Init(80000000/10);
	Timer1_Init(80000000, 1);  //Initiates timer used for edge triggered ISR
	Timer1A_Stop();
  						// Disable until we want to shoot microcontroller
	BeginningScreen();
	Delay1ms(20000);
	ST7735_SetRotation(1);
	ST7735_FillScreen(0xFE21);            // set screen to background color // Spanish doesnt work yet because reusing port f button
	//ST7735_DrawBitmap(50, 50, test, 22,34);
		
	EnableInterrupts();
	
	while(Player1Count != 3 && Player2Count != 3){
		Delay1ms(10000);
		ST7735_SetRotation(1);
		ST7735_FillScreen(0xFE21);
		buildingFunc();  
		Turn = 0;
		while(1){
			if(Turn==0){
				Player1Turn();
				if (winnerFlag == 1){
					Player1Count += 1;
					break;
				}
				else if (winnerFlag == 2){
					Player2Count += 1;
					break;
				}
			}
	
			else{
				Player2Turn();
				if (winnerFlag == 1){
					Player1Count += 1;
					break;
				}
				else if (winnerFlag == 2){
					Player2Count += 1;
					break;
				}
			}
		}
		ST7735_SetRotation(0);
		ST7735_FillScreen(0X0000);
		Delay1ms(3000);
		
		if (langFlag == 1){
			if (Player1Count == 3){
				ST7735_FillScreen(0xFE21);
				ST7735_SetCursor(3, 2);
				ST7735_OutString("Yerraballi Wins!");
				ST7735_DrawBitmap(44,102,YerrEng,40,53);
				break;
			} else if (Player2Count == 3){
				ST7735_FillScreen(0x0000);
				ST7735_SetCursor(3, 2);
				ST7735_OutString("Valvano Wins!");
				break;
			} else if(winnerFlag ==1){
				ST7735_SetCursor(3, 2);
				ST7735_OutString("Yerraballi Wins \n\n\n       this round");
			} else if(winnerFlag == 2){
				ST7735_SetCursor(5, 2);
				ST7735_OutString("Valvano Wins \n\n\n       this round");
			}
		}
		
		else if (langFlag == 0){
			if (Player1Count == 3){
				ST7735_FillScreen(0x0000);
				ST7735_SetCursor(3, 2);
				ST7735_OutChar(0xAD);
				ST7735_OutString("Yerraballi Gana!");
				ST7735_DrawBitmap(44,102,YerrEng,40,53);
				  break;
			} else if (Player2Count == 3){
				ST7735_FillScreen(0x0000);
				ST7735_SetCursor(3, 2);
				ST7735_OutChar(0xAD);
				ST7735_OutString("Valvano Gana!");
				break;
			} else if(winnerFlag ==1){
				ST7735_SetCursor(3, 2);
				ST7735_OutString("Yerraballi gana \n\n\n      este ronda");  //should be in spanish
			} else if(winnerFlag == 2){
				ST7735_SetCursor(5, 2);
				ST7735_OutString("Valvano gana \n\n\n       este ronda");
			}
		}
		Delay1ms(3000);
	}
}

void DisplayScore(void){
	ST7735_SetCursor(11,12);
	LCD_OutDec(Player1Count);
	ST7735_OutString(" vs ");
	LCD_OutDec(Player2Count);
}

void SysTick_Handler(void){ // every 100 ms
  PF1 ^= 0x02;  // Heartbeat on transmit message
	Data = ADC_In();      // Sample ADC
	Flag = 1;
}	

//Player 1 function
void Player1Turn(void){
	DisplayScore();
	Display();
	//Delay100ms(10);
	Angle *= pi;
	Angle /= 180;
	Velocity = Velocity/2;
	microCont1.dx = Velocity * cos(Angle);
	microCont1.dy = Velocity * sin(Angle);
	ST7735_FillRect(0,0,60, 30, 0xFE21);
	ST7735_SetCursor(10, 0);
	if (langFlag == 1){
		ST7735_OutString("Shoot!");
	} else{
		ST7735_OutChar(0xAD);
		ST7735_OutString("Dispara!");
	}
	while (doneFlag == 0){}
	Turn = 1;
	doneFlag = 0;
}

//Player 2 function
void Player2Turn(void){
	DisplayScore();
	Display();
	//Delay100ms(10);
	Angle *= pi;
	Angle /= 180;
	Velocity = Velocity/2;
	microCont2.dx = Velocity * cos(Angle);
	microCont2.dx *= -1;
	microCont2.dy = Velocity * sin(Angle);
	ST7735_FillRect(0,0,60, 30, 0xFE21);
	ST7735_SetCursor(10, 0);
	if (langFlag == 1){
		ST7735_OutString("Shoot!");
	} else{
		ST7735_OutChar(0xAD);
		ST7735_OutString("Dispara!");
	}
	while (doneFlag == 0){}
	Turn = 0; 
	doneFlag = 0;
}




	


