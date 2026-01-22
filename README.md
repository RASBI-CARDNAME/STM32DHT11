# 🌡️ STM32 DHT11 Single-Wire Driver (Hardware Timer 기반)

## 1. 프로젝트 개요 (Overview)
DHT11 온습도 센서의 **Single-Wire 통신 프로토콜을 분석하여 STM32 HAL 기반으로 직접 구현한 C 드라이버**입니다.
단순한 `for` 루프 지연(Software Delay)이 아닌, **하드웨어 타이머(TIM)를 활용한 마이크로초(µs) 단위의 정밀 지연**을 사용하여 통신 신뢰성을 확보했습니다.

*   **목적:** AES-128 암호화 프로젝트의 데이터 수집부로 활용하기 위해 개발
*   **특징:** Blocking 모드를 최소화하고, Checksum 검증을 통해 데이터 무결성 보장

---

## 2. 기술 스택 (Tech Stack)
*   **MCU:** STM32F103 (Cortex-M3)
*   **Framework:** STM32 HAL Library
*   **Hardware Feature:** General Purpose Timer (Microsecond Counter)
*   **Protocol:** DHT11 Single-Wire (Asynchronous)

---

## 3. 동작 원리 및 타이밍 분석 (Timing Analysis)
데이터시트에 명시된 타이밍을 오실로스코프로 검증하며 구현했습니다.

### 1) 통신 프로토콜
*   **Start Signal:** MCU가 핀을 18ms 이상 Low로 유지 후 High로 전환
*   **Response:** 센서가 80µs Low → 80µs High 신호 응답
*   **Data Transmission (0 vs 1):**
    *   모든 비트는 50µs Low로 시작
    *   **Bit '0':** 이어지는 High 구간이 **26~28µs**
    *   **Bit '1':** 이어지는 High 구간이 **70µs**
    *   *→ 하드웨어 타이머로 High 구간의 길이를 측정하여 0과 1 판별*

### 2) 데이터 무결성 검사 (Checksum)
*   수신된 40비트 데이터: `Int_RH` + `Dec_RH` + `Int_Temp` + `Dec_Temp` + `CheckSum`
*   **검증 로직:** 앞의 4바이트 합계가 마지막 `CheckSum` 바이트와 일치하는지 확인하여 오류 데이터 필터링

---

## 4. 사용 방법 (Usage)

### 1) 타이머 설정 (CubeMX)
*   임의의 타이머(예: TIM1)를 선택합니다.
*   **Prescaler:** 시스템 클럭에 맞춰 1틱(Tick)이 **1µs**가 되도록 설정합니다. (예: 72MHz 클럭 시 Prescaler = 71)

### 2) 코드 적용
```c
#include "dht11.h"

// 1. 초기화
DHT11_t dht;
dht.GPIO_Channel = GPIOB;
dht.pin = GPIO_PIN_3;
dht.htim = &htim1; // 1µs 단위로 설정된 타이머 핸들
init_DHT11(&dht);

// 2. 데이터 읽기
uint8_t temp_i, temp_d, hum_i, hum_d;
uint8_t status = read_DHT11(&dht, &temp_i, &temp_d, &hum_i, &hum_d);

if (status == 0) {
    // 성공: LCD 출력 또는 암호화 모듈로 전달
} else {
    // 에러 처리: 재시도 로직 수행
}

// 주의: DHT11 센서 특성상 읽기 간격은 최소 2초 이상 권장
HAL_Delay(2000);

```

---

## 5. 오류 코드

| 코드 | 의미                    |
|------|----------------------------|
| 1    | 센서가 라인을 LOW로 끌어내리지 않음 |
| 2    | 센서가 라인을 HIGH로 끌어올리지 않음 |
| 3    | 센서가 HIGH 상태에 고정됨          |
| 4    | 비트 시작 LOW 타임아웃      |
| 5    | 비트 HIGH 펄스 너무 길음    |
| 6    | 체크섬 불일치          |

---


## 📄 라이선스

MIT 라이선스 – 자세한 내용은 [`LICENSE`](LICENSE)를 참조하십시오.
