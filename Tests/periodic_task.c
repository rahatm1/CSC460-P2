#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include "../UART/BlockingUART.h"

void task() {
	for(;;) {
		UART_print("%d\n",Task_GetArg());
		Task_Next();
	}
}

void a_main(void) {

	Task_Create_Period(task, 0, 8, 3, 0);
	Task_Create_Period(task, 1, 8, 3, 5);
	Task_Create_Period(task, 2, 8, 3, 10);
	Task_Create_Period(task, 3, 8, 3, 15);
	Task_Create_Period(task, 4, 8, 3, 20);
	Task_Create_Period(task, 5, 8, 3, 25);
	Task_Create_Period(task, 6, 8, 3, 30);
}
