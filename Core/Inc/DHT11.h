/*
 * DHT11.h
 *
 *  Created on: Jul 16, 2025
 *      Author: RASBI
 *
 */

#ifndef SRC_DHT11_H_
#define SRC_DHT11_H_

#include "stm32f1xx_hal.h"
#define DHT11_PINMODE_OUTPUT 0
#define DHT11_PINMODE_INPUT 1

typedef struct {
	GPIO_TypeDef* GPIO_Channel;
	uint16_t pin;
	TIM_HandleTypeDef* htim;

} DHT11_t;

void set_pin_mode(DHT11_t* dht11, uint8_t pinmode);
void init_DHT11(DHT11_t* dht11); //User-defined structure values are used for initialization.
uint8_t read_DHT11(DHT11_t* dht11, uint8_t* temp_i, uint8_t* temp_d, uint8_t* humidity_i, uint8_t* humidity_d);

#endif /* SRC_DHT11_H_ */
