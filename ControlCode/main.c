#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"

#define DEBUG 1
#define bit_get(p,m) ((p) & (m))
#define BIT(x) (0x01 << (x))


int JoyStick_X2 = 0; // x drive
int JoyStick_Y2 = 1; // y drive
int JoyStick_X = 2; // x
int JoyStick_Y = 3; // y
BOOL stopped = TRUE;

void digital_init_C(int port) {
	DDRC &= ~(BIT(port));
}
void drive(){
	while(1) {

		int x = read_analog(JoyStick_X2);
		int y = read_analog(JoyStick_Y2);
		BOOL z = bit_get(PINC, 6); //(add code)to be filled in later

#ifdef DEBUG
		UART_print("%u\n",x);
		UART_print("%u\n", y);
		UART_print("%u\n", z);
		UART_print("\n");
#endif
		//UART_Transmit1('a'); //use to send charcter -> use 1 to send over BT
		  if (!z) { //fire up the meat grinder boys, things are about to get hairy
			if (y <= 127) {
			  if (x <= 383) {
			   UART_Transmit1('q');
				stopped = FALSE;
			  } else if (x <= 639) {
				UART_Transmit1('o');
				stopped = FALSE;
			  } else {
				UART_Transmit1('l');
				stopped = FALSE;
			  }


			} else if (y <= 383) {
			  if (x <= 127) {
				UART_Transmit1('q');
				stopped = FALSE;
			  } else if (x <= 383) {
				UART_Transmit1('p');;
				stopped = FALSE;
			  } else if (x <= 639) {
				UART_Transmit1('n');
				stopped = FALSE;
			  } else if (x <= 895) {
				UART_Transmit1('m');
				stopped = FALSE;
			  } else {
				UART_Transmit1('l');
				stopped = FALSE;
			  }


			} else if (y <= 639) {
			  if (x <= 127) {
				UART_Transmit1('k');
				stopped = FALSE;
			  } else if (x <= 383) {
				UART_Transmit1('j');;
				stopped = FALSE;
			  } else if (x <= 639) {
				if (stopped) {
				  //do nothing
				  UART_Transmit1('i');
				} else {
				  UART_Transmit1('i');
				  stopped = TRUE;
				}
			  } else if (x <= 895) {
				UART_Transmit1('h');
				stopped = FALSE;
			  } else {
				UART_Transmit1('g');
				stopped = FALSE;
			  }


			} else if (y <= 895) {
			  if (x <= 127) {
				UART_Transmit1('f');
				stopped = FALSE;
			  } else if (x <= 383) {
				UART_Transmit1('e');
				stopped = FALSE;
			  } else if (x <= 639) {
				UART_Transmit1('d');
				stopped = FALSE;
			  } else if (x <= 895) {
				UART_Transmit1('b');
				stopped = FALSE;
			  } else {
				UART_Transmit1('a');
				stopped = FALSE;
			  }


			} else {
			  if (x <= 383) {
				UART_Transmit1('f');
				stopped = FALSE;
			  } else if (x <= 639) {
				UART_Transmit1('c');
				stopped = FALSE;
			  } else {
				UART_Transmit1('a');
				stopped = FALSE;
			  }
			}
	  } else { //use me to do something later
		UART_Transmit1('i');
#ifdef DEBUG
		UART_print("STOPPING");
#endif
		stopped = TRUE;
	  }
		Task_Next();
	}
}

void aim() {
	while(1){

	  int x2 = read_analog(JoyStick_X);
	  int y2 = read_analog(JoyStick_Y);
	  BOOL z2 = bit_get(PINC, 7); //(add code)to be filled in later

	  if (!z2) {

		if (x2 <= 383) {
		  UART_Transmit1('R');
		} else if (x2 >= 639) {
		  UART_Transmit1('L');
		}
		if (y2 <= 383) {
		  UART_Transmit1('D');
		} else if (y2 >= 639) {
		  UART_Transmit1('U');
		}else{
		  UART_Transmit1('S');
		}
	  } else {
		UART_Transmit1('Z');
	  }

		Task_Next();
	}
}

void a_main(void) {
	setup_controllers();
	digital_init_C(6);
	digital_init_C(7);
	UART_Init1(9600);
	Task_Create_Period(drive,0,5,3,0);
	Task_Create_Period(aim,0,5,3,4);
}
