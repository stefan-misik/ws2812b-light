#include "driver/base.hpp"

#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_gpio.h"

#include "support/cpu_pins.h"


namespace driver
{

void Base::init()
{
    clock_init();
    enable_peripheral_clocks();
    configure_pins();
}

void Base::clock_init()
{
  ::LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(::LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  }

  /* HSI configuration and activation */
  ::LL_RCC_HSI_Enable();
  while(::LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* LSI configuration and activation */
  ::LL_RCC_LSI_Enable();
  while(::LL_RCC_LSI_IsReady() != 1)
  {
  }

  /* Main PLL configuration and activation */
  ::LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
  ::LL_RCC_PLL_Enable();
  ::LL_RCC_PLL_EnableDomain_SYS();
  while(::LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  ::LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  ::LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(::LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set APB1 prescaler*/
  ::LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  ::LL_SetSystemCoreClock(64000000);
}

void Base::enable_peripheral_clocks()
{
    ::LL_IOP_GRP1_EnableClock(
            LL_IOP_GRP1_PERIPH_GPIOA |
            LL_IOP_GRP1_PERIPH_GPIOB |
            LL_IOP_GRP1_PERIPH_GPIOC);

    ::LL_AHB1_GRP1_EnableClock(
            LL_AHB1_GRP1_PERIPH_DMA1
            );

    ::LL_APB1_GRP1_EnableClock(
            LL_APB1_GRP1_PERIPH_TIM3 |
            LL_APB1_GRP1_PERIPH_TIM6 |
            LL_APB1_GRP1_PERIPH_WWDG |
            LL_APB1_GRP1_PERIPH_USART2 |
            LL_APB1_GRP1_PERIPH_USART3 |
            LL_APB1_GRP1_PERIPH_I2C1 |
            LL_APB1_GRP1_PERIPH_I2C2);

    ::LL_APB2_GRP1_EnableClock(
            LL_APB2_GRP1_PERIPH_SYSCFG |
            LL_APB2_GRP1_PERIPH_TIM1 |
            LL_APB2_GRP1_PERIPH_TIM16
            );
}


#define GPIO_INIT(name, af)                          \
    gpio_init.Pin = name ## _Pin;                    \
    gpio_init.Alternate = (af);                      \
    ::LL_GPIO_Init(name ## _GPIO_Port, &gpio_init)

void Base::configure_pins()
{
    ::LL_GPIO_InitTypeDef gpio_init;

    // Keypad
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_INPUT;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(BTN_O, LL_GPIO_AF_0);
    GPIO_INIT(BTN_LEFT, LL_GPIO_AF_0);
    GPIO_INIT(BTN_UP, LL_GPIO_AF_0);
    GPIO_INIT(BTN_DOWN, LL_GPIO_AF_0);
    GPIO_INIT(BTN_RIGHT, LL_GPIO_AF_0);
    GPIO_INIT(BTN_X, LL_GPIO_AF_0);

    // Status LEDs
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(STATUS_LED1, LL_GPIO_AF_0);
    GPIO_INIT(STATUS_LED2, LL_GPIO_AF_0);
    GPIO_INIT(STATUS_LED3, LL_GPIO_AF_0);

    // LED Data
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(LED_DATA, LL_GPIO_AF_2);

    // Buzzer
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(BUZZER, LL_GPIO_AF_5);

    // IR Data
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(IR_DATA, LL_GPIO_AF_1);

    // I2C
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(I2C_SCL, LL_GPIO_AF_6);
    GPIO_INIT(I2C_SDA, LL_GPIO_AF_6);

    // Control UART
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(CTRL_TX, LL_GPIO_AF_1);
    GPIO_INIT(CTRL_RX, LL_GPIO_AF_1);
    // Control Enable
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(CTRL_EN, LL_GPIO_AF_0);

    // Debug Trace
#if 0  // Use UART TX for tracing
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(TRACE, LL_GPIO_AF_4);
#else  // Use GPIO output for tracing
    ::LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    GPIO_INIT(TRACE, LL_GPIO_AF_0);
#endif
}

}  // namespace driver
