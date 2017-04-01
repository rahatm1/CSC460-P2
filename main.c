#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"

int JoyStick_X2 = 0; // x drive
int JoyStick_Y2 = 1; // y drive
int JoyStick_Z2 = 34; // key
int JoyStick_X = 2; // x 
int JoyStick_Y = 3; // y 
int JoyStick_Z = 35; // key
boolean stopped = true;
//int photocellPin = A8;
int photocellReading;

void setup() {
	while(1) {
		UART_print("%u\n",read_analog(JoyStick_X2) );
		UART_print("%u\n",read_analog(JoyStick_Y2) ); 
		UART_print("\n");
		_delay_ms(1000);
	}
}
void drive(){
	while(1) {
		int x = read_analog(JoyStick_X2) 
		int y = read_analog(JoyStick_Y2)
		int z = 1; //(add code)to be filled in later
		UART_print("%u\n",x);
		UART_print("%u\n", y); 
		UART_print("\n");
		//UART_Transmit0('a'); //use to send charcter -> use 1 to send over BT
		  if (z == 1) { //fire up the meat grinder boys, things are about to get hairy
    if (y <= 127) {
      if (x <= 383) {
       UART_Transmit0('q');
        stopped = false;
      } else if (x <= 639) {
        UART_Transmit0('o');
        stopped = false;
      } else {
        UART_Transmit0('l');
        stopped = false;
      }


    } else if (y <= 383) {
      if (x <= 127) {
        UART_Transmit0('q');
        stopped = false;
      } else if (x <= 383) {
        UART_Transmit0('p');;
        stopped = false;
      } else if (x <= 639) {
        UART_Transmit0('n');
        stopped = false;
      } else if (x <= 895) {
        UART_Transmit0('m');
        stopped = false;
      } else {
        UART_Transmit0('l');
        stopped = false;
      }


    } else if (y <= 639) {
      if (x <= 127) {
        UART_Transmit0('k');
        stopped = false;
      } else if (x <= 383) {
        UART_Transmit0('j');;
        stopped = false;
      } else if (x <= 639) {
        if (stopped) {
          //do nothing
        } else {
          UART_Transmit0('i');
          stopped = true;
        }
      } else if (x <= 895) {
        UART_Transmit0('h');
        stopped = false;
      } else {
        UART_Transmit0('g');
        stopped = false;
      }


    } else if (y <= 895) {
      if (x <= 127) {
        UART_Transmit0('f');
        stopped = false;
      } else if (x <= 383) {
        UART_Transmit0('e');
        stopped = false;
      } else if (x <= 639) {
        UART_Transmit0('d');
        stopped = false;
      } else if (x <= 895) {
        UART_Transmit0('b');
        stopped = false;
      } else {
        UART_Transmit0('a');
        stopped = false;
      }


    } else {
      if (x <= 383) {
        UART_Transmit0('f');
        stopped = false;
      } else if (x <= 639) {
        UART_Transmit0('c');
        stopped = false;
      } else {
        UART_Transmit0('a');
        stopped = false;
      }
    }
  } else { //use me to do something later
    UART_Transmit0('i');
    UART_print("STOPPING"); 
    stopped = true;
  }
		Task_Next();
	}
}

void a_main(void) {
	//Task_Create_System(setup, 0);
	setup_controllers();
	UART_Init1(9600);
	Task_Create_Period(drive,0,15,5,10);
}

