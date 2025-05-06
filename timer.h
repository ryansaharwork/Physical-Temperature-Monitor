#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>

// pwm constants for the servo
#define SERVO_MIN_PULSE  12   // full left
#define SERVO_MAX_PULSE  35   // full right
#define SERVO_STEPS      10   // how long servo ticks for

void timer1_init(void);
void timer2_init(void);
void update_green_led_brightness(int16_t tempFahrenheit);

extern volatile uint8_t ledState;
extern volatile uint16_t servoCounter;
extern volatile uint8_t servoMoving;
extern volatile uint8_t servoPosition;
extern volatile uint8_t stepCounter;
extern volatile uint8_t servoDone;

extern void get_current_settings(uint8_t *low, uint8_t *high);

#endif 