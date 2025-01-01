//----------------------- H E A D E R - C O N F I G U R A T I O N  -------------------------------------------------------

#include <16F877A.h>
#use delay(clock=4M)  // 4 MHz osilatör frekansı
#fuses XT, WDT        // Osilatör tipi ve Watchdog Timer etkinleştirildi

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

//----------------------- P I C - C O N F I G U R A T I O N  -------------------------------------------------------

void set_pic() {
    set_tris_b(0x22);  							// TRIGGER_PIN çıkış, ECHO_PIN ve BUTTON_PIN giriş olarak ayarla
    set_tris_c(0x00);  							// PWM çıkış pini ayarları
    set_tris_d(0x00);  							// LCD pin ayarları
    setup_timer_0(RTCC_INTERNAL | RTCC_DIV_8);  // Timer0 ayarları
    set_timer0(231);  							// Timer0 başlangıç değeri
    enable_interrupts(INT_timer0);  			// Timer0 kesmesi etkinleştir
    enable_interrupts(GLOBAL);  				// Global kesmeleri etkinleştir
    setup_timer_1(T1_INTERNAL | T1_DIV_BY_2);  	// Timer1 ayarları
    set_timer1(0);  							// Timer1 sıfırla
    lcd_init();									// LCD'yi ayarla

    setup_wdt(WDT_2304MS);  					// Watchdog Timer zaman aşımı süresini ayarla
}

//----------------------- S E R V O  -  M O T O R -------------------------------------------------------

// Gerekli değişkenler
int sayac = 0, duty = 0, adim = 0;
const int deger[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // Servo motorun oransal açı değerleri
int16 gecikme = 0;
int8 yon = 1;  	// 1: Ileri, -1: Geri

// Timer0 kesmesi ile servo kontrolu yapan fonksiyon
#int_timer0
void move_servo() {
    set_timer0(231); 				// Timer0'i yeniden başlat (~1 ms)
    restart_wdt();                  // Watchdog Timer'ı sıfırla
    if (sayac == 0)					// Servo PWM sinyali üretimi
        output_high(pin_c2);  		// PWM sinyali başlat (pin_c2'de)
    if (sayac >= duty)
        output_low(pin_c2);  		// PWM sinyali bitir (pin_c2'de)
    sayac++;
    if (sayac == 50) {
        sayac = 0;
    }
    gecikme++;
    if (gecikme > 320) {  			// Gecikmeyi azaltarak hareketi hızlandır
        gecikme = 0;
        adim += yon;
        if (adim >= 9) {  			// 9. adım sonrasında geri dön
            yon = -1;  				// Geriye doğru hareket et
        } else if (adim <= 0) {  	// 0. adım sonrasında ileriye dön
            yon = 1;  				// Ileriye doğru hareket et
        }
        duty = deger[adim];			// Servo açısını güncelle
    }
}

//----------------------- U L T R A S O N İ K - S E N S Ö R -------------------------------------------------------

// Kalibrasyon faktörü
float calibration_factor = 30.0;  // Mesafe kalibrasyon çarpanı, elimle deneyerek kalibre ettim
unsigned int16 distance;

void measure_distance() {
    unsigned int16 i = 0;

    // Ölçüm fonksiyonu LCD için gerekli kodlar
    lcd_putc("\f");  
    lcd_gotoxy(1, 1);
    lcd_putc("Radar Calisiyor.");
    lcd_gotoxy(1, 2);
    lcd_putc("Taraniyor...");
	
	// SONAR sensör için trigger/echo gönder/al komutları
    output_high(TRIGGER_PIN);
    delay_us(10);
    output_low(TRIGGER_PIN);
    set_timer1(0);
    while (!input(ECHO_PIN) && (get_timer1() < 5000));
	
	//Gerekli mesafe hesaplamaları
    if (get_timer1() >= 5000)
        return;
    set_timer1(0);
    while (input(ECHO_PIN) && (i < 25000))
        i = get_timer1();
    if (i >= 25000)
        return;
	
    distance = i / calibration_factor;
	
	// ÜStte tanımlanan mesafede cisim tespit edilirse yapılacak olan kesme (interrupt) fonksiyonu
    if (distance < RADAR_DISTANCE && distance != 0) {
        disable_interrupts(GLOBAL);  // Tüm kesmeleri devre dışı bırakır
        lcd_putc("\f");  
        lcd_gotoxy(1, 1);
        lcd_putc("CISIM BULUNDU!");
        lcd_gotoxy(1, 2);
        printf(lcd_putc, "Mesafe: %3Lu cm", distance);
        output_high(BUZZER_PIN);  
        delay_ms(3000);  // 3 saniye buzzer çaldırır
        output_low(BUZZER_PIN);
        enable_interrupts(GLOBAL);  // Kesmeleri tekrar etkinleştirir ve servoyu/sensörü harekete geçirir
    }
}

//----------------------- M A I N - F O N K S İ Y O N U -------------------------------------------------------

void main(void) {
    set_pic();  				// PIC ayarlarını yap

    while (TRUE) {
        restart_wdt();          // Watchdog Timer'ı sıfırla
        measure_distance();  	// Mesafe ölçümünü yap
    }
}