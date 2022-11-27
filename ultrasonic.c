/*
 * ultrasonic.c
 *
 * Created: 2022-08-10 오후 4:21:51
 *  Author: kccistc
 */ 

#include "ultrasonic.h"


// 왼쪽 초음파 PE4 : 외부 INT4 초음파 센서 상승, 하강 edge 둘다 이쪽으로 들어온다.
// 초음파 : 1cm를 이동하는데 20us 소요 1cm 왕복시간 : 29 * 2 = 58us 소요
// 16000000Hz / 1024 = 15625 Hz
// T = 1/f ==> 0.0000064 sec = 6.4us
ISR(INT4_vect)
{
	if (ECHO_PIN_LEFT & (1 << ECHO_LEFT))  //  상승 edge
	{
		TCNT3 = 0;
	}
	else                         //  하강 edge, 에코핀의 펄스 길이를 us 단위로 환산
	{
		left_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;	 //  1cm : 58us소요			 
	}
}

//외부 INT5 : 중앙 초음파 센서 INT
ISR(INT5_vect)
{
	if (ECHO_PIN_CENTER & (1 << ECHO_CENTER))  //  상승 edge
	{
		TCNT3 = 0;
	}
	else                         //  하강 edge, 에코핀의 펄스 길이를 us 단위로 환산
	{
		center_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;	 //  1cm : 58us소요
	}
}

//외부 INT6 : 우측 초음파 센서 INT
ISR(INT6_vect)
{
	if (ECHO_PIN_RIGHT & (1 << ECHO_RIGHT))  //  상승 edge
	{
		TCNT3 = 0;
	}
	else                         //  하강 edge, 에코핀의 펄스 길이를 us 단위로 환산
	{
		right_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;	 //  1cm : 58us소요
	}
}

//초음파 센서 초기화 함수
void init_ultrasonic()
{
	//=============== LEFT ultrasonic =================//
	TRIG_DDR_LEFT |= (1 << TRIG_LEFT);   // output mode로 설정
	ECHO_DDR_LEFT &= ~(1 << ECHO_LEFT);  // input mode로 설정
	
	EICRB |= (0 << ISC41) | (1 << ISC40);  //  0 1 : 어떠한 형태로든지 신호 변화가 발생되면 interrupt 요청 (상승, 하강 edge 모두다 interrupt) 
	TCCR3B |= (1 << CS32) | (1 << CS30);   //  1024 분주
	// EICRA : INT 0~3
	// EICRB : INT 4~7
	// 0   1 : 어떠한 신호의 변화가 있는지 INT를 요청한다. (rising, falling edge 둘다 int 발생)
	// 16비트 timer 1번을 1024분주해서 공급
	// 16MHz를 1024 분주  ==>  16000000/1024 ==> 15.625kHz
	// 1펄스의 길이 : t = 1/f = 64us
	
	EIMSK |= (1 << INT4);   //  외부 INT4(ECHO) 사용
	
	//=============== CENTER ultrasonic ===============//
	TRIG_DDR_CENTER |= (1 << TRIG_CENTER);   // output mode로 설정
	ECHO_DDR_CENTER &= ~(1 << ECHO_CENTER);  // input mode로 설정
	
	EICRB |= (0 << ISC51) | (1 << ISC50);  //  0 1 : 어떠한 형태로든지 신호 변화가 발생되면 interrupt 요청 (상승, 하강 edge 모두다 interrupt)
	TCCR3B |= (1 << CS32) | (1 << CS30);   //  1024 분주
	// EICRA : INT 0~3
	// EICRB : INT 4~7
	// 0   1 : 어떠한 신호의 변화가 있는지 INT를 요청한다. (rising, falling edge 둘다 int 발생)
	// 16비트 timer 1번을 1024분주해서 공급
	// 16MHz를 1024 분주  ==>  16000000/1024 ==> 15.625kHz
	// 1펄스의 길이 : t = 1/f = 64us
	
	EIMSK |= (1 << INT5);   //  외부 INT4(ECHO) 사용
	
	//=============== RIGHT ultrasonic ===============//
	TRIG_DDR_RIGHT |= (1 << TRIG_RIGHT);   // output mode로 설정
	ECHO_DDR_RIGHT &= ~(1 << ECHO_RIGHT);  // input mode로 설정
	
	EICRB |= (0 << ISC61) | (1 << ISC60);  //  0 1 : 어떠한 형태로든지 신호 변화가 발생되면 interrupt 요청 (상승, 하강 edge 모두다 interrupt)
	TCCR3B |= (1 << CS32) | (1 << CS30);   //  1024 분주
	// EICRA : INT 0~3
	// EICRB : INT 4~7
	// 0   1 : 어떠한 신호의 변화가 있는지 INT를 요청한다. (rising, falling edge 둘다 int 발생)
	// 16비트 timer 1번을 1024분주해서 공급
	// 16MHz를 1024 분주  ==>  16000000/1024 ==> 15.625kHz
	// 1펄스의 길이 : t = 1/f = 64us
	
	EIMSK |= (1 << INT6);   //  외부 INT4(ECHO) 사용
}
void distance_check(void)  // 
{
	ultrasonic_trigger();
	
	//printf("left: %5dcm\n", left_distance);
    //printf("center: %5dcm\n", center_distance);
	//printf("right: %5dcm\n", right_distance);
}
int u_timer = 0;
void ultrasonic_trigger()
{
	switch(u_timer)
	{
		case 1 :	
		//======== LEFT ULTRASONIC ==============
		TRIG_PORT_LEFT &= ~(1 << TRIG_LEFT);  //  low
		_delay_us(1);
		TRIG_PORT_LEFT |= (1 << TRIG_LEFT);   //  high
		_delay_us(15);
		TRIG_PORT_LEFT &= ~(1 << TRIG_LEFT);  //  low
		break;

		case 2 :
		//======== CENTER ULTRASONIC ==============
		TRIG_PORT_CENTER &= ~(1 << TRIG_CENTER);  //  low
		_delay_us(1);
		TRIG_PORT_CENTER |= (1 << TRIG_CENTER);   //  high
		_delay_us(15);
		TRIG_PORT_CENTER &= ~(1 << TRIG_CENTER);  //  low
		break;
	
		case 3 :
		//======== RIGHT ULTRASONIC ==============
		TRIG_PORT_RIGHT &= ~(1 << TRIG_RIGHT);  //  low
		_delay_us(1);
		TRIG_PORT_RIGHT |= (1 << TRIG_RIGHT);   //  high
		_delay_us(15);
		TRIG_PORT_RIGHT &= ~(1 << TRIG_RIGHT);  //  low
		u_timer =0;
		break;
	
	 //  초음파 센서 echo 응답시간 최대 38ms
	//  _delay_ms 를 timer INT로 처리한다. delay말고 timer 로 처리
	
	}
	u_timer++;
}
