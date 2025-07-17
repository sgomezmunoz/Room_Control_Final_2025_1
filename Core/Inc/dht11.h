#ifndef DHT11_H_
#define DHT11_H_

#include "stm32l4xx_hal.h"

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
} DHT11_Data_t;

void DHT11_Init(GPIO_TypeDef *DHT_PORT, uint16_t DHT_PIN);
uint8_t DHT11_Read(DHT11_Data_t *data);

#endif /* DHT11_H_ */