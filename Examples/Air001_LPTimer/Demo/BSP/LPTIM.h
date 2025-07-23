#ifndef __LPTIM_H
#define __LPTIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "air001xx_hal.h"
#include "air001xx_hal_lptim.h"
#include "air001xx_hal_gpio.h"
#include "air001xx_hal_rcc.h"

void LPTIM_Init(uint16_t arr);
void LPTIM_Start(void);
void LPTIM_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __LPTIM_H */

