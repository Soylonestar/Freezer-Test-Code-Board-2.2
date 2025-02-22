# Freezer-Test-Code-Board-2.2
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

void timer1_16_bit (void);
void GPIO_init (void);
void ADC_init (void);

void read_ADC(void);

void USART0_TX_init (void);
void USART0_TX_Data(char Data); //transmit char data type to serial
void USART0_Data (const char *str); //divides up the string data with addresses

void LED_init (void);
void LED_Toggle(void);
void INT5_init(void); //testing timer with RED_LED by using Interrupts

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
	
	_delay_ms(10);
	
    while(1)
    {
         /*
		 if (adc_conv_complete)
		 {
			adc_conv_complete = false; // resets the conversion indicator to 0
			
			USART0_Data(volt_text); //sends String to be divided then sent
			
			sprintf(adc_temp, "%.3f", adc_val); //Formal Specifier %3f is Signed float with 3 decimal places; Converts Double to String
			USART0_Data(adc_temp); //transmit ADC data "String" to serial
			
			USART0_Data(" V \n"); //sends String to be addressed then sent
			
			LED_Toggle(); //toggles the Green LED
		 }
		 */
		 USART0_Data(volt_text); 
		 read_ADC();
		 sprintf(adc_temp, "%.3f", adc_val);
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
	timer1_16_bit(); //enabling Timer1/Timer3 counter
	USART0_TX_init(); //enabling USART0 TX
	INT5_init(); //enable External Interrupt for INT5
	
	sei(); //enabling Global Interrupt Bit
}

void timer1_16_bit(void) //has two timers to verify that OC1B Timer works...
{
	/*
	//Timer 1 initialize; OC1B (PB6); Timer 1 Channel B
	TCCR1A &= ~(1 << WGM11) | ~(1 << WGM10) | (1 << COM1B1); //WGM11-10 are set for CTC mode; COM1B1 is set for Toggle on Compare Match
	TCCR1A |= (1 << COM1B0); //COM1B0 set for Toggle on Compare Match
	
	TCCR1B &= ~(1 << WGM13) | ~(1 << CS11) | ~(1 << CS10); //WGM13 is set for CTC mode; CS11-10 are set for prescalar of 256
	TCCR1B |= (1 << WGM12) | (1 << CS12); //WGM12 is set for CTC mode; CS12 is set for prescalar of 256
	
	OCR1A = 31249; //Compare Value for 16-bit timer; TOP value; set for 1 Hz with prescalar of 256
	*/
	
	//Timer 3 initialize; OC3C (PE5); Timer 3 Channel C
	TCCR3A &= ~(1 << WGM31) | ~(1 << WGM30) | (1 << COM1C1); //WGM11-10 are set for CTC mode; COM1C1 is set for Toggle on Compare Match
	TCCR3A |= (1 << COM1C0); //COM1C0 set for Toggle on Compare Match
	
	TCCR3B &= ~(1 << WGM33) | ~(1 << CS31) | ~(1 << CS30); //WGM33 is set for CTC mode; CS31-30 are set for prescalar of 256
	TCCR3B |= (1 << WGM32) | (1 << CS32); //WGM32 is set for CTC mode; CS32 is set for prescalar of 256
	
	OCR3A = 31249; //Compare Value for 16-bit timer; TOP value; set for 1 Hz with prescalar of 256
}

ISR (INT5_vect) //Interrupt Service Routine for Timer and Blinking RED LED
{
	PORTH ^= (1 << PH6);
}

void INT5_init(void) //INT5 (PE5)
{
	//setting up PE5 (INT5) interrupt; not input because there is no button or anything that'll cause change
	DDRE |= (1 << PE5); //pin set as on output for PE5; 
	
	//setting up the INT5 interrupt
	EICRB |= (1 << ISC51) | (1 << ISC50); //setting the External Interrupt Control; with Interrupt Sense Control for rising edge
	EIMSK |= (1 << INT5); //enabling the External Interrupt
}

//Keep ISR as short as possible...
/*
ISR (ADC_vect) //Interrupt Service Routine for the Analog to Digital Conversion
{ 
	adc_conv_complete = true; //set to 1 for main if-statement telling the the conversion is complete
	
	//Since our ADC Auto Trigger uses Timer1 Compare Match B (OC1B)
	TIFR1 |= (1 << OCF1B); //Timer1 Interrupt Flag Register enabled to reset flag
	
	//adc_const is getting more accurate...
	adc_val = ADC * 0.0402229606331169; //Data Conversion with Constant = V_batt / (Vin * 2^10)/Vref; Vin needed a voltage divider	
}
*/
void ADC_init(void) //initializes Analog to Digital Converter; ADC0(PF0) pin, Channel 0;
{
	//setting Voltage Reference for ADC to AVCC (5V) with 0.1uF external capacitor at AREF pin
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1); 
	
	ADCSRA |= (1 << ADEN); // enables the ADC
	
	/*
	//bottom three go together, because both are about the trigger
	ADCSRA |= (1 << ADATE); // enables ADC Auto Trigger Enable bit; meaning ADC will start conversion on a positive edge of the selected trigger signal
	
	// setting ADC Auto Trigger Source to OC1B (TImer1 Compare Match B)
	ADCSRB |= (1 << ADTS2) | (1 << ADTS0); 
	ADCSRB &= ~(1 << ADTS1);
	*/

	// setting ADC Prescalar Selection to a Division Factor of 128; F_CPU / 128 = 125kHz within 50kHz and 200kHz necessary for conversion time
	ADCSRA |= (1 << ADPS1) | (1 << ADPS2) | (1 << ADPS0);
	
	DIDR0 |= (1 << ADC0D); //enabling Digital Input Disable; digital input buffer on the corresponding ADC pin is disabled

	//enabling ADC Conversion Complete Interrupt
	//ADCSRA |= (1 << ADIE); // enables ADC Interrupt
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
	DDRH |= (1 << PH5) | (1 << PH6); //enabling Green_LED and RED_LED pins as output

	PORTH |= (1 << PH6) | (1 << PH5); //initializing both LED off
}

void LED_Toggle (void)
{
	PORTH ^= (1 << PH5); //toggles the Green LED
}
