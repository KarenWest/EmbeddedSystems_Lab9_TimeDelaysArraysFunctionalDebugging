// ***** 0. Documentation Section *****
// main.c for Lab 9
// Runs on LM4F120/TM4C123
// In this lab we are learning functional debugging by dumping
//   recorded I/O data into a buffer
// February 21, 2014

// Lab 9
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****



/* 
This Lab9 starter project is the same as C9_Debugging example but 
includes the connections to the Lab9 grader. You will make three changes. 
First, make the LED flash at 10 Hz. In other words, make it turn on for 0.05 seconds, 
and then turn off for 0.05 seconds. 
Second, make the LED flash if either switch SW1 or SW2 are pressed 
(this means either PF4 or PF0 is 0). 
Third, record PortF bits 4,1,0 every time the input changes or the output changes. 
For example, if your system detects a change in either PF4 or PF0 input, 
record PortF bits 4,1,0. If your system causes a change in PF1, record PortF bits 4,1,0. 

If both PF4 and PF0 switch are not pressed, the PF1 output should be low.  
If either PF4 or PF0 switches is pressed, the output toggles at 10 Hz (±10%). 
Information collected in the Data array matches the I/O on PortF.
50 data points are collected only on a change in input or a change in output.
(i.e., no adjacent elements in the array are equal).

*/


void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;        // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it             
  NVIC_ST_CTRL_R = 0x00000005;          // enable SysTick with core clock
}
unsigned long Led;
void Delay(void){unsigned long volatile time;
  time = 80000; // 0.05 seconds (LED on for .05sec, then off for .05 sec, flash it at 10MHz
  while(time){
   time--;
  }
}
// first data point is wrong, the other 49 will be correct
unsigned long Time[50];
// you must leave the Data array defined exactly as it is
unsigned long Data[50];
int main(void){  
	unsigned long i,lastSW1, lastSW2,nowSW1, lastLED, nowSW2, nowLED;
  
	TExaS_Init(SW_PIN_PF40, LED_PIN_PF1);  // activate grader and set system clock to 16 MHz
  PortF_Init();   // initialize PF1 to output
  SysTick_Init(); // initialize SysTick, runs at 16 MHz
  i = 0;          // array index
  //last = NVIC_ST_CURRENT_R;
	lastSW1 = lastSW2 = nowSW1 = nowSW2 = lastLED = nowLED = 0;
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
    Led = GPIO_PORTF_DATA_R;   // read previous
		nowSW1 = Led & 0x10;
		nowSW2 = Led & 0x01;
		nowLED = Led & 0x02;
		if ((lastSW1 != nowSW1) || (lastSW2 != nowSW1)) { //if switch 1 or 2's input value changed record data
				Data[i] = Led & 0x13; //only record bits 4,1,0 (PF4,PF1,PF0)
				i++;
				lastSW1 = nowSW1;
				lastSW2 = nowSW2;
		}
		if (!nowSW1 || !nowSW2) { //if switch 1 or 2's button is pressed
			Led = Led^0x02;            // toggle red LED
			GPIO_PORTF_DATA_R = Led;   // output 
			Led = GPIO_PORTF_DATA_R;   // record data since PF1 output just changed
			Data[i] = Led & 0x13; //only record bits 4,1,0 (PF4,PF1,PF0)
			lastLED = Led;
			i++;
			Delay();
		}
		else { //neither switch 1 or switch 2 are pressed so turn OFF LED PF1 output
			GPIO_PORTF_DATA_R &= ~0x01; //output PF1 red LED = 0 when neither button pressed
			if (nowLED) { //red LED was ON at start of this loop but is now off so record data, output PF1 changed
				Data[i] = GPIO_PORTF_DATA_R & 0x13; //only record bits 4,1,0 (PF4,PF1,PF0)
				lastLED = 0;
				i++;
			}
  }
}
/*    if(i<50){
      now = NVIC_ST_CURRENT_R;
      Time[i] = (last-now)&0x00FFFFFF;  // 24-bit time difference
      Data[i] = GPIO_PORTF_DATA_R&0x02; // record PF1
      last = now;
      i++;
    }
*/


// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
