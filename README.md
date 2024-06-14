# Freezer-Test-Code-Board-2.2

The goal of this code is to be able to read data from serial port, once board is inside the freezer. During this time, the board will read BATT_Ref, and then read/store data to transmit to the serial every x amount of times. <br />

[Atmega 2560 Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf) <br/>

[AtMega 2560 Documentation Tables](https://www.microchip.com/en-us/product/ATmega2560#document-table) <br/>

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
Experiencing problems in the ADC data not converting correctly, despite the constant being calculated correctly. This may be due to hardware issues. <br />

It appears to be hardware issue, the original board had an Vin (5.7V) higher than the Vref (5V). Causing problems. <br />
New board has a much lower Vin (1.4) than Vref (5V) when V_batt is 12V. Which is the intended maximum voltage we monitor. This may cause lack of resolution of our data. <br />
Making the question, how to make our Vin closer to 5V when V_batt is 12V? (Do we change the voltage dividers? Do we change the voltage reference to something else? <br />

6/11/2024 <br />
Determined the necessary resistors to make sure that at the high end (18V) the Vref is still under 5V. Those resistors being 2610 ohms and 1000 ohms. <br />
At V_batt 18V, we get 4.98V. Less the V_ref of 5V. <br />
At V_batt 12V, we get 3.32V. Less the V_ref of 5V. <br />
Moving onto how to change the resistors. We've decided to do that on V2.1 boards. So I need to change the code a bit for V2.1 Boards. Code will be clearly shown above. <br />
Update: <br />
In board V2.1, we changed the Divsion Factor to 128, because when divide by to F_CPU, we get 125kHz, within the necessary time for the ADC Conversion. <br />
Another thing is we commented/erased some LED code becasue the RED_LED is overridden by the OC1B Timer so, it'll just blink with the timer. <br />

6/12/2024 <br />
So I changed the resistor values, their approximated values are 2682 ohms and 998 ohms. Which is close. After installing and recalculating constant again, we still have the same problem. But it's closer than before. Almost 90% of the time, within 0.1V. <br/>
Update: <br/>
Raised the voltage to 12.7V, btw. After messing around with Timer Values, delays, and prescalar selections, etc., we still have the same problem. Even hooked up a battery to it, same problem.
Here is a helpful link: <br/>
[ADC Atmega Helpful Link](https://www.tspi.at/2021/07/15/atmegaavradc.html) <br/>
This is another ADC tutorial link, may help: <br/>
[Electronics Wings Tutorials](https://www.electronicwings.com/avr-atmega/getting-started-with-atmel-studio) <br/>
IMPORTANT UPDATE: <br/>
Andy stated that I should change back to V2.2 boards. So back to that code... <br/>

6/14/2024 <br/>
Here is the internal and external impedance of the ADC pin...<br/>
<img src="https://github.com/Soylonestar/Freezer-Test-Code-Board-2.2/assets/53907747/839425e8-c842-4e3e-a3de-0f538b5a1019" width="525" /> <br/>
Update: <br/>
Forbes and I are gonna try programming a professionally done board with a ATMEGA 1284P.<br/>
But before that, we are checking the overall noise of our V2.2 board. We got a ramping noise effect going on, with a Vpp of 0.1V. Not only that but there is a huge amount of ground noise, despite everything off. <br/>
[Atmega 1284P Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATmega164A_PA-324A_PA-644A_PA-1284_P_Data-Sheet-40002070B.pdf) <br/>
[Mega-1284P Board Schematics](https://ww1.microchip.com/downloads/en/Appnotes/doc8377.pdf) <br/>
