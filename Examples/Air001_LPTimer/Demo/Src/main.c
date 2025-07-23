/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) AirM2M.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "LpTimer.h"
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LPTIM_HandleTypeDef       LPTIMConf = {0};
LpTimer test1_timer;
LpTimer test2_timer;
LpTimer test3_timer;
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_LPTIMInit(void);
static void APP_LPTIMStart(uint16_t arr);
//void HWLpTimer_Puase(void);
//void HWLpTimer_Start(void);
//void HWLpTimer_SetWakeUpTime(uint32_t period);
//void HWLpTimer_SetCnt(uint32_t counter);
//uint32_t HWLpTimer_GetCnt(void);
//void HWLpTimer_WakeUpHook(void *p_context);

void test1_timer_hook(void){
    BSP_LED_Toggle(LED1);
    BSP_USART_Printf("test1\r\n");
  }
void test2_timer_hook(void){
  BSP_LED_Off(LED2);
  BSP_USART_Printf("test2\r\n");
}
void test3_timer_hook(void){
    BSP_LED_Toggle(LED3);
    BSP_USART_Printf("test3\r\n");
    static uint8_t i = 0;
    i++; 
    if(i == 2){
      i = 0;
      if(LPTIMER_IS_STOPED(&test2_timer)){
        LpTimer_Start(&test2_timer,MS2TICK(3000));
        BSP_LED_On(LED2);
      }
    }
}

uint8_t into_low_power = 0;

/**
  * @brief   应用程序入口函数
  * @retval  int
  */
int main(void)
{
  /* 外设、systick初始化 */
  HAL_Init();
  
  /* 时钟设置 */
  APP_RCCOscConfig();
  // /* 初始化LED */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);

  /* 初始化按键 */
//  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

  /* LPTIM初始化 */
  APP_LPTIMInit();
  
  /* 使能PWR */
  __HAL_RCC_PWR_CLK_ENABLE();
  BSP_USART_Init(115200);
  BSP_USART_Printf("uart_test!\r\n");
  BSP_USART_Deinit();

  /* 关闭LED */
  BSP_LED_Off(LED1);
  BSP_LED_Off(LED2);
  BSP_LED_Off(LED3);
  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
  LpTimer_Create(&test1_timer, E_LPTIMERMODE_PERIODIC, test1_timer_hook);
  LpTimer_Create(&test2_timer, E_LPTIMERMODE_ONCE, test2_timer_hook);
  LpTimer_Create(&test3_timer, E_LPTIMERMODE_PERIODIC, test3_timer_hook);
  LpTimer_Start(&test1_timer,MS2TICK(1000));
  LpTimer_Start(&test2_timer,MS2TICK(5000));
  LpTimer_Start(&test3_timer,MS2TICK(3200));
  // __HAL_LPTIM_DISABLE(&LPTIMConf);
  // APP_LPTIMStart(256*2);
  // HAL_SuspendTick(); //关闭systick
  // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);//低功耗模式
  
  
  while (1)
  {
    if(into_low_power){
      // HAL_SuspendTick(); //关闭systick
      BSP_USART_Deinit();
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);//低功耗模式
      into_low_power = 0;
    }
    // /* 失能 LPTIM */
    // __HAL_LPTIM_DISABLE(&LPTIMConf);
   
    // /* 使能LPTIM和中断，并开启单次计数模式 */
    // APP_LPTIMStart();
        
    // /* 进入STOP模式，使用中断唤醒 */
    // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    // BSP_LED_Toggle(LEDRED);
    // BSP_LED_Toggle(LEDGREEN);
    // BSP_LED_Toggle(LEDBLUE);
    // BSP_USART_Printf("sheep! hello world!");
    /* 延时500ms */
    // APP_delay_us(500000);
  }
}

/**
  * @brief   时钟配置函数
  * @param   无
  * @retval  无
  */
void APP_RCCOscConfig(void)
{
  RCC_OscInitTypeDef OSCINIT;
  RCC_PeriphCLKInitTypeDef LPTIM_RCC;
  // RCC_ClkInitTypeDef ClkInit; 

  /* LSI时钟配置 */
  OSCINIT.OscillatorType = RCC_OSCILLATORTYPE_LSI;  /* 选择配置LSI */
  OSCINIT.LSIState = RCC_LSI_ON;                    /* LSI开启 */
  OSCINIT.HSIState = RCC_HSI_ON;                    /* HSI开启 */
  OSCINIT.HSIDiv = RCC_HSI_DIV1;                   /* HSI分频1 ,24MHz,唤醒后是8MHz */
  // OSCINIT.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT; /* HSI校准值默认 */
  // HSI的开启似乎没有什么用
  /* 时钟初始化 */
  if (HAL_RCC_OscConfig(&OSCINIT) != HAL_OK)
  {
    Error_Handler();
  }

  // ClkInit.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // ClkInit.APB1CLKDivider = RCC_HCLK_DIV1;
  // ClkInit.ClockType = RCC_CLOCKTYPE_SYSCLK;
  // ClkInit.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  
  // if(HAL_RCC_ClockConfig(&ClkInit, FLASH_LATENCY_0) != HAL_OK){
  //   Error_Handler();
  // }

  /* LPTIM时钟配置 */
  LPTIM_RCC.PeriphClockSelection = RCC_PERIPHCLK_LPTIM;     /* 选择配置外设时钟：LPTIM */
  LPTIM_RCC.LptimClockSelection = RCC_LPTIMCLKSOURCE_LSI;   /* 选择LPTIM时钟源：LSI */
  /* 外设时钟初始化 */
  if (HAL_RCCEx_PeriphCLKConfig(&LPTIM_RCC) != HAL_OK)
  {
    Error_Handler();
  }
  
  /*使能LPTIM时钟*/
  __HAL_RCC_LPTIM_CLK_ENABLE();
}

/**
  * @brief   初始化LPTIM
  * @param   无
  * @retval  无
  */
static void APP_LPTIMInit(void)
{
  /*LPTIM配置*/
  LPTIMConf.Instance = LPTIM;                         /* LPTIM */
  LPTIMConf.Init.Prescaler = LPTIM_PRESCALER_DIV128;  /* 128分频 */ // 32.768KHz / 128 = 256Hz
  /* LPTIM时钟频率为256Hz，计数周期为1/256Hz = 3.90625ms */
  LPTIMConf.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE; /* 立即更新模式 */
  /*初始化LPTIM*/
  if (HAL_LPTIM_Init(&LPTIMConf) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief   使能LPTIM和中断，并开启单次计数模式
  * @param   无
  * @retval  无
  */
static void APP_LPTIMStart(uint16_t arr)
{
  /* 使能重载中断 */
  __HAL_LPTIM_ENABLE_IT(&LPTIMConf, LPTIM_IT_ARRM);

  /* 使能LPTIM */
  __HAL_LPTIM_ENABLE(&LPTIMConf);

  /* 加载重载值 */
  __HAL_LPTIM_AUTORELOAD_SET(&LPTIMConf, arr);

  /* 延时65us */
  APP_delay_us(65);
  
  /* 开启单次计数模式 */
  __HAL_LPTIM_START_SINGLE(&LPTIMConf);
}

/**
  * @brief   LPTIM重装载中断回调函数
  * @param   无
  * @retval  无
  */
// void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
// {
//   APP_RCCOscConfig();
//   BSP_USART_Init(115200);
//   BSP_USART_Printf("LPTIM AutoReload Match Callback!\r\n");
//   BSP_USART_Deinit();
//   BSP_LED_Toggle(LED1);
//   BSP_LED_Toggle(LED2);
//   BSP_LED_Toggle(LED3);
//   __HAL_LPTIM_DISABLE(&LPTIMConf); 
//   //必须先失能定时器，才能写IER中断失能寄存器开启中断
//   //换句话说，要重新开启中断，需要1.失能定时器，2.清除ISR标志位(这个在中断服务函数中做了)
//   APP_LPTIMStart(256*2);
//   into_low_power = 1;
// }
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
    LpTimer_Execute();
    BSP_USART_Printf("%d\r\n", HAL_LPTIM_ReadCounter(&LPTIMConf));
    into_low_power = 1;
}
/**
  * @brief   微秒延时函数
  * @param   nus：延时时间
  * @retval  无
  * @note    此函数会关闭SysTick中断，如需要使用请重新初始化SysTick
  */
void APP_delay_us(uint32_t nus)
 {
  HAL_Init();
  HAL_SuspendTick();
  uint32_t temp;
  SysTick->LOAD=nus*(SystemCoreClock/1000000);
  SysTick->VAL=0x00;
  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
  do
  {
    temp=SysTick->CTRL;
  }
  while((temp&0x01)&&!(temp&(1<<16)));
  SysTick->CTRL=0x00;
  SysTick->VAL =0x00;
 }



//void HWLpTimer_Puase(void){
//  // 暂停定时器
//  __HAL_LPTIM_DISABLE(&LPTIMConf); 
//}  
//void HWLpTimer_Start(void){
//  // 启动定时器
//  /* 使能重载中断 */
//  __HAL_LPTIM_ENABLE_IT(&LPTIMConf, LPTIM_IT_ARRM);
//  /* 使能LPTIM */
//  __HAL_LPTIM_ENABLE(&LPTIMConf);
//  /* 开启单次计数模式 */
//  __HAL_LPTIM_START_SINGLE(&LPTIMConf);
//} 
//void HWLpTimer_SetWakeUpTime(uint32_t period){
//  // 设置定时器周期
//  /* 使能LPTIM */
//  __HAL_LPTIM_ENABLE(&LPTIMConf);
//  /* 加载重载值 */
//  __HAL_LPTIM_AUTORELOAD_SET(&LPTIMConf, (uint16_t)period);
//  /* 延时65us */
//  APP_delay_us(65);
//} 
//void HWLpTimer_SetCnt(uint32_t counter){
//  // 设置定时器计数值
//  LPTIMConf.Instance->CNT = counter; //直接设置计数寄存器
//} 
//uint32_t HWLpTimer_GetCnt(void){
//  //得到当前定时器的计数值
//    return HAL_LPTIM_ReadCounter(&LPTIMConf); 
//  //如果计数值大于重载值，则返回重载值
//} 
//  
//void HWLpTimer_WakeUpHook(void* p_context){
//  APP_RCCOscConfig();
//  BSP_USART_Init(115200);
//  BSP_USART_Printf("LPTIM %d\r\n", HWLpTimer_GetCnt());
//}







/**
  * @brief   错误执行函数
  * @param   无
  * @retval  无
  */
void Error_Handler(void)
{
  /* 无限循环 */
  while (1)
  {
  }
}
#ifdef  USE_FULL_ASSERT
/**
  * @brief  输出产生断言错误的源文件名及行号
  * @param  file：源文件名指针
  * @param  line：发生断言错误的行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* 用户可以根据需要添加自己的打印信息,
     例如: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* 无限循环 */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
