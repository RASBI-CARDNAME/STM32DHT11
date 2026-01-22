/*
 * DHT11.c
 *
 *  Created on: Jul 16, 2025
 *      Author: RASBI
 */

/**
 * @brief Reads temperature and humidity from DHT11 sensor.
 * @param dht11: pointer to DHT11_t structure
 * @param temp_i: pointer to integer part of temperature
 * @param temp_d: pointer to decimal part of temperature
 * @param humidity_i: pointer to integer part of humidity
 * @param humidity_d: pointer to decimal part of humidity
 * @retval status code
 *     0: OK
 *     1: No LOW response from sensor
 *     2: No HIGH after LOW
 *     3: No LOW after HIGH
 *     4: Data bit start LOW timeout
 *     5: Data bit HIGH too long
 *     6: Checksum error
 */

#include "dht11.h"

//타이머를 이용한 마이크로초 딜레이 함수
void delay_us(DHT11_t* dht11, uint16_t time) {
	__HAL_TIM_SET_COUNTER(dht11->htim,0);
	while ((uint16_t)__HAL_TIM_GET_COUNTER(dht11->htim) < time);
}

//gpio 핀 초기 설정
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

//DHT 11 초기화
void init_DHT11(DHT11_t* dht11) { //User-defined structure values are used for initialization.

	//#0 타이머 시작
	HAL_TIM_Base_Start(dht11->htim);
}

//DHT 11로 부터 정보 읽기
uint8_t read_DHT11(DHT11_t* dht11, uint8_t* temp_i, uint8_t* temp_d, uint8_t* humidity_i, uint8_t* humidity_d) {

	// #0 변수
	uint8_t status = 0; //status for check dht11 response
	uint8_t data[5] = {0};

	// #1 DHT 11 센서 깨우기
	__disable_irq();
	set_pin_mode(dht11, DHT11_PINMODE_OUTPUT);
	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_SET);
	delay_us(dht11, 10000); //wait 10ms

	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_RESET);
	delay_us(dht11, 18000);

	HAL_GPIO_WritePin(dht11->GPIO_Channel, dht11->pin, GPIO_PIN_SET);
	delay_us(dht11, 30);
	set_pin_mode(dht11, DHT11_PINMODE_INPUT);

	// #2 응답 확인 - HIGH => 80us / LOW => 80us / HIGH => LOW, 데이터 전송 시작
	delay_us(dht11, 1);
	// #2-1: 신호가 LOW로 떨어지나 확인
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 50) {
	    	status = 1; // error code 1: fail to initializing
	    	goto ERROR_HANDLE;
	    }
	}

	// #2-2: 신호가 다시 HIGH로 올라오나 확인
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(!HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 90) {
	    	status = 2; // error code 2: fail to initializing
	    	goto ERROR_HANDLE;
	    }
	}

	// #2-3: 신호가 다시 LOW로 떨어지나 확인
	__HAL_TIM_SET_COUNTER(dht11->htim, 0);
	while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
	    if (__HAL_TIM_GET_COUNTER(dht11->htim) > 90) {
	        status = 3; // error code 3: high to low fail
	        goto ERROR_HANDLE;
	    }
	}

	// #3 데이터 읽기
	for (int i = 0; i<sizeof(data)/sizeof(data[0]); i++) {
		for (int j = 0; j <8; j++) {
			//신호가 LOW -> HIGH로 되었는지 확인
			__HAL_TIM_SET_COUNTER(dht11->htim,0);
			while(!HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
				if (__HAL_TIM_GET_COUNTER(dht11->htim)>60) {
					status = 4; //error code 4: data send fail
					goto ERROR_HANDLE;
				}
			}
			//HIGH 펄스 길이 측정
			__HAL_TIM_SET_COUNTER(dht11->htim,0);
			while(HAL_GPIO_ReadPin(dht11->GPIO_Channel, dht11->pin)) {
				if (__HAL_TIM_GET_COUNTER(dht11->htim)>90) {
					status = 5; //error code 5: data send fail
					goto ERROR_HANDLE;
				}
			}
			//읽은 데이터를 변수에 쓰기
			if (__HAL_TIM_GET_COUNTER(dht11->htim)>40) {
				data[i] |= (1<<(7-j));
			}
		}
	}
	// 체크썸 확인 :)
	if (data[4] != (uint8_t)(data[0]+data[1]+data[2]+data[3])) {
		status = 6; ////error code 6: check sum error!
		goto ERROR_HANDLE;
	}

	// #4 데이터 리턴 
	*humidity_i = data[0];
	*humidity_d = data[1];
	*temp_i = data[2];
	*temp_d = data[3];

	ERROR_HANDLE:
	__enable_irq();
	return status;

}
