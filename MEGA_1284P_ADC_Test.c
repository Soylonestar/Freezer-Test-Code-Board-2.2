/*
 * main.c
 *
 * Created: 6/14/2024 12:06:48 PM
 *  Author: agarciasampedroperez
 */ 

//////MEGA_1284P Board////////////

#define F_CPU 11059200UL //system clock 11.0592MHz
#define USART_1_BAUD 57600 //baud rate of USART

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>

void usart1_init(void);
void adc_init(void);
void timer1_init(void);
void usart1_Data(const char *str);
void usart1_TX_Data (char Data);
void LED_init(void);
void LED_toggle(void);

volatile double adc_data;
char volt_text [] = "Voltage: ";
char test [] = "Hello World";
char adc_temp [16];
bool conv_complete = false;

/*
#if defined( __GNUC__ )
** Make sure printf knows where to print. The macro fdev_setup_stream()
 * is used to prepare a user-supplied FILE buffer for operation with stdio.
 */
/*
FILE usart1_str = FDEV_SETUP_STREAM((int(*)(char, FILE *))usart1_putchar, NULL, _FDEV_SETUP_WRITE);
#endif
*/

int main(void)
{
	sei(); //enabling the Global Interrupt bit
	usart1_init(); 
	adc_init();
	timer1_init();
	LED_init();
	
	/*#if defined (_GNUC__)
		* Setup stdout to point to the correct usart (USART1). This is needed to
		* use the fdev_setup_stream() macro in GCC.
		*/
		/*stdout = &usart1_str;
	#endif*/
	
    while(1)
    {
		if (conv_complete)
		{
			conv_complete = false; // resets the conversion indicator to 0
			
			usart1_Data(volt_text); //sends String to be divided then sent
			
			sprintf(adc_temp, "%.3f", adc_data); //Formal Specifier %3f is Signed float with 3 decimal places; Converts Double to String
			usart1_Data(adc_temp); //transmit ADC data "String" to serial
			
			usart1_Data(" V \n"); //sends String to be addressed then sent
			
			LED_toggle();
		} 
		//usart1_Data(test);
		//_delay_ms(200);
		//LED_toggle();
		
	}
}

/** \brief Function to setup USART1.
 *  //////We DO NOT screw with this because this communicates the USB to Board Controller to Atmega1284P//////
 * 
 * USART1 is used to communicate with the board controller (AT32UC3256B1).
 *
 * USART1 configuration: (TXD1)
 * - 57600 baud //used the equation to calculate this...
 * - No parity
 * - 1 stop bit
 * - 8-bit character size
 */
void usart1_init(void)
{
	// Make sure I/O clock to USART1 is enabled
	PRR0 &= ~(1 << PRUSART1);

	// Set baud rate
	UBRR1 = ((float)(F_CPU / (16 * (float)USART_1_BAUD)) - 1);
	
	// Set baud rate to 57.6k at fOSC = 11.0592 MHz
	//UBRR1 = 0x0B;

	// Clear USART Transmit complete flag, normal USART transmission speed
	UCSR1A |= (1 << TXC1) | (0 << U2X1);

	// Enable transmitter
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);

	// Asynchronous mode, no parity, 1 stop bit, character size = 8-bit
	UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10) | (0 << UCPOL1);
}

void usart1_Data (const char *str) //addresses string by its characters' addresses
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		usart1_TX_Data(str[i]); //sends char data to Transmit function
	}
}

void usart1_TX_Data (char Data) //transmit data type char to usb????
{
	while (!(UCSR1A & (1 << UDRE1))) //wait for empty transmit buffer...
	{
		;
	}
	UDR1 = Data; //Puts data into the buffer, sends the data through TX
}

/** \brief Function to setup ADC.
 *
 * ADC configuration:
 * - AVCC voltage with 100nF cap at AREF pin as reference
 * - 172.8kHz ADC frequency
 */
void adc_init(void)
{
	// Make sure I/O clock to the ADC is enabled //don't normally need to do this because this initializes at 0 (enabled)
	PRR0 &= ~(1 << PRADC);

	// Enable ADC, ADC frequency set to 16MHz/128 = 125kHz
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// Digital Input enable
	//DIDR0 = 0x00; //gonna see why we need this on, I'm not trying to use an input
	
	DIDR0 |= (1 << ADC0D); 	//Disable Digital Input; ADC0 pin...

	// Use AVCC with external 100nF cap at AREF pin
	ADMUX |= (0 << REFS1) | (1 << REFS0);
	
	//Enable auto trigger
	ADCSRA |= (1 << ADATE);
	
	//Trigger Sources is Timer/Counter1 Compare Match B
	ADCSRB |= (1 << ADTS2) | (1 << ADTS0);
	
	//enabling ADC Interrupt
	ADCSRA |= (1 << ADIE); //activating the ADC Conversion Complete Interrupt
}

ISR (ADC_vect) //OC1B pin
{
	conv_complete = true;
	TIFR1 |= (1 << OCF1B); //Clear the Output Compare B Match Flag
	
	adc_data = ADC * 0.00468681828097731;
}

/**
 * \brief Function to start ADC conversion.
 *
 * This function will do ADC_NUM_OVERSAMPLINGx oversampling. Per default it
 * will perform 16x oversampling.
 *
 * \param channel    ADC channel number
 *
 * \return adc_data  Result from ADC reading of selected ADC channel
 */
/*
void read_adc(adc_data)
{
	for (int i = 0 ; i < 16 ; i++) {
		//Start single conversion
		ADCSRA |= (1 << ADSC);

		//Wait for conversion complete
		while(!(ADCSRA & (1 << ADIF)));

		//Read ADC conversion result
		adc_data += ADC;
	}
	//takes the average of the 16 data bits...
	adc_data = adc_data/16;

	return adc_data;
}
*/

/** \brief Function to setup timer1 as a time base for the touch lib.
 *
 * In this mode Timer1 is used to generate PWM signal on the LEDs.
 *
 * Timer1 configuration:
 * - CTC mode (clear on compare match B)
 * - clkIO/256 prescaling
 */
void timer1_init(void) //16-bit timer; OC1B pin, Timer 1 and Channel B
{
	// Make sure I/O clock to timer1 is enabled
	PRR0 &= ~(1 << PRTIM1);

	// Enable timer1 output compare match B interrupt ISR
	//TIMSK1 = (1 << OCIE1B); // this is an interrupt bit mask

	// Set timer compare value (how often timer ISR will fire)
	OCR1A = 21599; //set at 1 Hz with a prescalar of 256

	//setting for Toggle OC1B on Compare Match; the rest initializes at 0
	TCCR1A |= (1 << COM1B0); 

	// Timer1 mode = CTC (count up to compare value, then reset); the rest initializes at 0
	TCCR1B |= (1 << WGM12);

	// Timer1 prescaler = system clock / 256
	TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
}

void LED_toggle (void)
{
	PORTB ^= (1 << PORTB3) | (1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0);
}

void LED_init (void)
{
	// (make sure PORTB0 is set as output)
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
	PORTB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
}
