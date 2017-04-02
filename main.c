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

void read_bt() {
  for(;;){
  char command = UART_Receive1_Non_Blocking();
  UART_Transmit0(command);
  if (command != -1) {
	UART_Transmit0(command);
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
    /* case 'U': */
    /*   tiltUp = true; */
    /*   tiltDown = false; */
    /*   break; */
    /* case 'D': */
    /*   tiltUp = false; */
    /*   tiltDown = true; */
    /*   break; */
    /* case 'L': */
    /*   panLeft = true; */
    /*   panRight = false; */
    /*   break; */
    /* case 'R': */
    /*   panLeft = false; */
    /*   panRight = true; */
    /*   break; */
    /* case 'S': */
    /*   tiltUp = false; */
    /*   tiltDown = false; */
    /*   panLeft = false; */
    /*   panRight = false; */
    /*   break; */
    /* case 'Z': */
    /*   laserOn = !laserOn; */
    /*   break; */
    }
  }
  Task_Next();
  }
}

void a_main(void) {
	UART_Init1(9600);
	Roomba_Init();
	Task_Create_Period(read_bt, 0, 7, 6, 0);
}
