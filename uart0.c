/*
 * uart.c
 *
 * Created: 2022-08-09 오전 10:47:37
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>  //  strncmp, strcpy 등등

#include "uart0.h"

ISR(USART0_RX_vect)
{
	unsigned char data;
	
	data = UDR0;			  //  uart0의 hardware register(UDR0)로 1byte를 읽어 들인다.
	
	if(data == '\r' || data == '\n')  //  문장의 끝이면 
	{
		rx_buffer[i] = '\0';    //  문장의 끝을 가리키는 NULL
		i = 0;				  //  개선점 : circular queue (환영큐)로 개선을 해야 한다.
							  //  이렇게 구성하면 새로운 메세지가 올 시 덮어 쓴다.
		rx_ready_flag = 1;
	}
	else
	{
		rx_buffer[i++] = data;  //  1. rx_buff[i] = data  2. i++
	}
}
//  1. 전송속도 : 9600bps : 총 byte(글자) 수 : 9600 / 10  ==> 960자 
//  2. 비동기식, data 8bit, non parity
//  3. RX{수신) inturrupt 활성화

void init_uart0(void)
{
	UBRR0H = 0x00;
	UBRR0L = 207;    //  9600bps  p219  표 9-9
	
	UCSR0A |= (1 << U2X0);
	UCSR0C |= 0x06;  //  ASYNC(비동기) data 8bit, non parity
	
				     //  RXEN0 : UART0로부터 수신이 가능하도록 설정
					 //  TXEN0 : UART0로부터 송신이 가능하도록 설정
					 //  RXCIE0 : UART0로부터 1byte가 들어오면 rx(수신) interrupt 를 띄워 달라
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
}

void UART0_transmit(uint8_t data)	   //  UART0로 1byte를 보내는 함수
{
	while (!(UCSR0A & (1 << UDRE0)))  //  data 가 전송중이면 data 가 모두 전송될 때 까지 기다린다.
		;							   //  no operation (아무 액션도 취하지 않는다)
	UDR0 = data;					   //  hardware 전송 register 에 data 를 보낸다.
}
//  1. command 를 함수화 하여 led.c에 배치한다.
//  -  ledallon
//  -  ledalloff
//  -  ledalltoggle  :  300ms 주기로 led 전체를 on/off 한다.

extern int led_command;

