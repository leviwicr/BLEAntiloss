#ifndef _DISPLAYUSE_H_
#define _DISPLAYUSE_H_
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
#define LCD_BL_PIN TFT_BL			// PWD 的 IO 引脚
#define LCD_BL_PWM_CHANNEL 0		// Channel  通道, 0 ~ 16，高速通道（0 ~ 7）由80MHz时钟驱动，低速通道（8 ~ 15）由 1MHz 时钟驱动

void DisplayInit();

#endif
