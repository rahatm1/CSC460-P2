#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <util/delay.h>
#include "../UART/BlockingUART.h"

void task(){
	for (;;) {
		UART_print("Hello from RR \n");
		Task_Next();
	}
}
void task2(){
	for (;;) {
		UART_print("Hello from System\n");
		Task_Next();
	}
}

void task3(){
	for (;;) {
		UART_print("Hello from period\n");
		Task_Next();
	}
}

void a_main(){
	UART_print("\ntest begin\n");
	
	Task_Create_RR(task, 22);
	UART_print("\1\n");
	Task_Create_System(task2, 99);
	UART_print("\2\n");
	Task_Create_Period(task3, 0, 8, 3, 0);
	UART_print("\3\n");
}

