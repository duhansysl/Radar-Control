#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pic16f887.h>

#define _XTAL_FREQ 4000000  // 4 MHz Freq
#define BUTTON_PIN PORTBbits.RB0 // Buton, RB0 pinine bağlanacak
#define TRIG_PIN LATBbits.LB1    // HC-SR04 trig pin RB1'e bağlanacak
#define ECHO_PIN PORTBbits.RB2   // HC-SR04 echo pin RB2'ye bağlanacak
#define BUZZER_PIN LATAbits.LATA0 // Buzzer, RA0 pinine bağlanacak
#define SERVO_PIN LATCbits.LC0  // Servo motor pin, RC0 pinine bağlanacak

// LCD için gerekli pin tanımlamaları
#define RS LATDbits.LATD0
#define EN LATDbits.LATD1
#define D4 LATDbits.LATD4
#define D5 LATDbits.LATD5
#define D6 LATDbits.LATD6
#define D7 LATDbits.LATD7

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
void servoDondur(int angle) {
    int pwmDeger = (angle * 10) + 500;  // PWM sinyalini hesapla, 0-180 derece için orantı yapılıyor
    CCP1CONbits.CCP1M = 0b1100;  // PWM moduna ayarla
    CCPR1L = pwmDeger >> 2;      // PWM değeri yüksek byte'ı
    CCP1CONbits.DC1B = pwmDeger & 0x03; // PWM değeri düşük byte'ı
}

// Ultrasonik sensörden mesafe ölçümü
int mesafeyiOlc() {
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

// LCD mesajını ekranda gösterme (Basit bir LCD yazdırma fonksiyonu)
void mesajGoster(char *message) {
    // LCD pinlerini kullanarak mesajı yazdırıyoruz (Örnek basit LCD yazdırma)
    RS = 0;
    EN = 1;
    D4 = (message[0] >> 4) & 0x01;
    D5 = (message[0] >> 5) & 0x01;
    D6 = (message[0] >> 6) & 0x01;
    D7 = (message[0] >> 7) & 0x01;
    EN = 0;
    __delay_ms(1);
    
    // Diğer karakterler için benzer şekilde devam edebiliriz
    // Bu kısmı LCD'nin datasheet'ine göre tamamlayabilirsiniz.
}

// Buton ile başlatma/durdurma
void butonKontrol() {
    static int radarRunning = 0;
    if (BUTTON_PIN == 1) {  // Butona basıldığında
        radarRunning = !radarRunning;  // Radar başlat/durdur
        if (radarRunning) {
            mesajGoster("Radar Baslatildi");
        } else {
            mesajGoster("Radar Durduruldu");
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
}

// Ana fonksiyon
void main() {
    config();           // Konfigürasyon ayarları
    setupTimer2();      // Timer2 yapılandırması

    // Servo motorun başlangıç açısını belirleyelim
    int angle = 0;

    while (1) {
        butonKontrol();   // Buton kontrolü
        int distance = mesafeyiOlc();  // Mesafe ölçümü

        // LCD ekranında mesafeyi göster
        char message[16];
        sprintf(message, "Mesafe: %d cm", distance);
        mesajGoster(message);

        // Servo motorun hareketi ve radar işlemleri
        if (angle < 180) {
            servoDondur(angle);
            angle += 1; // Her döngüde servo 1 derece hareket edecek
        } else {
            angle = 0; // 180 dereceye ulaştığında 0'a geri dön
        }

        __delay_ms(100); // 100 ms bekleyelim, bu süreyi ihtiyaca göre değiştirebilirsiniz
    }
}
