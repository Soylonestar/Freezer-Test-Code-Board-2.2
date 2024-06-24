/*
 * main.c
 *
 * Created: 6/5/2024 2:26:20 PM
 *  Author: agarciasampedroperez
 */ 

//////This is for V 2.2 Boards////////

#define F_CPU 16000000UL //system clock 16MHz
#define USART_0_BAUD 9600 //baud rate of USART

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

void GPIO_init (void);
void ADC_init (void);

void read_ADC(void);

void USART0_TX_init (void);
void USART0_TX_Data(char Data); //transmit char data type to serial
void USART0_Data (const char *str); //divides up the string data with addresses

void LED_init (void);
void LED_Toggle(void);

volatile double adc_val; //this is for the 16-bit data of ADC data register
bool adc_conv_complete = false; 

char volt_text [] = "Battery Voltage: ";
char adc_temp[16]; //creating an array of length 10 (minimum), because this ADC Data register is 10 bits

int main(void)
{
	GPIO_init(); //General Purpose Initializations function; 

	ADC_init(); //enabling Analog to Digital Conversion
	
	_delay_ms(10);
	
    while(1)
    {
		 USART0_Data(volt_text); 
		 read_ADC();
		 sprintf(adc_temp, "%.3f", adc_val); //Converts Double to String
		 USART0_Data(adc_temp);
		 USART0_Data(" V \n");
		 LED_Toggle();
		 _delay_ms(300);
    }
	return 0;
}

void GPIO_init(void) //General Purpose Initializations function
{
	LED_init(); //enabling GREEN_LED and RED_LED
	USART0_TX_init(); //enabling USART0 TX

	sei(); //enabling Global Interrupt Bit
}

void ADC_init(void) //initializes Analog to Digital Converter; ADC0(PF0) pin, Channel 0;
{
	//setting Voltage Reference for ADC to AVCC (5V) with 0.1uF external capacitor at AREF pin
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1); 
	
	ADCSRA |= (1 << ADEN); // enables the ADC

	// setting ADC Prescalar Selection to a Division Factor of 128; F_CPU / 128 = 125kHz within 50kHz and 200kHz necessary for conversion time
	ADCSRA |= (1 << ADPS1) | (1 << ADPS2) | (1 << ADPS0);
	
	DIDR0 |= (1 << ADC0D); //enabling Digital Input Disable; digital input buffer on the corresponding ADC pin is disabled
}

void read_ADC() //reads ADC data x amount of times and then takes the average of it
{
	for (int i = 0; i < 16; i++)
	{
		//start single conversion manually
		ADCSRA |= (1 << ADSC);
		
		//wait for conversion to complete
		while(!(ADCSRA & (1 << ADIF)));
		
		//Reading the ADC data and adding it into the variable
		adc_val += ADC;
	}
	//takes the average (16) and multiplies by the conversion constant
	adc_val = adc_val * 0.002513935039569810;
}

void USART0_TX_init(void) //TXD0(PE1)
{
	//Set Internal Clock Generator or Baud Rate
	UBRR0 = ((float)(F_CPU / (16 * (float)USART_0_BAUD)) - 1); //Equation in Pg 203 for calculating UBRR Value
	
	//Set pin and transmitter for USART
	DDRE |= (1 << PE1); //enabling TX pin as output
	UCSR0B |= (1 << TXEN0); //enabling Transmitter for USART overriding pin (PE1)

	//Turns on the RS232_01 so that TX goes to serial port; we do this because this is an active low shutdown;
	DDRA |= (1 << DDA2); //setting PA2 for RS232 USART0 as output
	PORTA |= (1 << PA2); //"enabling" RS232 USART0 (turning on)
}

void USART0_Data (const char *str) //addresses String Data by the individual character addresses
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		USART0_TX_Data(str[i]); //sends char data to Transmit function
	}
}

void USART0_TX_Data(char Data) //this is my data to serial transmission function
{
	while (!(UCSR0A & (1 << UDRE0))) //wait for empty transmit buffer...
	{
		;
	}
	UDR0 = Data; //Puts data into the buffer, sends the data through TX
}

void LED_init(void)
{
	DDRH |= (1 << PH5); //enabling Green_LED pin as output

	PORTH |= (1 << PH5); //initializing LED off
}

void LED_Toggle (void)
{
	PORTH ^= (1 << PH5); //toggles the Green LED
}
