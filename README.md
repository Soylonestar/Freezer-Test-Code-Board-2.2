# Freezer-Test-Code-Board-2.2

The main goal of this code (on 2.2 boards):
	1)Read the Batt_REF of the board and be able to transmit that data through the serial port
 	2)In order to do that:
  		a)Set up Timer1
		b)Set up USART0_TX only to serial port
		c)Set up all necessary pins (look at GIO_init in code)
  		d)Set up ADC on ADC0 pin (Analog to Digital Conversion)
		e)With USART0 read/store/transmit ADC data to serial
  		....

[https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf](url)
