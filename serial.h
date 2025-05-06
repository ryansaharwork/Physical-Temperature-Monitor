#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

// temp high and low limits
#define TEMP_MIN 50
#define TEMP_MAX 90

void serial_init(void);
void transmit_settings(void);
void process_received_settings(void);

extern volatile uint8_t tempLow;
extern volatile uint8_t tempHigh;
extern volatile uint8_t remoteLow;
extern volatile uint8_t remoteHigh;
extern volatile uint8_t remoteSettings;
extern volatile uint8_t useRemote;
extern volatile uint8_t rxData;
extern volatile uint8_t rxBufferIdx;
extern volatile uint8_t rxBuffer[];
extern volatile uint8_t rxDataStarted;

extern void update_leds(int16_t tempFahrenheit);
extern void update_green_led_brightness(int16_t tempFahrenheit);

#endif 