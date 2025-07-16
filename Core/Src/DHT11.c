/*
 * DHT11.c
 *
 *  Created on: Jul 16, 2025
 *      Author: RASBI
 */


#include "dht11.h"
#define COUNTER_MAX 65535

//simple delay_us
void delay_us(DHT11_t* dht11, uint16_t time) {
	__HAL_TIM_SET_COUNTER(dht11->htim,0);

	if (time >= COUNTER_MAX) {
		time = COUNTER_MAX - 1;
	}

	while ((uint16_t)__HAL_TIM_GET_COUNTER(dht11->htim) < time);
}

//set gpio pin mode
void set_pin_mode(DHT11_t* dht11, uint8_t pinmode) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = dht11->pin;

	if (pinmode == DHT11_PINMODE_OUTPUT) {
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	} else if (pinmode == DHT11_PINMODE_INPUT) {
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
	}

	HAL_GPIO_Init(dht11->GPIO_Channel, &GPIO_InitStruct);
}

//init DHT11
void init_DHT11(DHT11_t* dht11) { //User-defined structure values are used for initialization.

	//#0 start timer
	HAL_TIM_Base_Start(dht11->htim);
}

uint8_t read_DHT11(DHT11_t* dht11, uint8_t* temp_i, uint8_t* temp_d, uint8_t* humidity_i, uint8_t* humidity_d, ) {

	// #0 variable
	uint8_t status = 0; //status for check dht11 response
	uint8_t data[5] = {0};

	// #1 Ask to DHT11
	__disable_irq();
	set_pin_mode(dht11, DHT11_PINMODE_OUTPUT);
	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_SET);
	delay_us(dht11, 10000); //wait 10ms

	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_RESET);
	delay_us(dht11, 18000);

	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_SET);
	delay_us(dht11, 30);
	set_pin_mode(dht11, DHT11_PINMODE_INPUT);

	// #2 Check response - HIGH => 80us LOW => 80us HIGH => LOW, start send data
	delay_us(dht11, 1);
	// #2-1: Does signal changed to LOW?
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 50) {
	    	status = 1; // error code 1: fail to initializing
	    	goto ERROR_HANDLE;
	    }
	}

	// #2-2: Dose signal changed to HIGH?
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(!HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 90) {
	    	status = 2; // error code 2: fail to initializing
	    	goto ERROR_HANDLE;
	    }
	}

	// #2-3: Dose signal changed to LOW?
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 90) {
	        status = 3; // error code 3: high to low fail
	        goto ERROR_HANDLE;
	    }
	}

	// #3 read data
	for (int i = 0; i<sizeof(data)/sizeof(data[0]); i++) {
		for (int j = 0; j <8; j++) {
			//Does signal changed LOW to HIGH?
			__HAL_TIM_SET_COUNTER(dht11->htim,0);
			while(!HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
				if (__HAL_TIM_GET_COUNTER(dht11->htim)>60) {
					status = 4; //error code 4: data send fail
					goto ERROR_HANDLE;
				}
			}
			//measure HIGH pulse
			__HAL_TIM_SET_COUNTER(dht11->htim,0);
			while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
				if (__HAL_TIM_GET_COUNTER(dht11->htim)>90) {
					status = 5; //error code 5: data send fail
					goto ERROR_HANDLE;
				}
			}
			//write data
			if (__HAL_TIM_GET_COUNTER(dht11->htim)>40) {
				data[i] |= (1<<(7-j));
			}
		}
	}
	// check check_sum :)
	if (data[4] != (uint8_t)(data[0]+data[1]+data[2]+data[3])) {
		status = 6; ////error code 6: check sum error!
		goto ERROR_HANDLE;
	}

	// #4 return data
	*temp_i = data[0];
	*temp_d = data[1];
	*humidity_i = data[2];
	*humidity_d = data[3];

	ERROR_HANDLE:
	__enable_irq();
	return status;

}
