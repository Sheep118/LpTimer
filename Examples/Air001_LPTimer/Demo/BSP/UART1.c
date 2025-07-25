#include "UART1.h"
UART_HandleTypeDef DebugUartHandle;
/**
 * @brief DEBUG_USART GPIO配置,模式配置,115200 8-N-1
 * @param None
 * @retval 无
 */
void BSP_USART_Init(uint32_t baudrate)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  DEBUG_USART_CLK_ENABLE();

  DebugUartHandle.Instance = DEBUG_USART;

  DebugUartHandle.Init.BaudRate = baudrate;
  DebugUartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  DebugUartHandle.Init.StopBits = UART_STOPBITS_1;
  DebugUartHandle.Init.Parity = UART_PARITY_NONE;
  DebugUartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  DebugUartHandle.Init.Mode = UART_MODE_TX_RX;

  HAL_UART_Init(&DebugUartHandle);

  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();

  /**USART GPIO Configuration
    PA2     ------> USART1_TX
    PA3     ------> USART1_RX
    */
  GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
  HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
  GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;

  HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);

  /* ENABLE NVIC */
  HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 0, 1);
  HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
  __HAL_UART_ENABLE_IT(&DebugUartHandle,UART_IT_RXNE);
}


void BSP_USART_Deinit(void)
{
    __HAL_UART_DISABLE_IT(&DebugUartHandle,UART_IT_RXNE);
    __HAL_UART_DISABLE(&DebugUartHandle);
    HAL_GPIO_DeInit(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN);
    HAL_GPIO_DeInit(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN);
}

int BSP_USART_Printf(const char *format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    HAL_UART_Transmit(&DebugUartHandle, (uint8_t*)buf, len, 0xFFFF);
    return len;
}

