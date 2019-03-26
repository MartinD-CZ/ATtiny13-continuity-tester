/*
 * ContinuityMeter.cpp
 *
 * Created: 20.05.2018 21:57:33
 * Author: Martin Danek
 * IC: ATtiny13
 * Fuses: L 0x6A, H 0xFF
 * More info: http://embedblog.eu/?p=194
 * Inspired by: http://www.technoblogy.com/show?1YON
 */ 

#define F_CPU 1200000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED_ON_HIGH		PORTB |= (1 << PORTB2)
#define LED_ON_LOW		PORTB &=~(1 << PORTB2)
#define LED_CONT_HIGH	PORTB |= (1 << PORTB3)
#define LED_CONT_LOW	PORTB &=~(1 << PORTB3)
#define BEEP_HIGH		PORTB |= (1 << PORTB4)
#define BEEP_LOW		PORTB &=~(1 << PORTB4)

//define how long it waits before going to sleep
#define SLEEP_MS		10000

volatile uint16_t ms = 0;

int main(void)
{
    //PB2:4 as outputs
	DDRB |= (1 << DDB2) | (1 << DDB3) | (1 << DDB4);

	//pullups on PB1 (probe) and PB0 (reference)
	PORTB |= (1 << PORTB0) | (1 << PORTB1);

	//enable analog comparator
	ACSR |= (1 << ACIE);

	//Timer0 keeps track of time - increases the ms variable
	TCCR0B |= (1 << CS01);			//clk/8
	OCR0A = 150;
	TIMSK0 |= (1 << OCIE0A);

	//sleep
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	//enable INT
	sei();

	LED_ON_HIGH;

    while (1); 
}

//analog comparator interrupt routine
ISR(ANA_COMP_vect)
{
	ms = 0;
	if(ACSR & (1 << ACO))
	{
		LED_CONT_HIGH;
		BEEP_HIGH;
	}
	else
	{
		LED_CONT_LOW;
		BEEP_LOW;
	}
}

//INT0 vector - wakes the IC from sleep
ISR(INT0_vect)
{
	sleep_disable();
	ms = 0;
	GIMSK &=~(1 << INT0);
	//cli();
}

//TIMER0 COMPARE INTERRUPT - sends the IC to sleep
ISR(TIM0_COMPA_vect)
{
	TCNT0 = 0;
	ms++;
	if (ms >= SLEEP_MS)
	{
		//we are going to sleep
		LED_ON_LOW;
		PORTB &=~(1 << PORTB0);
		GIMSK |= (1 << INT0);
		sei();
		sleep_bod_disable();
		sleep_enable();
		sleep_cpu();

		//SLEEP, until INT0 wakes the IC up

		sleep_disable();
		sei();
		LED_ON_HIGH;
		PORTB |= (1 << PORTB0);
	}
}

