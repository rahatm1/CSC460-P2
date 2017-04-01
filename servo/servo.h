/**
 * \file servo.h
 * \brief Simple control of a servo using hardware PWM.
 *
 * Created: 19/08/2013 9:16:26 AM
 *  \Author: Daniel McIlvaney
 */ 


#ifndef SERVO_H_
#define SERVO_H_
// MG995 middle:1350 us, +/- 870 ms


void servo_init(){
	//Using Timer 1
	TIMSK4 &= ~(1<<OCIE4A);
	
	//Set to Fast PWM mode 15
	TCCR4A |= (1<<WGM40) | (1<<WGM41);
	TCCR4B |= (1<<WGM42) | (1<<WGM43);
	
	//TCCR3A |= (1<<COM3B1);
	TCCR4A |= (1<<COM4C1);
	TCCR4B |= (1<<CS41)|(1<<CS40);
	
	OCR4A=5000;  //20 ms period

	DDRE |= (1<<PE5);  //PWM Pins as Out	
}

void servo_set(uint16_t pos) {
	if(pos < 135) {
		pos = 135;
	}
	if(pos > 550) {
		pos = 550;
	}
	OCR4C = pos;
}


void servo_pen_down(){
	servo_set(550L);
}

void servo_pen_up(){
	servo_set(340L);
}

#endif /* SERVO_H_ */