#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include "../UART/BlockingUART.h"

void task() {
	for (;;) {
		UART_print("%d\n",Task_GetArg());
		//This ensures a_main stack is reused
		if (Task_GetArg() == 0) {
			Task_Create_RR(task, 200);
		}
		Task_Next();
	}
}

void a_main() {
	UART_print("\ntest begin\n");

	int i;
	for (i = 0; i < 20; i++) {
		Task_Create_RR(task, i);
	}
}
