#ifndef __UART1_H
#define __UART1_H



#include "air001xx_hal.h"
#include "air001xx_hal_uart.h"
#include "air001xx_hal_gpio.h"
#include "air001xx_hal_rcc.h"
#include "stdarg.h"
#include "string.h"


// debug printf redirect config
#define DEBUG_USART USART1

#define DEBUG_USART_CLK_ENABLE()                           \
  do                                                       \
  {                                                        \
    __IO uint32_t tmpreg = 0x00U;                          \
    SET_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN);           \
    /* RCC外围时钟启用后的延迟 */                \
    tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN); \
    UNUSED(tmpreg);                                        \
  } while (0U)

#define DEBUG_USART_RX_GPIO_PORT GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_USART_RX_PIN GPIO_PIN_3
#define DEBUG_USART_RX_AF GPIO_AF1_USART1

#define DEBUG_USART_TX_GPIO_PORT GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_USART_TX_PIN GPIO_PIN_2
#define DEBUG_USART_TX_AF GPIO_AF1_USART1

#define DEBUG_USART_IRQHandler USART1_IRQHandler
#define DEBUG_USART_IRQ USART1_IRQn

void BSP_USART_Init(uint32_t baudrate);
void BSP_USART_Deinit(void);
int BSP_USART_Printf(const char *format, ...);



#endif /* __UART1_H */

