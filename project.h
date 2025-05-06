#ifndef PROJECT_H
#define PROJECT_H

#include <avr/io.h>

#define TEMP_MIN 50
#define TEMP_MAX 90

void update_display(int16_t temp_fahrenheit);
void update_temp_display(int16_t temp_fahrenheit);
void update_low_display(void);
void update_high_display(void);
void save_settings_to_eeprom(void);
void load_settings_from_eeprom(void);
void get_current_settings(uint8_t *low, uint8_t *high);
void update_leds(int16_t temp_fahrenheit);
int16_t convert_temp_to_fahrenheit(int16_t temp_celsius);

extern volatile uint8_t tempLow;
extern volatile uint8_t tempHigh;
extern volatile uint8_t adjustHigh;
extern volatile uint8_t buttonPressed;
extern volatile uint8_t displayUpdating;
extern volatile uint8_t ledState;
extern volatile int8_t encouderCnt;
extern volatile uint8_t changed;
extern volatile uint8_t servoMoving;
extern volatile uint8_t servoDone;
extern volatile uint16_t servoCounter;
extern volatile uint8_t servoPosition;
extern volatile uint8_t stepCounter;
extern volatile uint8_t sendButton;
extern volatile uint8_t localRemote;
extern volatile uint8_t useRemote;
extern volatile uint8_t remoteSettings;
extern volatile uint8_t remoteLow;
extern volatile uint8_t remoteHigh;
extern volatile uint8_t rx_buffer[];
extern volatile uint8_t rxDataStarted;
extern volatile uint8_t rxBufferIdx;
extern volatile uint8_t rxData;

#endif 