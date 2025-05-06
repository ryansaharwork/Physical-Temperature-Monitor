#ifndef ENCODER_H
#define ENCODER_H

#include <avr/io.h>

void encoder_init(void);

extern volatile int8_t encouderCnt;
extern volatile uint8_t changed;

#endif 