#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>

void Pong() {
	for(;;) {
		toggle_LED_C7();
		_delay_ms(100);
		Task_Next();
	}
}

void Ping() {
	for (;;) {
		toggle_LED_B7();
		Task_Next();
	}
}

void a_main(void) {
	init_LED_C7();
	init_LED_B7();

	Task_Create_Period(Pong, 0, 100, 30, 300);
	Task_Create_RR(Ping, 0);
}
