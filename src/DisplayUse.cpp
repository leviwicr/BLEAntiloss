#include "DisplayUse.h"

TFT_eSPI tft=TFT_eSPI();//创建屏幕对象，可指定对象尺寸，不指定将使用User_Setup.h中定义的尺寸
/* 显示屏部分 */
void DisplayInit(){
  // /* 配置LEDC PWM通道属性，PWD通道为 0，频率为1KHz，8位分辨率*/
    ledcSetup(LCD_BL_PWM_CHANNEL, 1000, 8);
 
	// /* 配置LEDC PWM通道属性,通道0的PWM波在LCD_BL_PIN上输出 */
    ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);
 
	ledcWrite(LCD_BL_PWM_CHANNEL, (int)(1 * 255));//满亮度
  tft.init();
  tft.setRotation(0);  //设置显示图像旋转方向
  tft.invertDisplay(0);  //是否反转所有显示颜色
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,2);//将“光标”设置在显示器的左上角（0,0），并选择字体2 
  tft.setTextColor(TFT_WHITE,TFT_BLUE);//将字体颜色设置为白色，背景为蓝色
  tft.setTextSize(1);//将文本大小倍增设置为1
  tft.println("Hello world");
}
