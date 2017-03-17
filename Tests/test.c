#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>

CHAN c1;

void Pong() {
	int val = Recv(c1);
	if (val==5) {
		for(;;) {
			toggle_LED_C7();
			_delay_ms(100);
			Task_Next();
		}
	}
}

void Ping() {
	_delay_ms(4000);
	Write(c1, 5);
	for (;;) {
		toggle_LED_B7();
		_delay_ms(100);
		Task_Next();
	}
}

void a_main(void) {
	init_LED_C7();
	init_LED_B7();

	c1 = Chan_Init();
	Task_Create_System(Pong, 0);
	Task_Create_System(Ping, 0);
}
