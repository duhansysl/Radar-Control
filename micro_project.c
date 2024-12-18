#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pic16f887.h>
#include "lcd.h"  // LCD kütüphanesi

#define _XTAL_FREQ 4000000  // 4 MHz Freq
#define BUTTON_PIN PORTBbits.RB0 // Buton, RB0 pinine bağlanacak
#define TRIG_PIN LATBbits.LB1    // HC-SR04 trig pin RB1'e bağlanacak
#define ECHO_PIN PORTBbits.RB2   // HC-SR04 echo pin RB2'ye bağlanacak
#define BUZZER_PIN LATAbits.LATA0 // Buzzer, RA0 pinine bağlanacak
#define SERVO_PIN LATCbits.LC0  // Servo motor pin, RC0 pinine bağlanacak

// Konfigürasyon ayarları
void config() {
    TRISB = 0x01;  // Buton giriş, diğer RB pinleri çıkış
    TRISC = 0xFE;  // Servo pin çıkış
    TRISA = 0x00;  // Buzzer çıkış
    TRISD = 0x00;  // LCD pinleri çıkış
    ANSEL = 0x00;  // Dijital I/O
    ANSELH = 0x00; // Dijital I/O
    INTCON = 0xC0;  // Global ve periferal kesmeleri aktif et
    T2CON = 0x04;   // Timer2 prescaler 1:1, aktif et
    TMR2 = 0;       // Timer2'yi sıfırla
    TRISBbits.TRISB0 = 1;  // Buton giriş olarak ayarlandı
    
    // Timer1 için yapılandırma (3 saniye için)
    T1CON = 0x31;   // Timer1 prescaler 1:8, aktif et
    TMR1H = 0;      // Timer1 başlangıç değeri
    TMR1L = 0;
}

// Timer2 yapılandırması
void setupTimer2() {
    TMR2IF = 0;       // Timer2 overflow kesme bayrağını sıfırla
    T2CONbits.TMR2ON = 1; // Timer2'yi başlat
    TMR2IE = 1;       // Timer2 overflow kesmesi etkinleştir
    PEIE = 1;         // Peripheral Interrupt Enable
    GIE = 1;          // Global Interrupt Enable
}

// Servo motoru kontrol etmek için PWM sinyali üretme
void moveServo(int angle) {
    int pwmValue = (angle * 10) + 500;  // PWM sinyalini hesapla, 0-180 derece için orantı yapılıyor
    CCP1CONbits.CCP1M = 0b1100;  // PWM moduna ayarla
    CCPR1L = pwmValue >> 2;      // PWM değeri yüksek byte'ı
    CCP1CONbits.DC1B = pwmValue & 0x03; // PWM değeri düşük byte'ı
}

// Ultrasonik sensörden mesafe ölçümü
int readDistance() {
    int distance;
    TRIG_PIN = 1;  // Trigger pulse başlat
    __delay_us(10); // 10 mikrosaniye
    TRIG_PIN = 0;
    while (ECHO_PIN == 0); // Echo pin HIGH olana kadar bekle
    TMR1H = 0; TMR1L = 0;  // Timer'ı sıfırla
    while (ECHO_PIN == 1);  // Echo pin LOW olana kadar bekle
    distance = (TMR1H << 8) + TMR1L; // Timer değerini mesafeye çevir
    return distance;
}

// Buton ile başlatma/durdurma
void buttonCheck() {
    static int radarRunning = 0;
    if (BUTTON_PIN == 1) {  // Butona basıldığında
        radarRunning = !radarRunning;  // Radar başlat/durdur
        if (radarRunning) {
            LCD_Clear();
            LCD_Set_Cursor(1, 1);
            LCD_Write_String("Radar Baslatildi");
        } else {
            LCD_Clear();
            LCD_Set_Cursor(1, 1);
            LCD_Write_String("Radar Durduruldu");
        }
    }
}

// Timer2 kesmesi
void __interrupt() ISR() {
    if (TMR2IF) {
        TMR2IF = 0;   // Timer2 kesmesini temizle
        BUZZER_PIN = 0; // Buzzer'ı kapat
        // Radar'ı tekrar başlatabiliriz
    }

    if (TMR1IF) {  // Timer1 kesmesi
        TMR1IF = 0;  // Timer1 kesmesini temizle
        // 3 saniye sonunda radar tekrar çalışacak
        BUZZER_PIN = 1; // Buzzer'ı çaldır
        TMR1H = 0;  // Timer1'yi sıfırla
        TMR1L = 0;
        TMR2 = 0;   // Timer2'yi başlat (3 saniye için)
        TMR2IF = 0; // Timer2 kesmesini sıfırla
    }
}

// Ana fonksiyon
void main() {
    config();           // Konfigürasyon ayarları
    LCD_Init();         // LCD'yi başlat
    setupTimer2();      // Timer2 yapılandırması

    // Servo motorun başlangıç açısını belirleyelim
    int angle = 0;

    while (1) {
        buttonCheck();   // Buton kontrolü
        int distance = readDistance();  // Mesafe ölçümü

        // LCD ekranında mesafeyi göster
        char message[16];
        sprintf(message, "Mesafe: %d cm", distance);
        LCD_Clear();
        LCD_Set_Cursor(1, 1);   // Satır 1, sütun 1'e yerleştir
        LCD_Write_String(message);

        // Servo motorun hareketi ve radar işlemleri
        if (angle < 180) {
            moveServo(angle);
            angle += 1; // Her döngüde servo 1 derece hareket edecek
        } else {
            angle = 0; // 180 dereceye ulaştığında 0'a geri dön
        }

        __delay_ms(100); // 100 ms bekleyelim, bu süreyi ihtiyaca göre değiştirebilirsiniz
    }
}
