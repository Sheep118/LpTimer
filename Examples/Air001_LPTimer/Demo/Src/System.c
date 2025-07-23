#include "System.h"

void SystemClock_Init(void)
{
    LL_UTILS_ClkInitTypeDef UTLIS_Struct = {LL_RCC_SYSCLK_DIV_1,LL_RCC_APB1_DIV_1};
    SystemInit(); //默认配置HSI为8M，且配置FLash起始位置
    LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz); //更改HSI为24M
    LL_PLL_ConfigSystemClock_HSI(&UTLIS_Struct); //配置PLL为系统时钟 2*HSI
    LL_RCC_ClocksTypeDef RCC_Clocks = {0};
    LL_RCC_GetSystemClocksFreq(&RCC_Clocks); //获取系统时钟
    LL_Init1msTick(RCC_Clocks.HCLK_Frequency); //设置SysTick 1ms运行周期
    LL_SYSTICK_EnableIT(); //使能SYStick中断
}
