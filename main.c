#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"
#include "servo/servo.h"

int panPos = 208;
int tiltPos = 208;
int laserOn = 0; //0 is Off, 1 is On
int laserPin = 30;

void tiltUp() {
	int foo=0; while(foo>0){
		servo_pen_up();
		foo--;
	}
}

void panLeft() {
	tiltPos-=2;
	servo_set(tiltPos);
}

void tiltDown() {
  int foo=0; while(foo>0){
		servo_pen_down();
		foo--;
	}
}

void panRight() {
    tiltPos+=2;
	servo_set(tiltPos);
}

void switchLaser() {
	if(laserOn==1){
		laserOn=0;
		toggle_LED_C7();
	}else{
		laserOn=1;
		toggle_LED_C7();
	}
}

void servo(){
	servo_init();
	while(1){
		int command = 'U';
		switch(command){
			case 'U': //tilt up
			tiltUp();
			break;
			case 'D': //tilt down
			tiltDown();
			break;
			case 'L': //pan left
			panLeft();
			break;
			case 'R': //pan right
			panRight();
			break;
			case 'Z':
			switchLaser();
			break;
		}
		Task_Next();
	}
}

void a_main(void) {
	UART_Init1(9600);
	Task_Create_Period(servo, 0,25,10,6);
}

