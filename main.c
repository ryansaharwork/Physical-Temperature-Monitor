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
 #include <avr/eeprom.h>
 #include "lcd.h"
 #include "ds18b20.h"
 #include "encoder.h"
 #include "timer.h"  
 #include "serial.h"
 #include "project.h"
 
 // eeprom addresses for storing temp thresh
 #define EEPROM_LOW_ADDR  100
 #define EEPROM_HIGH_ADDR 101
 
 // buffer size for serial comms
 #define BUFFER_SIZE 5
 
 volatile uint8_t tempLow = 70;      // default low temp threshold
 volatile uint8_t tempHigh = 75;     // default high temp threshold
 volatile uint8_t adjustHigh = 0;   
 volatile uint8_t buttonPressed = 0; // button flag for debounce
 volatile uint8_t displayUpdating = 0; // flag - stop display issues
 
 // variables for servo control
 volatile uint16_t servoCounter = 0;    // counter for servo movement timer
 volatile uint8_t servoMoving = 0;      // flag - if servo is active
 volatile uint8_t servoPosition = 0;    // curr position of servo
 volatile uint8_t stepCounter = 0; // counter for servo steps
 volatile uint8_t servoDone = 0;    // flag - shows completion of full rotation
 
 // for rotary encoder
 volatile uint8_t changed = 0;  // flag - state change
 volatile int8_t encouderCnt = 0;
 
 // led state tracking
 volatile uint8_t ledState = 0; 
 
 // serial commus variables
 volatile uint8_t sendButton = 0;   // flag - send button
 volatile uint8_t localRemote = 0;  // flag - local or remote button
 volatile uint8_t useRemote = 0;   // flag - use remote settings
 volatile uint8_t remoteSettings = 0; // flag - valid remote settings
 volatile uint8_t remoteLow = 0;       // remote low temp setting
 volatile uint8_t remoteHigh = 0;      // remote high temp setting
 
 // serial receiver variables
 volatile uint8_t rxBuffer[BUFFER_SIZE];    // buffer for received data
 volatile uint8_t rxDataStarted = 0;       // flag indicating start of data
 volatile uint8_t rxBufferIdx = 0;       // index into receive buffer
 volatile uint8_t rxData = 0;         // flag indicating valid data received
 
 // convert raw temp data from DS18B20 to fahrenheit
 int16_t convert_temp_to_fahrenheit(int16_t tempCelsius) 
 {
     int32_t temp = tempCelsius;
     
     // First convert tofahrneit 
     temp = ((temp * 9) / 5) + (32 * 16);
     
     // display with one decimal place
     temp = (temp * 10) / 16;
     
     return (int16_t)temp;
 }
 
 // updates the whole display
 void update_display(int16_t tempFahrenheit) 
 {
     displayUpdating = 1; 
     
     char buf[17];
     
     lcd_writecommand(1);
     
     // shows the local settings on 1st line
     lcd_moveto(0, 0);
     
     // checks if local settings are active
     uint8_t localChosen = !useRemote || !remoteSettings;
     
     // just changes the format regarding the ">" carrot
     if (adjustHigh) 
     {
         lcd_stringout(" L=");
         snprintf(buf, sizeof(buf), "%02d", tempLow);
         lcd_stringout(buf);
         
         lcd_stringout(">H=");
         snprintf(buf, sizeof(buf), "%02d", tempHigh);
         lcd_stringout(buf);
         
         if (localChosen) 
         {
             lcd_stringout("<"); 
         } 
         else 
         {
             lcd_stringout(" "); \
         }
     } 
     else 
     {
         lcd_stringout(">L=");
         snprintf(buf, sizeof(buf), "%02d", tempLow);
         lcd_stringout(buf);
         
         lcd_stringout(" H=");
         snprintf(buf, sizeof(buf), "%02d", tempHigh);
         lcd_stringout(buf);
         
         // show < for local settings, none for remote
         if (localChosen) 
         {
             lcd_stringout("<");  
         } 
         else 
         {
             lcd_stringout(" ");  // Space when remote settings are active
         }
     }
     
     // displays curr temp
     snprintf(buf, sizeof(buf), " %2d.%1d", 
             tempFahrenheit / 10, tempFahrenheit % 10);
     lcd_stringout(buf);
     
     // shows remote settings on 2nd line
     if (remoteSettings) 
     {
         lcd_moveto(1, 0);
         
         if (useRemote && remoteSettings) 
         {
             lcd_stringout(" L=");
             snprintf(buf, sizeof(buf), "%02d", remoteLow);
             lcd_stringout(buf);
             lcd_stringout(" H=");
             snprintf(buf, sizeof(buf), "%02d<", remoteHigh);
             lcd_stringout(buf);
             lcd_stringout("     ");
         } 
         else 
         {
             lcd_stringout(" L=");
             snprintf(buf, sizeof(buf), "%02d", remoteLow);
             lcd_stringout(buf);
             lcd_stringout(" H=");
             snprintf(buf, sizeof(buf), "%02d", remoteHigh);
             lcd_stringout(buf);
             lcd_stringout("      ");
         }
     }
     
     displayUpdating = 0; 
 }
 
 // update temp part of display
 void update_temp_display(int16_t tempFahrenheit) 
 {
     // if display is being updated somewhere else, skip
     if (displayUpdating) return;
     
     displayUpdating = 1;
     
     char buf[8];
     
     lcd_moveto(0, 11); 
     snprintf(buf, sizeof(buf), " %2d.%1d", 
              tempFahrenheit / 10, tempFahrenheit % 10);
     lcd_stringout(buf);
     
     displayUpdating = 0;
 }
 
 // updates the low temp 
 void update_low_display() 
 {
     if (displayUpdating) return;
     
     // flag to stop other things from running
     displayUpdating = 1;
     
     char buf[3];
     uint8_t currLow;
     
     // shows the local or remote settings
     if (useRemote && remoteSettings) 
     {
         currLow = remoteLow;
     } 
     else 
     {
         currLow = tempLow;
     }
          
     lcd_moveto(0, 3);
     snprintf(buf, sizeof(buf), "%02d", currLow);
     lcd_stringout(buf);
     
     displayUpdating = 0;
 }
 
 // updates the high temp setting
 void update_high_display() 
 {
     // skips if dipslay is being updated somewhere else
     if (displayUpdating) return;
     
     // sets flag to prevent other updates (prevent weird display issues)
     displayUpdating = 1;
     
     char buf[3];
     uint8_t currHigh;
     
     // shows the active setting (local or remote)
     if (useRemote && remoteSettings) 
     {
         currHigh = remoteHigh;
     } 
     else 
     {
         currHigh = tempHigh;
     }
     
     lcd_moveto(0, 8);
     snprintf(buf, sizeof(buf), "%02d", currHigh);
     lcd_stringout(buf);
     
     displayUpdating = 0;
 }
 
 // save temp settings from eeprom
 void save_settings_to_eeprom() 
 {
     eeprom_update_byte((void *)EEPROM_LOW_ADDR, tempLow);
     eeprom_update_byte((void *)EEPROM_HIGH_ADDR, tempHigh);
 }
 
 //load temp settings from eeprom
 void load_settings_from_eeprom() 
 {
     uint8_t low_temp = eeprom_read_byte((void *)EEPROM_LOW_ADDR);
     uint8_t high_temp = eeprom_read_byte((void *)EEPROM_HIGH_ADDR);
     
     if (low_temp >= TEMP_MIN && low_temp <= TEMP_MAX && 
         high_temp >= TEMP_MIN && high_temp <= TEMP_MAX && 
         low_temp <= high_temp) 
     {
         tempLow = low_temp;
         tempHigh = high_temp;
     }
 }
 
 // grabs the low and high temp settings
 void get_current_settings(uint8_t *low, uint8_t *high) 
 {
     if (useRemote && remoteSettings) 
     {
         *low = remoteLow;
         *high = remoteHigh;
     } 
     else 
     {
         *low = tempLow;
         *high = tempHigh;
     }
 }
 
 // update the leds regarding temp
 void update_leds(int16_t temp_fahrenheit) 
 {
     int16_t temp = temp_fahrenheit / 10;
     
     uint8_t current_low, current_high;
     get_current_settings(&current_low, &current_high);
     
     PORTC |= (1 << PC2) | (1 << PC3) | (1 << PC4);
     
     // check temps with thresholds
     if (temp < current_low) 
     {
         // too cold, turn blue
         PORTC &= ~(1 << PC2);  
         ledState = 1;
         
         // start servo countdown
         if (!servoMoving && !servoDone) 
         {
             servoMoving = 1;
             servoPosition = SERVO_MIN_PULSE;
             OCR2A = servoPosition;
             servoCounter = 0;
             stepCounter = 0;
         }
     } 
     else if (temp > current_high) 
     {
         // too hot, turn red
         PORTC &= ~(1 << PC4);  
         ledState = 2;
         
         // start servo countdown
         if (!servoMoving && !servoDone) 
         {
             servoMoving = 1;
             servoPosition = SERVO_MIN_PULSE;
             OCR2A = servoPosition;
             servoCounter = 0;
             stepCounter = 0;
         }
     } 
     else 
     {
         // tmep in range
         ledState = 0;
         
         // if servo was active, stop it
         if (servoMoving) 
         {
             servoMoving = 0;
         }
         
         // reset servo flag when back in correct range
         servoDone = 0;
         
         // timer1 interrupts handles green led brightness
     }
 }
 
 // isr for high/low button
 ISR(PCINT0_vect) 
 {
     // debounce
     _delay_us(1000);
     
     if ((PINB & (1 << PB5)) == 0) 
     {
         // toggle only if the button isn't pressed yet
         if (!buttonPressed) 
         {
             buttonPressed = 1;
             
             // if we are using remote settings, don't do anything
             if ((useRemote && remoteSettings) || displayUpdating) 
                 return;
             
             displayUpdating = 1;
             
             // swithces from low or high settings
             adjustHigh = !adjustHigh;
             
             // updates the indicators (the carrots <>)
             if (adjustHigh) 
             {
                 lcd_moveto(0, 0);
                 lcd_stringout(" "); 
                 lcd_moveto(0, 5);
                 lcd_stringout(">");  
             } 
             else 
             {
                 lcd_moveto(0, 0);
                 lcd_stringout(">"); 
                 lcd_moveto(0, 5);
                 lcd_stringout(" "); 
             }
             
             displayUpdating = 0;
         }
     } 
     else 
     {
         buttonPressed = 0;
     }
 }
 
 // isr for send and local/remote button 
 ISR(PCINT1_vect)
 {
     static uint8_t pc0_last_state = 1;  
     static uint8_t pc5_last_state = 1;  
     
     // debounce
     _delay_ms(20); 
     
     // reads curr button presses
     uint8_t pc0_current_state = (PINC & (1 << PC0)) > 0;
     uint8_t pc5_current_state = (PINC & (1 << PC5)) > 0;
     
     // checks the send butotn
     if (!pc0_current_state && pc0_last_state) 
     {
         sendButton = 1;
     }
     
     // checks the local/remote button
     if (!pc5_current_state && pc5_last_state) 
     {
         localRemote = 1;
     }
     
     // update last states
     pc0_last_state = pc0_current_state;
     pc5_last_state = pc5_current_state;
 }
 
 int main(void) 
 {
     uint8_t tempData[2];      // array that hold temp data from our sensor
     int16_t tempCelsius;      // vemp value in celsius
     int16_t tempFahrenheit;   // temp value in fahrenheit 
     
     // initialize LCD
     lcd_init();
     
     // sets the high and low button as input with pull-up
     DDRB &= ~(1 << PB5);     
     PORTB |= (1 << PB5);     
     
     // sets the local and remote button as input with pull-up
     DDRC &= ~(1 << PC5);     
     PORTC |= (1 << PC5);    
     
     // sets tgb led outputs - PC2 is blue, PC3 is green, PC4 is red)
     DDRC |= (1 << PC2) | (1 << PC3) | (1 << PC4);  
     PORTC |= (1 << PC2) | (1 << PC3) | (1 << PC4); 
     
     // sets the servo as an output 
     DDRB |= (1 << PB3);    
     PORTB &= ~(1 << PB3);    
     
     // sets the pin change interrupts for button 
     PCICR |= (1 << PCIE0);    
     PCMSK0 |= (1 << PCINT5);  
     
     // sets teh pin change interrupt for the local and remote button and SEND button
     PCICR |= (1 << PCIE1);    
     PCMSK1 |= (1 << PCINT13) | (1 << PCINT8);  
     
     // initialize rotary encoder
     encoder_init();
     
     // initialize timers
     timer1_init();
     timer2_init();
     
     // initialize UART
     serial_init();
     
     // turn on global interrupts
     sei();
     
     // displays splash screen
     lcd_writecommand(1);  
     lcd_moveto(0, 0);
     lcd_stringout("Temp Monitor");
     lcd_moveto(1, 0);
     lcd_stringout("Ryan Sahar");
     _delay_ms(2000);
     lcd_writecommand(1);
     
     // load settings from our eeprom
     load_settings_from_eeprom();
     
     // check if DS18B20 initialized correctly
     if (ds_init() == 0) 
     {
         lcd_writecommand(1); 
         lcd_moveto(0, 0);
         lcd_stringout("DS18B20 Error!");
         while (1) { } 
     }
     
     // start the first temp conversion
     ds_convert();
     
     // initialize temp to a default value until we get our first reading
     tempFahrenheit = 750;  
     
     // initial display setup
     update_display(tempFahrenheit);
     
     // led setup
     update_leds(tempFahrenheit);
     update_green_led_brightness(tempFahrenheit);
     
     // main loop
     while (1) 
     {
         // gets temp readings first
         if (ds_temp(tempData)) 
         {
             // convert the temp data to a 16-bit signed value
             tempCelsius = ((int16_t)tempData[1] << 8) | tempData[0];
             
             // converts celsius to fahrenheit
             tempFahrenheit = convert_temp_to_fahrenheit(tempCelsius);
             
             // update just the temp part of the display
             update_temp_display(tempFahrenheit);
             
             // update the leds based on the temp
             update_leds(tempFahrenheit);
             update_green_led_brightness(tempFahrenheit);
             
             // start the next temp conversion
             ds_convert();
         }
         
         // checks if send button is pressed
         if (sendButton) 
         {
             sendButton = 0;
             
             // sends the temp settings
             transmit_settings();
         }
         
         // checks if the local remote button was pressed
         if (localRemote) 
         {
             // flag for button presses
             localRemote = 0;
             
             // if the remote settings are valid, toggle
             if (remoteSettings) 
             {
                 // select the active settings
                 useRemote = !useRemote;
                 
                 update_display(tempFahrenheit);
                 
                 // update led and servo based on new settings
                 update_leds(tempFahrenheit);
                 update_green_led_brightness(tempFahrenheit);
             }
         }
         
         // checks if data has been received
         if (rxData) 
         {
             rxData = 0;
             process_received_settings();
             update_display(tempFahrenheit);
         }
         
         // processes the encoder changes 
         if (changed) 
         {
             changed = 0;
             
             // adjusts local settings (not remote)
             if (!useRemote) 
             {
                 // updates the high temp setting
                 if (adjustHigh) 
                 {
                     if (encouderCnt > 0 && tempHigh < TEMP_MAX) 
                     {
                         tempHigh++;
                         save_settings_to_eeprom();
                     } 
                     else if (encouderCnt < 0 && tempHigh > tempLow) 
                     {
                         tempHigh--;
                         save_settings_to_eeprom();
                     }
                     
                     update_high_display();
                 } 
                 else 
                 {
                     // updates the low temp setting
                     if (encouderCnt > 0 && tempLow < tempHigh) 
                     {
                         tempLow++;
                         save_settings_to_eeprom();
                     } 
                     else if (encouderCnt < 0 && tempLow > TEMP_MIN) 
                     {
                         tempLow--;
                         save_settings_to_eeprom();
                     }
                     
                     update_low_display();
                 }
                 
                 encouderCnt = 0;
                 
                 // updates led based on settings
                 update_leds(tempFahrenheit);
                 update_green_led_brightness(tempFahrenheit);
             }
             else 
             {
                 encouderCnt = 0;
             }
             
             // delay to prevent weird display issues
             _delay_ms(5);
         }
     }
     
     return 0; 
 }