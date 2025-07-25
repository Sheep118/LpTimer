#include "LPTIM.h"

LPTIM_HandleTypeDef       LPTIMConfig = {0};

void LPTIM_Init(uint16_t arr)
{
  RCC_PeriphCLKInitTypeDef LPTIM_RCC;
  /* LPTIM时钟配置 */
  LPTIM_RCC.PeriphClockSelection = RCC_PERIPHCLK_LPTIM;     /* 选择配置外设时钟：LPTIM */
  LPTIM_RCC.LptimClockSelection = RCC_LPTIMCLKSOURCE_LSI;   /* 选择LPTIM时钟源：LSI 32KHz*/
  /* 外设时钟初始化 */
  if (HAL_RCCEx_PeriphCLKConfig(&LPTIM_RCC) != HAL_OK)
  {
    while(1);
  }
  
  /*使能LPTIM时钟*/
  __HAL_RCC_LPTIM_CLK_ENABLE();
  /*LPTIM配置*/
  LPTIMConfig.Instance = LPTIM;                         /* LPTIM */
  LPTIMConfig.Init.Prescaler = LPTIM_PRESCALER_DIV128;  /* 128分频 */
  LPTIMConfig.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE; /* 立即更新模式 */
  __HAL_LPTIM_AUTORELOAD_SET(&LPTIMConfig, arr);       /* 设置重载值 */   

  /*初始化LPTIM*/
  if (HAL_LPTIM_Init(&LPTIMConfig) != HAL_OK)
  {
    while(1);
  }
}

void LPTIM_Start(void)
{
  /* 使能重载中断 */
  __HAL_LPTIM_ENABLE_IT(&LPTIMConfig, LPTIM_IT_ARRM);

  /* 使能LPTIM */
  __HAL_LPTIM_ENABLE(&LPTIMConfig);
}

void LPTIM_Stop(void)
{
  /* 禁用LPTIM */
  __HAL_LPTIM_DISABLE(&LPTIMConfig);
  /* 禁用重载中断 */
  __HAL_LPTIM_DISABLE_IT(&LPTIMConfig, LPTIM_IT_ARRM);
}
