# STM32 DHT11 Driver (C)

A simple and reliable DHT11 temperature and humidity sensor driver for STM32 microcontrollers using HAL and hardware timer-based microsecond delay.

## 📦 Features

- Written in C using STM32 HAL libraries
- Uses hardware timer (TIMx) for accurate microsecond delays
- Fully non-blocking except for reading period
- Detects and returns detailed error codes
- Lightweight and portable (no external dependencies)

## 🔧 Requirements

- STM32 MCU (tested on STM32F1 series)
- STM32CubeMX generated project using HAL
- DHT11 sensor connected to a GPIO pin
- 1 pull-up resistor (4.7kΩ–10kΩ) recommended on data line

## 🚀 Usage

1. Include the files:

```c
#include "dht11.h"
```

2. Initialize the struct:

```c
DHT11_t dht;
dht.GPIO_Channel = GPIOB;
dht.pin = GPIO_PIN_3;
dht.htim = &htim1; // Timer configured for 1µs resolution
init_DHT11(&dht);
HAL_Delay(1500); // Wait for sensor startup
```

3. Read values:

```c
uint8_t temp_i, temp_d, hum_i, hum_d;
uint8_t status = read_DHT11(&dht, &temp_i, &temp_d, &hum_i, &hum_d);

if (status == 0) {
    // Success
} else {
    // Handle error
}
```

## ⚠️ Error Codes

| Code | Meaning                    |
|------|----------------------------|
| 1    | Sensor did not pull line LOW |
| 2    | Sensor did not pull line HIGH |
| 3    | Sensor stuck HIGH          |
| 4    | Bit start LOW timeout      |
| 5    | Bit HIGH pulse too long    |
| 6    | Checksum mismatch          |

## 📄 License

MIT License – see [`LICENSE`](LICENSE) for details.
