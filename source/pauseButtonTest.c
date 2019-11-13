/*	Author: nmoor004
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab # 11  Exercise # 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

////////////////////////////////////////////
///////////TIMER///////////////////////////
//////////////////////////////////////////

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= 0x0B;	// bit3 = 1: CTC mode (clear timer on compare)
					// bit2bit1bit0=011: prescaler /64
					// 00001011: 0x0B
					// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
					// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
					// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
					// So when TCNT1 register equals 125,
					// 1 ms has passed. Thus, we compare to 125.
					// AVR timer interrupt mask register

	TIMSK1 	= 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void TimerOff() {
	TCCR1B 	= 0x00; // bit3bit2bit1bit0=0000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; 			// Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}













////////////////////////////////////////////////
//Global Variables/////////////////////////////
//////////////////////////////////////////////

unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0;


/////////////////////////////////////////////////
///////////////pauseButtonSM/////////////////
/////////////////////////////////////////////

enum pauseButtonSM_States { pauseButton_wait, pauseButton_press, pauseButton_release} ;

int pauseButtonSMTick(int state) {
	unsigned char press = ~PINA & 0x01;

	switch(state) {
		case pauseButton_wait: 
			state = press == 0x01 ? pauseButton_press: pauseButton_wait;
			break;

		case pauseButton_press:
			state = pauseButton_release; 
			break;

		case pauseButton_release:
			state = press = 0x00 ? pauseButton_wait: pauseButton_press;
			break;

		default: 
			state = pauseButton_wait;
			break;
	}
	
	switch(state) {
		case pauseButton_wait: 
			break;
		case pauseButton_press:
			pause = (pause == 0) ? 1 : 0;
			break;
		case pauseButton_release: 
			break;
	}

	return state;

}


/////////////////////////////////////////////////////////////////////////////
//////toggleLEDs///////////////////////////////
///////////////////////////////////////////////////////////////

enum toggleLED0_States { toggleLED0_wait, toggleLED0_blink};

int toggleLED0SMTick(int state) {
	switch(state) {
		case toggleLED0_wait: state = !pause? toggleLED0_blink: toggleLED0_wait; break;
		case toggleLED0_blink: state = pause? toggleLED0_wait: toggleLED0_blink; break;
		default: state = toggleLED0_wait; break;
	}
	switch(state) {
		case toggleLED0_wait: break;
		case toggleLED0_blink:
			led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
			break;
	}
	return state;
}

enum toggleLED1_States { toggleLED1_wait, toggleLED1_blink};

int toggleLED1SMTick(int state) {
	switch(state) {
		case toggleLED1_wait: state = !pause? toggleLED1_blink: toggleLED1_wait; break;
		case toggleLED1_blink: state = pause? toggleLED1_wait: toggleLED1_blink; break;
		default: state = toggleLED0_wait; break;
	}
	switch(state) {
		case toggleLED1_wait: break;
		case toggleLED1_blink:
			led1_output = (led1_output == 0x00) ? 0x01 : 0x00;
			break;
	}
	return state;
}

/////////////////////////////////////////////////////////////
////////////////display states//////////////////////////////
////////////////////////////////////////////////////////

enum display_States {display_display} ;

int displaySMTick(int state) {

	unsigned char output;
	switch (state) {
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}
	switch(state) {
		case display_display:
			output = led0_output | led1_output << 1;
			break;
	}
	PORTB = output;
	return state;

}


//////////////////////////
//////////GCD///////////
///////////////////////////

unsigned long int findGCD(unsigned long int a, unsigned long int b) {

	unsigned long int c;
	while (1) {
		c = a%b;
		if (c == 0) {
			return b;
		}
		a = b;
		b = c;
	}

	return 0;
	
}



////////////////////////////////////////
////////////////Task Scheduler Struct//
//////////////////////////////////////

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task; 


//////////////////////////////////////////////////////////////////////////
/////////////////////MAIN/////////////////////////////////////
//////////////////////////////////////////////

int main() {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	

	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &pauseButtonToggleSMTick;	

	task2.state = start;
	task2.period = 500;
	task2.elapsedTime = task2.period;
	task2.TickFct = &toggleLED0SMTick;

	task3.state = start;
	task3.period = 1000;
	task3.elapsedTime = task3.period;
	task3.TickFct = &toggleLED1SMTick;
	
	task4.state = start; 
	task4.period = 10;
	task4.elapsedTime = task4.period;
	task4.TickFct = &SMTick4;
	
	unsigned long GCD = tasks[0]->period;
	for (i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}


	TimerSet(GCD);
	TimerOn();
	unsigned short i;

    /* Insert your solution below */
	while (1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}

		while(!TimerFlag);
		TimerFlag = 0;
	}
			
    }
    return 0;
}


