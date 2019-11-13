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
#include "timer.h"
#include "keypad.h"
#include "bit.h"
#include "scheduler.h"


task tasks;
const unsigned char tasksNum = 1;



enum SM_Updater {start, wait, display} updater;

int updateState(int state) {

	unsigned char key = GetKeypadKey();
	switch(state) {
		case start:
			state = wait;

			break;
		case wait:
			if (key == '\0') {
				state = wait;
			}
			else {
				state = display;
			}
			PORTB = 0x1F;
			break;
		case display:
			if (key == '\0') {
				state = wait;
			}
			else {
				state = display;
			}
			switch (key) {
				case '1': PORTB = 0x01; break;
				case '2': PORTB = 0x02; break;
				case '3': PORTB = 0x03; break;
				case '4': PORTB = 0x04; break;
				case '5': PORTB = 0x05; break;
				case '6': PORTB = 0x06; break;
				case '7': PORTB = 0x07; break;
				case '8': PORTB = 0x08; break;
				case '9': PORTB = 0x09; break;
				case 'A': PORTB = 0x0A; break;
				case 'B': PORTB = 0x0B; break;
				case 'C': PORTB = 0x0C; break;
				case 'D': PORTB = 0x0D; break;
				case '*': PORTB = 0x0E; break;
				case '0': PORTB = 0x00; break;
				case '#': PORTB = 0x0F; break;
			}

			break;
		default: 
			state = start;
			break;
	}

	switch (state) {
		case start:
			break;
		case wait:
			break;
		case display:
			break;
	}

}
			

//////////////////////////////////////////////////////////////////////////
/////////////////////MAIN/////////////////////////////////////
//////////////////////////////////////////////

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;

	//unsigned char i = 0;
	tasks.state = start;
	tasks.TickFct = &updateState;

    /* Insert your solution below */
	while (1) {
		tasks.state = tasks.TickFct(tasks.state);
	}
			
    }
    return 0;
}
