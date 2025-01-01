//----------------------- H E A D E R - C O N F I G U R A T I O N  -------------------------------------------------------

#include <16F877A.h>
#use delay(clock=4M)  // 4 MHz osilatör frekansı
#fuses XT             // Osilatör tipi

// LCD tanımlamalarını yaptım
#define LCD_RS_PIN PIN_D0
#define LCD_RW_PIN PIN_D1
#define LCD_ENABLE_PIN PIN_D2
#define LCD_DATA4 PIN_D3
#define LCD_DATA5 PIN_D4
#define LCD_DATA6 PIN_D5
#define LCD_DATA7 PIN_D6

// LCD kütüphanesi ekledim
#include "lcd.c"
