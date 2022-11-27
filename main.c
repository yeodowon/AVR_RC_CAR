/*
 * RC_CAR.c
 *
 * Created: 2022-08-18 오전 9:48:43
 * Author : kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h> // printf, scanf 등이 정의 되어 있다.
#include <string.h> // strcpy, strcat, strcmp 등이 들어 있음


extern void UART0_transmit(uint8_t data);
extern void init_UART1();


// 외부파일에 있는 변수를 참조는 extern로 선언

extern int get_button1();
extern void init_button();
extern void init_uart0();
extern void init_led();
extern void distance_check(void);
extern void init_ultrasonic();
extern int button1_state;

extern volatile int left_distance;
extern volatile int center_distance;
extern volatile int right_distance;



// 1. FOR printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);



uint32_t ms_count1=0;		// ms를 측정
uint32_t ms_count2=0;
uint32_t sec_count=0;

void init_timer0();

int auto_state = 0;


volatile int state = 0;
volatile int state1 = 0;
volatile int auto_timer = 0;


// portb.0 : 500ms on/off
// portb.1 : 300ms on/off
// 1. 분주 : 64분주  ==>  16000000 / 64 = 250000Hz
// 2. T 계산 ==>  1/f = 1/250000 = 0.000004sec (4us)
// 3. 8 bit Timer OV : 4us * 256 = 1.024ms
// 256개의 pulse 를 count 하면 이곳으로 온다
//

ISR(TIMER0_OVF_vect) // 인터럽트 루틴을 길게 짤 수록 output delay가 증가하여 원하는 시간 이내에  출력이 되지 않음
{
	TCNT0 = 6; // TCNT를 6~256 == > 정확히 1ms 를 유지하기 위해
	// TINT 0 OVF INT
	ms_count1++;
	

	if (ms_count1 >= 1000)   // 1000ms ==> 1sec
	{
		ms_count1=0;
		sec_count++;    // sec counter 증가
		auto_timer++;
	}

}




/*
	1. LEFT MOTORi
		PORT.0 : IN1
		PORT.1 : IN2
	2. RIGHT MOTOR
		PORTF.2 : IN3
		PORTF.3 : IN4
		IN1.IN3		IN2.IN4
		=======    ========
			0		1 : 역회전
			1		0 : 정회전
			1		1 : 정지
*/

#define MOTOR_DRIVER_PORT PORTF
#define MOTOR_DRIVER_PORT_DDR DDRF

#define MOTOR_DDR  DDRB
#define MOTOR_RIGHT_PORT_DDR 5 // OC1A
#define MOTOR_LEFT_PORT_DDR 6 // OC1B
ISR(TIMER3_OVF_vect)
{
	TCNT3 = 6;
	auto_timer++;
}

void init_pwm_motor(void)
{
	MOTOR_DRIVER_PORT_DDR |= (1 << 0) | ( 1 << 1) | ( 1 << 2) | (1 << 3); // MOTOR DIRVER PORT
	MOTOR_DDR |= (1 << MOTOR_RIGHT_PORT_DDR) | (1 << MOTOR_LEFT_PORT_DDR); // PWM PORT
/*	MOTOR_DRIVER_PORT_DDR &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT_DDR &= 0b11110000;
*/
	TCCR1B |= (0 << CS12) | ( 1 << CS11) | ( 1 << CS10) ; // 64분주 , 16000000Hz/64 -> 250000Hz(Timer 1번에 공급되는 clock)
	// 256 / 250000Hz -> 1.02ms (펄스를 256개를 count하면 1.02가 걸린다.)
	// 127 / 250000 -> 0.5ms
	// 0x3ff(1023) ==> 4ms
	
	TCCR1B |= (1 << WGM13) | (1 << WGM12); // 모드 14 고속PWM ICR1
	TCCR1A |= (1 << WGM11) | (0 << WGM10); // 모드 14 고속PWM ICR1
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0); //  비반전 모드 : OCR LOW , TOP HIGH
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);
	
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |=(1 << 2) | (1 << 0); // 자동차를 전진 모드로 setting
	ICR1 = 0x3ff; // 1023 -> 4ms
}


void init_timer0()
{
	TCNT0 = 6;
	
	TCCR0 |= (1 << CS02) | (0 << CS01) | (0 << CS00);  //  분주비를 64로 설정
	
	TIMSK |= (1 << TOIE0);	
}



int main(void)
{
	
	init_led();
	init_button();
	init_uart0(); // UART0를 초기화
	init_UART1(); // UART1를 초기화
	init_timer0();
	init_pwm_motor();
	init_ultrasonic();
	
	stdout = &OUTPUT; // for printf /fprintf(stdout, "test"); ==> printf stdin : 입력
	sei();			 // global interrupt 활성화
	
	while (1)
	{
		manual_mode_run();  //  bluetooth car command run
		distance_check();   //  초음파 거리 측정
		auto_mode_check();  //  button1 check
		
		if(button1_state)
		{
			auto_drive();
		}
	}
}

//  자동 주행 프로그램

void auto_drive(void)
{
	auto_state=0;

	if(center_distance <60) 
	{
		if(left_distance > right_distance)  //  좌회전
		{
			auto_state = 1;
		
		}
		else if(left_distance < right_distance)  //  우회전
		{
			auto_state = 2;
			
		}
	}
	if(center_distance <10) 
			
			auto_state = 3;
	
	
//=========================================================================================================================
	switch(auto_state)
	{
		case 0 :  // 직진
		forward(1000);
		break;
		
		
		case 1 :   //  좌회전
		if((center_distance < 60) && (left_distance > right_distance)) 
		{

				turn_left(900);
			
		}
		else if(center_distance >= 30)
		{
			break;
		}
		
		case 2 :   //  우회전
		if((center_distance < 60) && (left_distance < right_distance))
		{
				turn_right(900);
		}
		else if(center_distance >= 30)
		{
			break;
		}
		
		case 3 :   //  후진
		if(center_distance < 10) 
		{
			backward(1000);
		}
		else if(center_distance >= 30)
		{
			break;
		}
	}
	
	auto_state = 0;
}


extern volatile unsigned char UART1_RX_data;

void manual_mode_run(void)
{
	switch(UART1_RX_data)
	{
		case 'F':
		forward(700);
		break;
		case 'B':
		backward(700);
		break;
		case 'L':
		turn_left(900);
		break;
		case 'R':
		turn_right(900);
		break;
		case 'S':
		stop();
		break;
		
	}
}

void forward(int speed)
{
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |=(1 << 2) | (1 << 0); // 자동차를 전진 모드로 setting
	
	OCR1A = speed; // PB4 PWM출력 left
	OCR1B = speed; // PB5 PWM출력 right
}

void backward(int speed)
{
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |=(1 << 3) | (1 << 1); // 0101자동차를 후진 모드로 setting
	
	OCR1A = speed; // PB4 PWM출력 left
	OCR1B = speed; // PB5 PWM출력 right
}

void turn_left(int speed)
{
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |=(1 << 2) | (1 << 0); // 자동차를 전진 모드로 setting
	
	OCR1A = speed; // PB4 PWM출력 left
	OCR1B = 0; // PB5 PWM출력 right
}

void turn_right(int speed)
{
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |=(1 << 2) | (1 << 0); // 자동차를 전진 모드로 setting
	
	OCR1A = 0; // PB4 PWM출력 left
	OCR1B = speed; // PB5 PWM출력 right
}



void stop(void)
{
	OCR1A = 0; // PB4 PWM출력 left
	OCR1B = 0; // PB5 PWM출력 right
	
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |= (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0); // 자동차를 stop	
}


void auto_stop(void)
{
	
	auto_timer = 0;
	
	if((auto_timer >= 0) && (auto_timer <= 1000))
	{
	OCR1A = 0; // PB4 PWM출력 left
	OCR1B = 0; // PB5 PWM출력 right
	
	MOTOR_DRIVER_PORT &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	MOTOR_DRIVER_PORT |= (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0); // 자동차를 stop	
	}
}