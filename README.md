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
Experiencing problems in the ADC data not converting correctly, despite the constant being calculated correctly. This may be due to hardware issues. <br \>

It appears to be hardware issue, the original board had an Vin (5.7V) higher than the Vref (5V). Causing problems. <br \>
New board has a much lower Vin (1.4) than Vref (5V) when V_batt is 12V. Which is the intended maximum voltage we monitor. This may cause lack of resolution of our data. <br \>
Making the question, how to make our Vin closer to 5V when V_batt is 12V? (Do we change the voltage dividers? Do we change the voltage reference to something else? <br \>

6/11/2024 <br />
Determined the necessary resistors to make sure that at the high end (18V) the Vref is still under 5V. Those resistors being 2610 ohms and 1000 ohms. <br \>
At V_batt 18V, we get 4.98V. Less the V_ref of 5V. <br \>
At V_batt 12V, we get 3.32V. Less the V_ref of 5V. <br \>
Moving onto how to change the resistors. We've decided to do that on V2.1 boards. So I need to change the code a bit for V2.1 Boards. Code will be clearly shown above.
