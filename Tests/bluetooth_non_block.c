#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"
#include "Roomba/roomba.h"

void funa() {
	while(1){
		if (UCSR2A & _BV(RXC2)) {
			UART_Transmit0(UDR2);
		}
		Task_Next();
	}
}

void a_main(void) {
	UART_Init1(9600);
	Task_Create_Period(funa, 0, 3, 2, 0);
}

