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

// Buzzer tanımlaması
#define BUZZER_PIN PIN_B4  // Buzzer'ın bağlı olduğu pin - Çıkış pini

// Buton tanımlamasını yaptım
#define BUTTON PIN_B7  // Butonun bağlı olduğu pin - Giriş pini

// PWM ve servo motor için pin
#define PWM_PIN PIN_C2  // Servo için PWM çıkışı

// Ultrasonik sensör pinleri
#define TRIGGER_PIN PIN_B0
#define ECHO_PIN PIN_B1

// Radar ölçüm menzili (LÜTFEN GİRİNİZ!!!)
#define RADAR_DISTANCE 20 // CM cinsinden mesafe
