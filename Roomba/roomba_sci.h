/*
 * roomba_sci.h
 *
 *  Created on: 17-May-2009
 *      Author: nrqm
 */

#ifndef ROOMBA_SCI_H_
#define ROOMBA_SCI_H_

#define HIGH_BYTE(x) (x>>8)
#define LOW_BYTE(x)  (x&0xFF)

/*****											Commands										*****/

#define RESET	7
#define START	128		// start the Roomba's serial command interface
#define BAUD	129		// set the SCI's baudrate (default on full power cycle is 57600
//#define CONTROL	130		// enable control via SCI
#define SAFE	131		// enter safe mode
#define FULL	132		// enter full mode
#define POWER	133		// put the Roomba to sleep
//#define SPOT	134		// start spot cleaning cycle
//#define CLEAN	135		// start normal cleaning cycle
//#define MAX		136		// start maximum time cleaning cycle
#define DRIVE	137		// control wheels
#define LEDS	139		// activate LEDs
#define SONG	140		// load a song into memory
#define PLAY	141		// play a song that was loaded using SONG
#define SENSORS	142		// retrieve one of the sensor packets
#define DOCK	143		// force the Roomba to seek its dock.

#define STOP	173

/*****											Arguments										*****/

/// Arguments to the BAUD command
typedef enum _br {
	ROOMBA_300BPS = 0,
	ROOMBA_600BPS = 1,
	ROOMBA_1200BPS = 2,
	ROOMBA_2400BPS = 3,
	ROOMBA_4800BPS = 4,
	ROOMBA_9600BPS = 5,
	ROOMBA_14400BPS = 6,
	ROOMBA_19200BPS = 7,
	ROOMBA_28800BPS = 8,
	ROOMBA_38400BPS = 9,
	ROOMBA_57600BPS = 10,
	ROOMBA_115200BPS = 11,
} ROOMBA_BITRATE;

/// Bits in the MOTORS command argument
#define MAIN_BRUSH	2
#define VACUUM		1
#define SIDE_BRUSH	0

/// Bits in the first byte to the LEDS command
#define STATUS_GREEN	5	// The status LED is 2 bits:
#define	STATUS_RED		4	// 00 for off, 01 for red, 10 for green, 11 for amber (red + green)
#define SPOT_LED		3
#define CLEAN_LED		2
#define MAX_LED			1
#define DIRT_DETECT		0

/*****											Sensor Bits										*****/

/// Bits in the Bumps/Wheeldrops byte
#define WHEELDROP_CASTER	4
#define WHEELDROP_LEFT		3
#define WHEELDROP_RIGHT		2
#define BUMP_LEFT			1
#define BUMP_RIGHT			0

/// Bits in the motor overcurrents byte
#define DRIVE_LEFT		4
#define DRIVE_RIGHT		3
#define MAIN_BRUSH_OC	2
#define VACUUM_OC		1
#define SIDE_BRUSH_OC	0

/// Bits in the buttons byte
#define POWER_BTN 		3
#define SPOT_BTN		2
#define CLEAN_BTN		1
#define MAX_BTN			0

/// Number to determine the current OI mode of the roomba
#define ROOMBA_MODE_OFF 	0
#define ROOMBA_MODE_PASSIVE 1
#define ROOMBA_MODE_SAFE	2
#define ROOMBA_MODE_FULL	3

/// bit mask for querying the LightBumperDetection field
#define LIGHT_BUMPER_RIGHT 			5
#define LIGHT_BUMPER_FRONT_RIGHT	4
#define LIGHT_BUMPER_CENTER_RIGHT	3
#define LIGHT_BUMPER_CENTER_LEFT	2
#define LIGHT_BUMPER_FRONT_LEFT		1
#define LIGHT_BUMPER_LEFT 			0

/// Values of the charging state byte
typedef enum _cs
{
	NOT_CHARGING = 0,
	CHARGING_RECOVERY = 1,
	CHARGING = 2,
	TRICKLE_CHARGING = 3,
	WAITING = 4,
	CHARGING_ERROR = 5,
} ROOMBA_CHARGING_STATE;


//DD/Baud pin on digital 20.
#define ROOMBA_DD_PORT	PORTD
#define ROOMBA_DD_PIN	PD1
#define ROOMBA_DD_DDR	DDRD


#endif /* ROOMBA_SCI_H_ */
