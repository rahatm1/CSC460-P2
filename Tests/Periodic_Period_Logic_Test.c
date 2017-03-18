#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include "../LED/LED_Test.h"
#include <util/delay.h>
#include "../UART/BlockingUART.h"

void task(){
	for (;;) {
		UART_print("RR \n");
		Task_Next();
	}
}

void task3(){
	for (;;) {
		toggle_LED_B7();
		Task_Next();
	}
}
void task5(){
	for (;;) {
		toggle_LED_C7();
		Task_Next();
	}
}

void a_main(){
	init_LED_C7();
	init_LED_B7();
	
	Task_Create_RR(task, 22);
	Task_Create_Period(task3, 0, 5, 3, 0);
	Task_Create_Period(task5, 3, 5, 3, 8);
}

