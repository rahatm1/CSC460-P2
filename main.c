#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "joystick.h"

void setup() {
	setup_controllers();
	while(1) {
		UART_print("%u\n",read_analog(1) );
		UART_print("%u\n",read_analog(2) );
	}
}

void a_main(void) {
	Task_Create_System(setup, 0);
}
