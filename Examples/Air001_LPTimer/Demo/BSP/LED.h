#ifndef __HAL_LED_H
#define __HAL_LED_H


#include "air001xx_hal.h"
#include "air001xx_hal_gpio.h"
#include "air001xx_hal_rcc.h"


#ifdef AIR001_DEVBOARD
typedef enum{
    LED1 = 0,
    LED2,
    LED3,
    LEDRED = LED1,
    LEDGREEN = LED2,
    LEDBLUE = LED3,
    MAX_LED_NUM
} Led_TypeDef;
#else 
typedef enum{
    LED1 = 0,
    MAX_LED_NUM
} Led_TypeDef;
#endif


/* LED相关函数声明 */
void BSP_LED_Init(Led_TypeDef Led);
void BSP_LED_DeInit(Led_TypeDef Led);
void BSP_LED_On(Led_TypeDef Led);
void BSP_LED_Off(Led_TypeDef Led);
void BSP_LED_Toggle(Led_TypeDef Led);

#endif /* __HAL_LED_H */

