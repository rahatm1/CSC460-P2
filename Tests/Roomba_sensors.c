#include <stdint.h>
#include "os.h"
#include "common.h"
#include "LED/LED_Test.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include "UART/BlockingUART.h"
#include "sensor/joystick.h"
#include  "Roomba/roomba.h"
#include "servo/servo.h"

void a_main(void) {
	Roomba_Init();
	roomba_sensor_data_t packet = {0};
	roomba_sensor_data_t packet2 = {0};
	while(1) {
		UART_print("%d", Roomba_LightBumperDetection());
		Roomba_UpdateSensorPacket(EXTERNAL, &packet);
		_delay_ms(30);
		UART_print("bumps_wheeldrops: %d\n virtual wall: %d\n",
					packet.bumps_wheeldrops,
					packet.virtual_wall
				);

		_delay_ms(500);
		Roomba_UpdateSensorPacket(LIGHT_BUMPERS, &packet2);
		_delay_ms(30);
		UART_print("%u %u %u %u %u %u\n",
				 packet2.light_left,
				 packet2.light_front_left,
				 packet2.light_center_left,
				 packet2.light_center_right,
				 packet2.light_front_right,
				 packet2.light_right);
		_delay_ms(500);

	}
}
