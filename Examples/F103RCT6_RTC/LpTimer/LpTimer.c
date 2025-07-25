#define LPTIMER_IMPLEMENT
#include "LPTimer.h"
// 在这个文件中实现LpTimer下硬件定时器的一些操作接口函数

// 使用hal库的函数实现接口，hal库中有些接口我把函数暴露出来了
#include  "rtc.h"
#include "usart.h" //是为了在唤醒后恢复串口
// 暂停定时器
static void HwLpTimer_Puase(void){ 
  __HAL_RTC_ALARM_DISABLE_IT(&hrtc, RTC_IT_ALRA);
  RTC_EnterInitMode(&hrtc);
  /* Clear flag alarm A */
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
  /* RTC Alarm Interrupt Configuration: Disable EXTI configuration */
  __HAL_RTC_ALARM_EXTI_DISABLE_IT();
  RTC_ExitInitMode(&hrtc);
}

// 启动定时器
static void HwLpTimer_Start(void){ 
    RTC_EnterInitMode(&hrtc);
    /* Clear flag alarm A */
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
    /* Configure the Alarm interrupt */
    __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
    /* RTC Alarm Interrupt Configuration: EXTI configuration */
    __HAL_RTC_ALARM_EXTI_ENABLE_IT();
    __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();
    RTC_ExitInitMode(&hrtc);
}

// 设置定时器周期
static void HwLpTimer_SetWakeUpTime(uint32_t period){ 
    RTC_WriteAlarmCounter(&hrtc, period);
}

// 设置定时器周期
static void HwLpTimer_SetCnt(uint32_t counter){ 
    RTC_WriteTimeCounter(&hrtc, counter);
}

//得到当前定时器的计数值
static uint32_t HwLpTimer_GetCnt(void){ 
    return RTC_ReadTimeCounter(&hrtc);
}

//唤醒任何一个LpTImer后会执行的hook函数 (可以用于低功耗模式下配置唤醒后的时钟)
static void HwLpTimer_WakeUpHook(void* p_context){ 
    HAL_Init();
    SystemClock_Config(); //恢复HSE时钟源
    HAL_UART_MspInit(&huart1); // 重新初始化USART1
}

