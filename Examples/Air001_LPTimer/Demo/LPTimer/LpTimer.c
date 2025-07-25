#include "main.h"
#define LPTIMER_IMPLEMENT
#include "LPTimer.h"


static volatile uint32_t temp = 0;

// 暂停定时器
static void HwLpTimer_Puase(void){ 
  // 暂停定时器
  __HAL_LPTIM_DISABLE(&LPTIMConf); 
}

// 启动定时器
static void HwLpTimer_Start(void){ 
  /* 使能重载中断 */
  __HAL_LPTIM_ENABLE_IT(&LPTIMConf, LPTIM_IT_ARRM);

  /* 使能LPTIM */
  __HAL_LPTIM_ENABLE(&LPTIMConf);

  /* 加载重载值 */
  __HAL_LPTIM_AUTORELOAD_SET(&LPTIMConf, temp);

  /* 延时65us */
  APP_delay_us(65);
  
  /* 开启单次计数模式 */
  __HAL_LPTIM_START_SINGLE(&LPTIMConf);
}

// 设置定时器周期
static void HwLpTimer_SetWakeUpTime(uint32_t period){ 
    temp = period;
}

// 设置定时器周期
static void HwLpTimer_SetCnt(uint32_t counter){ 
    // 设置定时器计数值
  LPTIMConf.Instance->CNT = counter; //直接设置计数寄存器
}

//得到当前定时器的计数值
static uint32_t HwLpTimer_GetCnt(void){ 
	  // BSP_USART_Printf("into get counter\r\n");
    // uint32_t last_cnt = HAL_LPTIM_ReadCounter(&LPTIMConf);
		// BSP_USART_Printf("last = %d\r\n", last_cnt);
    // uint32_t cur_cnt = HAL_LPTIM_ReadCounter(&LPTIMConf);
		// BSP_USART_Printf("cur = %d\r\n", last_cnt);
		// if(last_cnt == cur_cnt)
    //     return cur_cnt;
    // else cur_cnt = HAL_LPTIM_ReadCounter(&LPTIMConf);
    // return cur_cnt;
    uint32_t cur_cnt = HAL_LPTIM_ReadCounter(&LPTIMConf);
    return cur_cnt;
}

//唤醒任何一个LpTImer后会执行的hook函数 (可以用于低功耗模式下配置唤醒后的时钟)
static void HwLpTimer_WakeUpHook(void* p_context){ 
   APP_RCCOscConfig();
   BSP_USART_Init(115200);
}

