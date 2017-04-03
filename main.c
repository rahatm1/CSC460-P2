#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"
#include  "Roomba/roomba.h"
#include "servo/servo.h"

char action = 'Q'; //unused;
BOOL laserOn = FALSE;

void read_bt() {
  for(;;){
  char command = UART_Receive1_Non_Blocking();
  /* UART_Transmit0(command); */
  UART_print("%c\n", command);
  if (command != -1) {
    switch (command)
    {
    //Roomba
    case 'a': //fast forward left
      Roomba_Drive(400,250);
      break;
    case 'b': //slow forward left
      Roomba_Drive(100, 250);
      break;
    case 'c': //fast forward
      Roomba_Drive(400, 32768);
      break;
    case 'd': //slow forward
      Roomba_Drive(100, 32768);
      break;
    case 'e': //slow forward right
      Roomba_Drive(100,-250);
      break;
    case 'f': //fast forward right
      Roomba_Drive(400,-250);
      break;
    case 'g': //fast rotate left on spot
      Roomba_Drive(400,1);
      break;
    case 'h': //slow rotate left on spot
      Roomba_Drive(100,1);
      break;
    case 'i': //stop
      Roomba_Drive(0,0);
      break;
    case 'j': //slow rotate right on spot
      Roomba_Drive(100, -1);
      break;
    case 'k': //fast rotate right on spot
      Roomba_Drive(400, -1);
      break;
    case 'l': //fast reverse left
      Roomba_Drive(-400, 250);
      break;
    case 'm': //slow reverse left
      Roomba_Drive(-100, 250);
      break;
    case 'n': //slow reverse
      Roomba_Drive(-100, 32768);
      break;
    case 'o': //fast reverse
      Roomba_Drive(-400, 32768);
      break;
    case 'p': //slow reverse right
      Roomba_Drive(-100,-250);
      break;
    case 'q': //fast reverse right
      Roomba_Drive(-400,-250);
      break;

    //Laser //TODO
    case 'Z':
	  toggle_LED_C7();
      break;
	default:
	  action = command;
    }
  }
  Task_Next();
  }
}

void servo1(){
	int i=475;
	while(1){
		if(action=='D'){
			if((i+15)<550){
				i+=15;
				servo_set_pin2(i);
			}
			action='Q'; //non used charcter
		}else if(action=='U'){
			if((i-15)>250){
				i-=15;
				servo_set_pin2(i);
			}
			action='Q';
		}
		Task_Next();
	}
}

void servo2(){
	int i=475;
	while(1){
		if(action=='L'){
			if((i+15)<550){
				i+=15;
				servo_set_pin3(i);
			}
		}else if(action=='R'){
			if((i-15)>250){
				i-=15;
				servo_set_pin3(i);
			}
		}
		action='Q';
		Task_Next();
	}
}

void a_main(void) {
	UART_Init1(9600);
	Roomba_Init();
	servo_init();
	init_LED_C7();
	Task_Create_Period(read_bt, 0, 3, 2, 0);
	Task_Create_Period(servo1, 0, 3, 2, 1);
	Task_Create_Period(servo2, 0, 3, 2, 2);
}
