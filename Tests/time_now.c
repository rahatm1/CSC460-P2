#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include "../UART/BlockingUART.h"

void Ping() {
	for (;;) {
		UART_print("%u\n", Now());
		_delay_ms(1000);
		Task_Next();
	}
}

void a_main(void) {
	Task_Create_System(Ping, 0);
}
