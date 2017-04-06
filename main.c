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
int fast = 400;
int slow = 100;
CHAN c1;

void read_bt() {
  for(;;){
  char command = UART_Receive1_Non_Blocking();
  /* UART_Transmit0(command); */
  if (command != -1) {
    switch (command)
    {
    //Roomba
    case 'a': //fast forward left
      Roomba_Drive(fast,250);
      break;
    case 'b': //slow forward left
      Roomba_Drive(slow, 250);
      break;
    case 'c': //fast forward
      Roomba_Drive(fast, 32768);
      break;
    case 'd': //slow forward
      Roomba_Drive(slow, 32768);
      break;
    case 'e': //slow forward right
      Roomba_Drive(slow,-250);
      break;
    case 'f': //fast forward right
      Roomba_Drive(fast,-250);
      break;
    case 'g': //fast rotate left on spot
      Roomba_Drive(fast,1);
      break;
    case 'h': //slow rotate left on spot
      Roomba_Drive(slow,1);
      break;
    case 'i': //stop
      Roomba_Drive(0,0);
      break;
    case 'j': //slow rotate right on spot
      Roomba_Drive(slow, -1);
      break;
    case 'k': //fast rotate right on spot
      Roomba_Drive(fast, -1);
      break;
    case 'l': //fast reverse left
      Roomba_Drive(-1*fast, 250);
      break;
    case 'm': //slow reverse left
      Roomba_Drive(-1*slow, 250);
      break;
    case 'n': //slow reverse
      Roomba_Drive(-1*slow, 32768);
      break;
    case 'o': //fast reverse
      Roomba_Drive(-1*fast, 32768);
      break;
    case 'p': //slow reverse right
      Roomba_Drive(-1*slow,-250);
      break;
    case 'q': //fast reverse right
      Roomba_Drive(-1*fast,-250);
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

void reverse() {
	while(1){
		if (Recv(c1) == 1) {
			Roomba_Drive(-1*slow, 32768);
			_delay_ms(1500);
			Roomba_Drive(0,0);
		}
		Task_Next();
	}
}

void sense() {
	for(;;) {
		if(Roomba_LightBumperDetection() != 0) {
			Send(c1, 1);
		}
		Task_Next();
	}
}

void a_main(void) {
	UART_Init1(9600);
	Roomba_Init();
	servo_init();
	init_LED_C7();
	Task_Create_Period(read_bt, 0, 5, 4, 0);
	Task_Create_Period(sense, 0, 5, 4, 3);
	Task_Create_RR(servo1, 0);
	Task_Create_RR(servo2, 0);
	c1 = Chan_Init();
	Task_Create_System(reverse, 0);
}
