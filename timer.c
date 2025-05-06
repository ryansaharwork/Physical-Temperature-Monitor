/*
*  Name: Ryan Sahar
*  Email: ryansaha@usc.edu
*  Section: Wed 3:30
*  Assignment: EE109 - Temperature Monitor Project - Checkpoint 3
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "project.h"

// sets up a timer to control the brightness of the green led
void timer1_init(void)
{
    // initializing the timer and prescalar
    TCCR1A = (1 << WGM11) | (1 << WGM10);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); 
    
    // timer restarts with this value
    OCR1A = 2500;
    
    // starts the green led at mediums
    OCR1B = OCR1A / 2;
    
    // turning on timer1 interrupts
    TIMSK1 = (1 << OCIE1B) | (1 << TOIE1);
}

// sets up a timer that controls the servo
void timer2_init(void)
{
    // initializing the timer and prescalar
    TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2A1);
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  
    
    // starting position for servo (all the way left)
    OCR2A = SERVO_MIN_PULSE;
    
    // turning on timer2 interrupts
    TIMSK2 = (1 << TOIE2);
}

// changes brightness of the green led
// dependent on the temperature
void update_green_led_brightness(int16_t tempFahrenheit) 
{
    // if temp is in range
    if (ledState == 0) 
    {
    // gets rid of decimal (just a whole #)
        int16_t temp = tempFahrenheit / 10;  
        
        // gets what the user set as low and high limits
        uint8_t currLow, currHigh;
        get_current_settings(&currLow, &currHigh);
        
        // if the low and high limits are the same, make the brightness medium
        if (currHigh == currLow) 
        {
            OCR1B = (OCR1A + (OCR1A / 2)) / 2;  
        } 
        else 
        {
            // the linear equation to determine OCR1B
            // if temp is rly low, led is dim
            // if temp is rly high, led is bright 
            int32_t tempRange = currHigh - currLow;
            int32_t tempPosition = temp - currLow;
            
            // calculate OCR1B value based on the temp
            if (temp <= currLow) 
            {
            // at low threshold, lowest brightness
            OCR1B = OCR1A;  
            }
            else if (temp >= currHigh) 
            {
            OCR1B = OCR1A / 2;  // at high threshold, highest brightness
            }
            else 
            {
            // changes brightenss based on how close we are to high temp
            int32_t ocrRange = OCR1A - (OCR1A / 2); 
            int32_t ocrVal = OCR1A - ((ocrRange * tempPosition) / tempRange);
            OCR1B = (uint16_t)ocrVal;
            }
        }
    } 
    else 
    {
        // temp out of range, turn green led off
        OCR1B = OCR1A; 
    }
}

// runs when timer for green light finishes a cycle
ISR(TIMER1_OVF_vect)
{
    // turn green light off
    PORTC |= (1 << PC3); 
}

// runs when the green light is going to turn on again
ISR(TIMER1_COMPB_vect)
{
    // if temp is in range, turn the green light on
    if (ledState == 0) { 
        PORTC &= ~(1 << PC3);  
    }
}


// used to move servo and runs when the timer overflows
ISR(TIMER2_OVF_vect)
{
    if (servoMoving) 
    {
        servoCounter++;
        
        // since the timer is fast, we slow it down with prescalar
        // after it's slowed down, timer overflows 61 times every second
        // we want servo to move slowly in the 10 seconds
        // move it every second
        // timer overflows 61 times every sec, so we wait for 61 overeflows before moving servo to next step
        if (servoCounter >= 61) 
        {
            servoCounter = 0;
            stepCounter++;
            
            if (stepCounter >= SERVO_STEPS) 
            {
                // after 10 seconds, stop the servo
                servoMoving = 0;
                
                // put a flag to not have the servo restart when the temp stays out of range (stops after 10 secs)
                servoDone = 1;
            } 
            else 
            {
                // thisc calculates the new servo position
                uint8_t range = SERVO_MAX_PULSE - SERVO_MIN_PULSE;
                servoPosition = SERVO_MIN_PULSE + 
                    ((range * stepCounter) / SERVO_STEPS);
                
                // updating the servo position 
                OCR2A = servoPosition;
            }
        }
    }
}