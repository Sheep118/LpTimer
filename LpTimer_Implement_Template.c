#define LPTIMER_IMPLEMENT
#include "LPTimer.h"

// 暂停定时器
static void HwLpTimer_Puase(void){ 

}

// 启动定时器
static void HwLpTimer_Start(void){ 

}

// 设置定时器周期
static void HwLpTimer_SetWakeUpTime(uint32_t period){ 

}

// 设置定时器周期
static void HwLpTimer_SetCnt(uint32_t counter){ 

}

//得到当前定时器的计数值
static uint32_t HwLpTimer_GetCnt(void){ 

}

//唤醒任何一个LpTImer后会执行的hook函数 (可以用于低功耗模式下配置唤醒后的时钟)
static void HwLpTimer_WakeUpHook(void* p_context){ 
    
}

