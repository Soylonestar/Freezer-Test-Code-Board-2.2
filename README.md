# Freezer-Test-Code-Board-2.2

The goal of this code is to be able to read data from serial port, once board is inside the freezer. During this time, the board will read BATT_Ref, and then read/store data to transmit to the serial every x amount of times.

## Steps and Procedures

1) Read the Batt_REF of the board and be able to transmit that data through the serial port <br /> 
2) In order to do that: <br />
>  	a) Set up Timer1 <br />
>	b) Set up USART0_TX only to serial port <br />
>	c) Set up all necessary pins (look at GIO_init in code) <br />
> 	d) Set up ADC on ADC0 pin (Analog to Digital Conversion) <br />
>	e) With USART0 read/store/transmit ADC data to serial <br />
>  	.... <br />

## Troubleshooting

6/10/2024 <br />
Experiencing problems in the ADC data not converting correctly, despite the constant being calculated correctly.
This may be due to hardware issues.
