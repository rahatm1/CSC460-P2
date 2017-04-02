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

void servo(){
	servo_init();
	int i;
	for(;;)
		for ( i=350; i<550; i+=10) {
			servo_set(i);
			_delay_ms(60);
		}

		for (; i>350; i-=10) {
			servo_set(i);
			_delay_ms(60);
		}
}

void a_main(void) {
	Task_Create_System(servo, 0);
}
