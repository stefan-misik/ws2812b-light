/**
 * @file
 */

#ifndef SUPPORT_CPU_PINS_H_
#define SUPPORT_CPU_PINS_H_

#include "stm32g0xx_ll_gpio.h"


#define CTRL_TX_Pin LL_GPIO_PIN_2
#define CTRL_TX_GPIO_Port GPIOA
#define CTRL_RX_Pin LL_GPIO_PIN_3
#define CTRL_RX_GPIO_Port GPIOA
#define CTRL_EN_Pin LL_GPIO_PIN_4
#define CTRL_EN_GPIO_Port GPIOA
#define BUZZER_Pin LL_GPIO_PIN_6
#define BUZZER_GPIO_Port GPIOA
#define STATUS_LED1_Pin LL_GPIO_PIN_1
#define STATUS_LED1_GPIO_Port GPIOB
#define STATUS_LED2_Pin LL_GPIO_PIN_2
#define STATUS_LED2_GPIO_Port GPIOB
#define STATUS_LED3_Pin LL_GPIO_PIN_8
#define STATUS_LED3_GPIO_Port GPIOA
#define LED_DATA_Pin LL_GPIO_PIN_9
#define LED_DATA_GPIO_Port GPIOA
#define IR_DATA_Pin LL_GPIO_PIN_6
#define IR_DATA_GPIO_Port GPIOC
#define DEBUG_1_Pin LL_GPIO_PIN_10
#define DEBUG_1_GPIO_Port GPIOA
#define DEBUG_2_Pin LL_GPIO_PIN_11
#define DEBUG_2_GPIO_Port GPIOA
#define DEBUG_3_Pin LL_GPIO_PIN_12
#define DEBUG_3_GPIO_Port GPIOA
#define BTN_O_Pin LL_GPIO_PIN_15
#define BTN_O_GPIO_Port GPIOA
#define BTN_LEFT_Pin LL_GPIO_PIN_3
#define BTN_LEFT_GPIO_Port GPIOB
#define BTN_DOWN_Pin LL_GPIO_PIN_4
#define BTN_DOWN_GPIO_Port GPIOB
#define BTN_RIGHT_Pin LL_GPIO_PIN_5
#define BTN_RIGHT_GPIO_Port GPIOB
#define BTN_X_Pin LL_GPIO_PIN_6
#define BTN_X_GPIO_Port GPIOB
#define BTN_UP_Pin LL_GPIO_PIN_7
#define BTN_UP_GPIO_Port GPIOB
#define I2C_SCL_Pin LL_GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin LL_GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOB
#define TRACE_Pin LL_GPIO_PIN_5
#define TRACE_GPIO_Port GPIOA



#endif  // SUPPORT_CPU_PINS_H_
