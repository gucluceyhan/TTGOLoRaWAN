#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// Kullanıcı tarafından belirtilen DEVEUI, APPEUI ve APPKEY değerleri
// NOT: LMIC kütüphanesi için DEVEUI ve APPEUI değerleri LSB formatında olmalıdır
// LSB (En düşük anlamlı bayt ilk) formatı - ChirpStack'teki değerleri ters çevirin

// MCCI_LMIC kütüphanesi saptandı - Versiyonu 5.0.1
// MCCI_LMIC kütüphanesinde AppKey MSB formatında (ChirpStack ile aynı sırada) kullanılır

// ChirpStack'teki değerleri alıp, DEVEUI ve APPEUI için LSB'ye (ters sıra) çevirin
// AppKey'i ise olduğu gibi (MSB formatında) bırakın

static const u1_t PROGMEM DEVEUI[8] = { 0xFF, 0xFE, 0x00, 0xD3, 0xA1, 0x2F, 0x2B, 0x14 }; // LSB formatında
static const u1_t PROGMEM APPEUI[8] = { 0x70, 0xB3, 0xD5, 0x55, 0x39, 0x37, 0xAF, 0x90 }; // LSB formatında
static const u1_t PROGMEM APPKEY[16] = { 0x4E, 0xF4, 0xD7, 0x33, 0x55, 0x39, 0x37, 0xAF, 0x90, 0xD7, 0x15, 0x98, 0x3F, 0xB5, 0x11, 0x27 }; // MSB formatında

// ÖNEMLI NOT:
// ChirpStack'te DEVEUI: 14:2b:2f:a1:d3:00:fe:ff (MSB formatı)
// ChirpStack'te APPEUI: 90:af:37:39:55:d5:b3:70 (MSB formatı)
// ChirpStack'te AppKey: 4e:f4:d7:33:55:39:37:af:90:d7:15:98:3f:b5:11:27 (MSB formatı)

// TTGO LoRa32 V2.1 için pin tanımları
#define LORA_SCK     5    // GPIO5  - SX1276 SCK
#define LORA_MISO    19   // GPIO19 - SX1276 MISO
#define LORA_MOSI    27   // GPIO27 - SX1276 MOSI
#define LORA_CS      18   // GPIO18 - SX1276 CS
#define LORA_RST     14   // GPIO14 - SX1276 RESET
#define LORA_IRQ     26   // GPIO26 - SX1276 IRQ (interrupt request)

// OLED Ekran için pin tanımları
#define OLED_SDA     21   // GPIO21 - SSD1306 SDA
#define OLED_SCL     22   // GPIO22 - SSD1306 SCL
#define OLED_ADDR    0x3C // SSD1306 I2C adresi (genellikle 0x3C veya 0x3D)
#define OLED_WIDTH   128  // OLED genişliği
#define OLED_HEIGHT  64   // OLED yüksekliği

// ESP32 saat hatası düzeltme yüzdesi - daha yüksek değerler daha geniş bir hata payı sağlar
#define CLOCK_ERROR_PERCENTAGE 40

// LMIC_DEBUG_LEVEL değeri (0: devre dışı, 1: hatalar, 2: bilgi, 3: detaylı debug)
#define LORA_DEBUG_LEVEL 3

// Özel alıcı ayarları
#define DISABLE_BEACONS 1     // Varsa Beacon özelliğini devre dışı bırakır
#define DISABLE_PING 1        // Varsa Ping özelliğini devre dışı bırakır
#define DISABLE_JOIN 0        // JOIN işlemini etkinleştirir 
#define ENABLE_SLOW_RX 1      // Daha güvenilir RX için

// LMIC kütüphanesi tarafından kullanılan geri çağırma fonksiyonları için prototip tanımları
void os_getArtEui(u1_t* buf);
void os_getDevEui(u1_t* buf);
void os_getDevKey(u1_t* buf);

#endif // APP_CONFIG_H 