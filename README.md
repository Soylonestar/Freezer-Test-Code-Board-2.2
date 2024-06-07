# Freezer-Test-Code-Board-2.2

/*
 * main.c
 *
 * Created: 6/5/2024 2:26:20 PM
 *  Author: agarciasampedroperez
 */ 

#define F_CPU 16000000UL //system clock 16MHz
#define USART_0_BAUD 9600 //baud rate of USART

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

void timer1_16_bit (void);
void GPIO_init (void);
void ADC_init (void);

void USART0_TX_init (void);
void USART0_print(void);
void USART0_sendChar(unsigned char str);
//void USART0_print(const char *str);

void LED_init (void);
void LED_Toggle(void);

void interrupt_init(void); //testing timer with RED_LED by using Interrupts

uint16_t volatile adc_val; //unsigned 16-bit integer called adc_val, volatile because data may change
int adc_conv_complete = 0;

int main(void)
{
	GPIO_init(); //enabling the General Purpose I/O Register 0
	timer1_16_bit(); //enabling Timer1 counter
	interrupt_init(); //enable interrupt
	ADC_init(); //enabling Analog to Digital Conversion
	sei(); //enabling Global Interrupt Bit
	
    while(1)
    {
         if (adc_conv_complete == 1)
		 {
			// USART0_print(); //transmits the adc_val through the serial
			 adc_conv_complete = 0; // resets the conversion indicator to 0
		 }
    }
	return 0;
}

void timer1_16_bit(void) //has two timers to verify that OC1B Timer works...
{
	//Timer 1 initialize; OC1B (PB6); Timer 1 Channel B
	TCCR1A &= ~(1 << WGM11) | ~(1 << WGM10) | (1 << COM1B1); //WGM11-10 are set for CTC mode; COM1B1 is set for Toggle on Compare Match
	TCCR1A |= (1 << COM1B0); //COM1B0 set for Toggle on Compare Match
	
	TCCR1B &= ~(1 << WGM13) | ~(1 << CS11) | ~(1 << CS10); //WGM13 is set for CTC mode; CS11-10 are set for prescalar of 256
	TCCR1B |= (1 << WGM12) | (1 << CS12); //WGM12 is set for CTC mode; CS12 is set for prescalar of 256
	
	OCR1A = 31249; //Compare Value for 16-bit timer; TOP value; set for 1 Hz with prescalar of 256
	
	//Timer 1 initialize; OC3C (PE5); Timer 3 Channel C
	TCCR3A &= ~(1 << WGM31) | ~(1 << WGM30) | (1 << COM1C1); //WGM11-10 are set for CTC mode; COM1C1 is set for Toggle on Compare Match
	TCCR3A |= (1 << COM1C0); //COM1C0 set for Toggle on Compare Match
	
	TCCR3B &= ~(1 << WGM33) | ~(1 << CS31) | ~(1 << CS30); //WGM33 is set for CTC mode; CS31-30 are set for prescalar of 256
	TCCR3B |= (1 << WGM32) | (1 << CS32); //WGM32 is set for CTC mode; CS32 is set for prescalar of 256
	
	OCR3A = 31249; //Compare Value for 16-bit timer; TOP value; set for 1 Hz with prescalar of 256
	
}

ISR (INT5_vect) //Interrupt Service Routine for Timer and Blinking LED
{
	PORTH ^= (1 << PH6);
}

void interrupt_init(void) //INT5(PE5)
{
	//setting up PE5 (INT5) interrupt; not input because there is no button or anything that'll cause change
	DDRE |= (1 << PE5); //pin set as on output for PE5; 
	
	//setting up the INT5 interrupt
	EICRB |= (1 << ISC51) | (1 << ISC50); //setting the External Interrupt Control; with Interrupt Sense Control for rising edge
	EIMSK |= (1 << INT5); //enabling the External Interrupt;
	
}

ISR (ADC_vect) //Interrupt Service Routine for the Analog to Digital Conversion
{ 
	adc_conv_complete = 1; //set to 1 for main if-statement telling the the conversion is complete
	LED_Toggle(); //toggles the Green LED
	TIFR1 |= (1 << OCIE1B); //Timer1 Interrupt Flag Register
}

void ADC_init(void) //initializes Analog to Digital Converter; ADC0(PF0) pin, Channel 0;
{
	ADMUX |= (1 << REFS0); //setting Voltage Reference for ADC to AVCC with external capacitor at AREF pin
	ADMUX &= ~(1 << REFS1); //setting Voltage Reference for ADC to AVCC with external capacitor at AREF pin
	
	ADCSRA |= (1 << ADEN); // enables the ADC
	
	//bottom two go together, because both are about the trigger
	ADCSRA |= (1 << ADATE); // enables ADC Auto Trigger Enable bit; meaning ADC will start conversion on a positive edge of the selected trigger signal
	ADCSRB |= (1 << ADTS2) | (1 << ADTS0); // setting ADC Auto Trigger Source to OC1B (TImer1 Compare Match B)
	ADCSRB &= ~(1 << ADTS1); // setting ADC Auto Trigger Source to OC1B (TImer1 Compare Match B)
	
	ADCSRA |= (1 << ADIE); // enables ADC Interrupt Enable
	
	while (!(ADCSRA & (1 << ADIF))); // polling until ADC Interrupt Flag
	
	ADCSRA |= (1 << ADPS1) | (1 << ADPS2) | (1 << ADPS0); // setting ADC Prescalar Selection to a Division Factor of 128; F_CPU / 128 = 125kHZ within 50 - 200KHz
	
	DIDR0 |= (1 << ADC0D); //enabling Digital Input Disable; digital input buffer on the corresponding ADC pin is disabled
}

void GPIO_init(void) //General Purpose Register
{
	USART0_TX_init(); //enabling USART0 TX
	LED_init(); //enabling GREEN_LED and RED_LED
	DDRF &= ~(1 << PF0); //enabling the ADCO pin as input (batt_ref), no pull-up resistors needed despite that
	
	//may need to enable this...
	PORTF &= ~(1 << PF0); //initializing to disable anything on that path
	
}

void USART0_TX_init(void) //TXD0(PE1)
{
	//Set Internal Clock Generator or Baud Rate
	UBRR0 = ((float)(F_CPU / (16 * (float)USART_0_BAUD)) - 1); //Equation in Pg 203 for calculating UBRR Value
	
	//Set pin and transmitter for USART
	DDRE |= (1 << PE1); //enabling TX pin as output
	UCSR0B |= (1 << TXEN0); //enabling Transmitter for USART overriding pin (PE1)
	
//CHECK WITH FORBES....
	//Turns on the RS232_01 so that TX goes to serial port
	DDRA |= (1 << DDA2); //setting PA2 for RS232 USART0 as output
	PORTA |= (1 << PA2); //enabling RS323 USART0 (turning on)
	
}

void USART0_sendChar(unsigned char str)
{
	while (!(UCSR0A & (1<< UDRE0))) //wait for empty transmit buffer...
	{
		;
	}
	UDR0 = str; //Puts data into the buffer, sends the data through TX
}

/*void USART0_print (const char *str)
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		USART0_sendInt(str[]);
	}
}
*/
void LED_init(void)
{
	DDRH |= (1 << PH5); //enabling Green_LED pin as output
	DDRH |= (1 << PH6); //enabling RED LED as output
}

void LED_Toggle (void)
{
	PORTH ^= (1 << PH5); //toggles the LED
	
	/*PORTH &= ~(1 << PH5); //
	_delay_ms(200);
	PORTH |= (1 << PH5);*/
}
