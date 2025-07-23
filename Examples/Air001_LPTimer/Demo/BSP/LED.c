#include "LED.h"

/* LED GPIO 端口数组 */
#ifdef AIR001_DEVBOARD
GPIO_TypeDef *LED_PORT[MAX_LED_NUM] = {GPIOB, GPIOB, GPIOB};
const uint16_t LED_PIN[MAX_LED_NUM] = {GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_3};
uint8_t LED_STATE[MAX_LED_NUM] = {0, 0, 0}; //是否使能LED
#else
GPIO_TypeDef *LED_PORT[MAX_LED_NUM] = {GPIOB};
const uint16_t LED_PIN[MAX_LED_NUM] = {GPIO_PIN_0};
uint8_t LED_STATE[MAX_LED_NUM] = {0}; //是否使能LED
#endif
/**
 * @brief 配置LED的GPIO。
 * @param Led 指定要配置的LED。
 * 这个参数可以是以下的参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the GPIO_LED Clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = LED_PIN[Led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(LED_PORT[Led], &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
  LED_STATE[Led] = 1; //标记使能LED
}

/**
 * @brief DeInitialize LED GPIO.
 * @param Led 指定要去配置的LED。
 * 这个参数可以是以下值之一：
 * @arg LED1
 * @注意 BSP_LED_DeInit()不会禁用GPIO时钟,只有三个灯都失能的时候会关闭时钟
 * @retval 无
 */
void BSP_LED_DeInit(Led_TypeDef Led)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Turn off LED */
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
  /* DeInit the GPIO_LED pin */
  GPIO_InitStruct.Pin = LED_PIN[Led];
  HAL_GPIO_DeInit(LED_PORT[Led], GPIO_InitStruct.Pin);
  LED_STATE[Led] = 0; //禁用LED
  for(uint8_t i = 0; i < MAX_LED_NUM; i++){ //检查是否所有LED都禁用
    if(LED_STATE[i] == 1){ //如果有一个LED使能，则不关闭GPIO时钟
      return;
    }
  }
  //如果所有LED都禁用，则关闭GPIO时钟
  __HAL_RCC_GPIOB_CLK_DISABLE();
  /* 关闭GPIO时钟 */
}

/**
 * @brief 将选定的LED打开。
 * @param Led 指定要设置的LED。
 * 这个参数可以是以下的参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}

/**
 * @brief 将选定的LED关闭。
 * @param Led 指定要关闭的LED。
 * 这个参数可以是下列参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
 * @brief 切换选定的LED电平。
 * @param Led 指定要切换的LED。
 * 这个参数可以是下列参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}

