#include "dht11.h"

static GPIO_TypeDef *DHT_PORT;
static uint16_t DHT_PIN;

void DHT11_Init(GPIO_TypeDef *port, uint16_t pin) {
    DHT_PORT = port;
    DHT_PIN = pin;

    // Habilita el DWT para delay_us()
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000L) * us;
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles);
}

static void DHT_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static void DHT_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static uint8_t DHT_CheckResponse(void) {
    delay_us(40);
    if (!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))) {
        delay_us(80);
        if ((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))) {
            delay_us(80);
            return 1;
        }
    }
    return 0;
}

static uint8_t DHT_ReadByte(void) {
    uint8_t i, j;
    uint8_t data = 0;
    for (j = 0; j < 8; j++) {
        while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));  // espera LOW → HIGH
        delay_us(40);
        if (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
            data |= (1 << (7 - j));
        }
        while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));   // espera HIGH → LOW
    }
    return data;
}

uint8_t DHT11_Read(DHT11_Data_t *data) {
    uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2, CheckSum;

    DHT_SetPinOutput();
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    delay_us(20);
    DHT_SetPinInput();

    if (!DHT_CheckResponse()) return 0;

    Rh_byte1 = DHT_ReadByte();
    Rh_byte2 = DHT_ReadByte();
    Temp_byte1 = DHT_ReadByte();
    Temp_byte2 = DHT_ReadByte();
    CheckSum = DHT_ReadByte();

    if ((Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2) != CheckSum) return 0;

    data->humidity = Rh_byte1;
    data->temperature = Temp_byte1;

    return 1;
}