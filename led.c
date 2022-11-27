



/*
 * led.c
 *
 * Created: 2022-08-09 오후 2:39:38
 *  Author: kccistc
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "led.h"


extern uint32_t nodelay;


int led_command = 0;  //  nothing
//  1: ledalltoggle
//  2: shift_left_led_on
//  3: shift_right_led_on

void init_led()
{
	AUTO_RUN_LED_PORT_DDR |= 1 << AUTO_RUN_LED_PIN;  //  led 출력	
}

int toggle = 0;

void ledalltoggle()
{
	toggle = !toggle;
	if (toggle)	
	PORTA = 0xff;
	else
	PORTA = 0x00;
	_delay_ms(300);
}





