/*
 * main.c
 *
 * Created: 6/11/2024 3:50:22 PM
 *  Author: agarciasampedroperez
 */ 

//////This is for V2.1 Boards///////

#define F_CPU 16000000UL //system clock 16MHz
#define USART_1_BAUD 9600 //baud rate of USART

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

void timer1_16_bit (void);
void GPIO_init (void);
void ADC_init (void);

void USART1_TX_init (void);
void USART1_TX_Data(char Data); //transmit char data type to serial
void USART1_Data (const char *str); //divides up the string data with addresses

void LED_init (void);
void LED_Toggle(void);

//determine what volatile is exactly; gives the variable the opportunity to change in the ISR without being optimized in main;
volatile double adc_val; //this is for the 16-bit data of ADC data register
bool adc_conv_complete = false; 

char volt_text [] = "Battery Voltage: ";
char adc_temp[16]; //creating an array of length 10 (minimum), because this ADC Data register is 10 bits

int main(void)
{
	GPIO_init(); //General Purpose Initializations function; enabling the General Purpose I/O Register 0 (probably not associated despite name)
	
	//According to data sheet, pg 276, conversion takes care of using the pin as input...
	//DDRF &= ~(1 << PF0); //enabling the ADCO pin as input (batt_ref), no pull-up resistors needed despite that

	ADC_init(); //enabling Analog to Digital Conversion
	
    while(1)
    {
         if (adc_conv_complete)
		 {
			adc_conv_complete = false; // resets the conversion indicator to 0
			
			USART1_Data(volt_text); //sends String to be divided then sent
			
			sprintf(adc_temp, "%.3f", adc_val); //Formal Specifier %3f is Signed float with 3 decimal places; Converts Double to String
			USART1_Data(adc_temp); //transmit ADC data "String" to serial
			
			USART1_Data(" V \n"); //sends String to be addressed then sent
			
			LED_Toggle(); //toggles the Green LED
		 }
    }
	return 0;
}

void GPIO_init(void) //General Purpose Initializations function
{
	LED_init(); //enabling GREEN_LED and RED_LED
	timer1_16_bit(); //enabling Timer1/Timer3 counter
	USART1_TX_init(); //enabling USART0 TX
	
	sei(); //enabling Global Interrupt Bit
}

void timer1_16_bit(void) //had two timers to verify that OC1B Timer works...
{
	//Timer 1 initialize; OC1B (PB6); Timer 1 Channel B
	TCCR1A &= ~(1 << WGM11) | ~(1 << WGM10) | (1 << COM1B1); //WGM11-10 are set for CTC mode; COM1B1 is set for Toggle on Compare Match
	TCCR1A |= (1 << COM1B0); //COM1B0 set for Toggle on Compare Match
	
	TCCR1B &= ~(1 << WGM13) | ~(1 << CS11) | ~(1 << CS10); //WGM13 is set for CTC mode; CS11-10 are set for prescalar of 256
	TCCR1B |= (1 << WGM12) | (1 << CS12); //WGM12 is set for CTC mode; CS12 is set for prescalar of 256
	
	OCR1A = 62499; //Compare Value for 16-bit timer; TOP value; set for 1 Hz with prescalar of 256
}
	
/* 
	//Question: The RED_LED blinks without our input, why?//

	// OC1B, since the RED_Led is on the same pin, the timer override pin operations, so when the Timer is at low the LED is on...
	// Pg. 177, to prevent RED_LED blinking must disable the Data Direction Register (DDR) of the LED pin...

ISR (INT5_vect) //Interrupt Service Routine for Timer and Blinking RED LED
{
	PORTB ^= (1 << PB6); //red led look above for reason as to why the LED blinks without our input ^^
}
*/

//Keep ISR as short as possible...
ISR (ADC_vect) //Interrupt Service Routine for the Completion of the Analog to Digital Conversion
{ 
	adc_conv_complete = true; //set to 1 for main if-statement telling the the conversion is complete
	
	//Since our ADC Auto Trigger uses Timer1 Compare Match B (OC1B)
	TIFR1 |= (1 << OCF1B); //Timer1 Interrupt Flag Register enabled to reset flag
	
	//adc_const is more accurate on this board...
	adc_val = ADC *0.01803430864601510; //Data Conversion with Constant = V_batt / (Vin * 2^10)/Vref; Vin needed a voltage divider	
}

void ADC_init(void) //initializes Analog to Digital Converter; ADC0(PF0) pin, Channel 0;
{
	//I don't need this because PRADC initializes at 0; disabled;
	PRR0 &= ~(1 << PRADC); //Disabling the Power Reduction ADC

	//setting Voltage Reference for ADC to AVCC (5V) with external capacitor at AREF pin
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1); 
	
	ADCSRA |= (1 << ADEN); // enables the ADC
		
	// setting ADC Prescalar Selection to a Division Factor of 128; F_CPU / 128 = 125kHz within 50kHz and 200kHz necessary for conversion time 
	ADCSRA |= (1 << ADPS1) | (1 << ADPS2) | (1 << ADPS0); 
	
	DIDR0 |= (1 << ADC0D); //enabling Digital Input Disable; digital input buffer on the corresponding ADC pin is disabled

	//bottom three go together, because both are about the trigger
	ADCSRA |= (1 << ADATE); // enables ADC Auto Trigger Enable bit; meaning ADC will start conversion on a positive edge of the selected trigger signal
	
	// setting ADC Auto Trigger Source to OC1B (TImer1 Compare Match B)
	ADCSRB |= (1 << ADTS2) | (1 << ADTS0);
	ADCSRB &= ~(1 << ADTS1);

	//enabling ADC Conversion Complete Interrupt
	ADCSRA |= (1 << ADIE); // enables ADC Interrupt
}

void USART1_TX_init(void) //TXD1(PD3)
{
	//Set Internal Clock Generator or Baud Rate
	UBRR1 = ((float)(F_CPU / (16 * (float)USART_1_BAUD)) - 1); //Equation in Pg 203 for calculating UBRR Value
	
	//Set pin and transmitter for USART
	DDRD |= (1 << PD3); //enabling TX pin as output
	UCSR1B |= (1 << TXEN1); //enabling Transmitter for USART overriding pin (PD3)

	//Turns on the RS232_01 so that TX goes to serial port; we do this because this is an active low shutdown;
	DDRA |= (1 << DDA2); //setting PA2 for RS232 USART1 as output
	PORTA |= (1 << PA2); //"enabling" RS232 USART1 (turning on)
}

void USART1_Data (const char *str) //addresses String Data by the individual character addresses
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		USART1_TX_Data(str[i]); //sends char data to Transmit function
	}
}

void USART1_TX_Data(char Data) //this is my data to serial transmission function
{
	while (!(UCSR1A & (1 << UDRE1))) //wait for empty transmit buffer...
	{
		;
	}
	UDR1 = Data; //Puts data into the buffer, sends the data through TX
}

void LED_init(void)
{
	DDRH |= (1 << PH5); //enabling Green_LED pin as output
	//DDRB |= (1 << PB6); //enabling RED LED as output

	PORTH |= (1 << PH5); //initializes the Green_LED off
	//PORTH |= (1 << PB6); //initializes the Red_LED off
}

void LED_Toggle (void)
{
	PORTH ^= (1 << PH5); //toggles the Green LED
}
