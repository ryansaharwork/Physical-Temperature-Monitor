/*
 *  Name: Ryan Sahar
 *  Email: ryansaha@usc.edu
 *  Section: Wed 3:30
 *  Assignment: EE109 - Temperature Monitor Project - Checkpoint 3
 */

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <util/delay.h>
 #include <stdio.h>
 #include "serial.h"
 #include "project.h"
 
 // given serial comms constants from slides
 #define FOSC 16000000
 #define BAUD 9600    
 #define MYUBRR (FOSC/16/BAUD-1)
 
 // our buffer size for serial comms
 #define BUFFER_SIZE 5
 
 // sets up serial so we can send or receive data
 void serial_init(void)
 {
     UBRR0 = MYUBRR;                         
     UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
     UCSR0C = (3 << UCSZ00);                
     
     // Enable the chip for serial data to reach board
     DDRB |= (1 << PB4);      
     PORTB &= ~(1 << PB4);    
 }
 
 // sends curr temp settings over serial
 void transmit_settings(void) 
 {     
     // wait for serial then sends start symbol
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '<';
     
     // sends low temp digits (2 digits)
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '0' + (tempLow / 10);
     
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '0' + (tempLow % 10);
     
     // sends high temp digits (2 digits)
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '0' + (tempHigh / 10);
     
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '0' + (tempHigh % 10);
     
     // end character
     while ((UCSR0A & (1 << UDRE0)) == 0) { }
     UDR0 = '>';
 }
 
 // gets remote settings
 void process_received_settings(void) 
 {
     // null terminate to treat it like a string
     rxBuffer[rxBufferIdx] = '\0';
     
     // if we didn't get exactly 4 digits, just ignore it
     if (rxBufferIdx != 4) 
     {
         rxData = 0; 
         return;
     }
     
     // turn the 4 characters into 2 #'s
     uint8_t receivedLow = (rxBuffer[0] - '0') * 10 + (rxBuffer[1] - '0');
     uint8_t receivedHigh = (rxBuffer[2] - '0') * 10 + (rxBuffer[3] - '0');
     
     // make sure the values are valid
     if (receivedLow >= TEMP_MIN && receivedLow <= TEMP_MAX && 
         receivedHigh >= TEMP_MIN && receivedHigh <= TEMP_MAX && 
         receivedLow <= receivedHigh) 
     {
         remoteLow = receivedLow;
         remoteHigh = receivedHigh;
         remoteSettings = 1;
         
         // using remote setting, refresh the led
         if (useRemote) 
         {
            // dummy value, placeholder
             int16_t tempFahrenheit = 750; 
             // real temp value is from main loop
             update_leds(tempFahrenheit);
             update_green_led_brightness(tempFahrenheit);
         }
     } 
     else 
     {
         // bad data
         rxData = 0;
     }
 }
 
 // ISR for USART receive complete
 ISR(USART_RX_vect)
 {
     // get the received character
     char c = UDR0;  
     
     // looking for start char
     if (c == '<') 
     {
         // starting a new data packet
         rxDataStarted = 1;
         rxBufferIdx = 0;
         return;
     }
     
     // looking for end char
     if (c == '>') 
     {
         // end of data packet
         if (rxDataStarted && rxBufferIdx > 0) 
         {
            // marks that we have valid data
             rxData = 1; 
         }
         // resetting for next packet
         rxDataStarted = 0; 
         return;
     }
     
     // stores only numbers
     if (rxDataStarted && rxBufferIdx < BUFFER_SIZE - 1) 
     {
         // making sure we only accept the values between 0-9 (inclusive)
         if (c >= '0' && c <= '9') 
         {
             rxBuffer[rxBufferIdx++] = c;
         } 
         else 
         {
             // invalid character, stops the packet
             rxDataStarted = 0;
         }
     } 
     else if (rxBufferIdx >= BUFFER_SIZE - 1) 
     {
         // too many characters, stops the packet
         rxDataStarted = 0;
     }
 }