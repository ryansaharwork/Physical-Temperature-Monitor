/*
*  Name: Ryan Sahar
*  Email: ryansaha@usc.edu
*  Section: Wed 3:30
*  Assignment: EE109 - Temperature Monitor Project - Checkpoint 3
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "encoder.h"
#include "project.h"

// states for rotary encoder
#define R_START     0
#define R_CW_1      1
#define R_CW_2      2
#define R_CW_3      3
#define R_CCW_1     4
#define R_CCW_2     5
#define R_CCW_3     6

// encoder variables
static volatile uint8_t newState, oldState;
static volatile uint8_t a, b;

// initializes rotary encoder
void encoder_init(void)
{
    //setting inputs
    DDRD &= ~((1 << PD2) | (1 << PD3));
    // enabling pull ups
    PORTD |= (1 << PD2) | (1 << PD3); 
    
    // turning on PCINT2 interrupts
    PCICR |= (1 << PCIE2);
    // turning on PD2 and PD3 interrupts
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19); 
    
    // initializing encoder state
    // reading curr input from port D
    uint8_t input = PIND;
    
    // stores state of pd2 and pd3
    uint8_t pd2;
    uint8_t pd3;

    // checks if pd2 is high or low
    if (input & (1 << PD2)) 
    {
        pd2 = 1;
    } 
    else 
    {
        pd2 = 0;
    }

    // checks if pd3 is high or low
    if (input & (1 << PD3)) 
    {
        pd3 = 1;
    } 
    else 
    {
        pd3 = 0;
    }
     
    // figures out the states based on the values
    if (b == 0 && a == 0) 
    {
        oldState = 0;
    } 
    else if (b == 0 && a == 1) 
    {
        oldState = 1;
    } 
    else if (b == 1 && a == 0) 
    {
        oldState = 2;
    } 
    else if (b == 1 && a == 1) 
    {
        oldState = 3;
    }

    // used for a later comparison
    newState = oldState;
}
 
// isr for rotary encoder for pd2 and pd3
ISR(PCINT2_vect) 
{
    // debounce
    _delay_us(1000);
    
    // reading curr input from port D
    uint8_t input = PIND;
    
    // checks the state of pd2 and pd3
    uint8_t pd2 = input & (1 << PD2);
    uint8_t pd3 = input & (1 << PD3);

    // if pd2 is high, make a = 1, else a = 0
    if (pd2) 
    {
        a = 1;
    } 
    else 
    {
        a = 0;
    }

    // if pd3 is high, set b = 1, else b = 0
    if (pd3) 
    {
        b = 1;
    } 
    else 
    {
        b = 0;
    }

    // determines rotoation direction thru old state
    if (oldState == 0) 
    {
        if (!b && a) 
        {
            newState = 1;
            encouderCnt--;
        } 
        else if (b && !a) 
        {
            newState = 2;
            encouderCnt++;
        }
    }
    else if (oldState == 1) 
    {
        if (b && a) 
        {
            newState = 3;
            encouderCnt--;
        } 
        else if (!b && !a) 
        {
            newState = 0;
            encouderCnt++;
        }
    }
    else if (oldState == 2) 
    {
        if (b && a) 
        {
            newState = 3;
            encouderCnt++;
        } 
        else if (!b && !a) 
        {
            newState = 0;
            encouderCnt--;
        }
    }
    else 
    {
        if (!b && a) 
        {
            newState = 1;
            encouderCnt++;
        } 
        else if (b && !a) 
        {
            newState = 2;
            encouderCnt--;
        }
    }
    
    // if state hasn't been changed, just flag and update
    if (newState != oldState) 
    {
        changed = 1;
        oldState = newState;
    }
}