# TTGO LoRa32 LoRaWAN Uygulaması

Bu proje, TTGO LoRa32 V2.1 geliştirme kartı için LoRaWAN protokolünü kullanan bir Arduino uygulamasıdır. MCCI LMIC kütüphanesi kullanılarak LoRaWAN ağına bağlanır ve mesajlar gönderir.

## Donanım Gereksinimleri

- TTGO LoRa32 V2.1 (868/915 MHz SX1276)
- USB kablo
- LoRa Gateway (ChirpStack veya TTN)

## Yazılım Gereksinimleri

- Arduino IDE
- [MCCI LoRaWAN LMIC Kütüphanesi](https://github.com/mcci-catena/arduino-lmic)
- [SSD1306 OLED Kütüphanesi](https://github.com/adafruit/Adafruit_SSD1306)
- ESP32 Arduino Core

## Proje Yapısı

```
TTGOLoRaWAN/
├── Core/                  # Çekirdek bileşenler
│   ├── Config/            # Yapılandırma dosyaları
│   ├── Display/           # Ekran yönetimi
│   ├── Lora/              # LoRa iletişim katmanı
│   └── Utils/             # Yardımcı fonksiyonlar
├── Features/              # Özellikler ve servisler
│   └── Messaging/         # Mesajlaşma servisi
└── src/                   # Kaynak kod uygulamaları
```

## Sorun Giderme Kılavuzu

### 1. Pin Mapping'i Kontrol Et

LMIC'in pin haritası (lmic_pinmap) tam olarak kartındaki SX1276/8 modülünün NSS, RST, DIO0/1/2 pinlerine uymalı. "TTGO LoRa32 V2.1 – PCB 1.6" için önerilen eşleme:

```cpp
const lmic_pinmap lmic_pins = {
  .nss  = 18,             // CS
  .rxtx = LMIC_UNUSED_PIN,
  .rst  = 14,             // Reset
  .dio  = { 26, 32, 33 }  // DIO0, DIO1, DIO2
};
```

Deneyebileceğin farklı kombinasyonlar:
- DIO1/DIO2'yi {33, 32} yerine {32, 33} yap.
- Varsa, DIO2'yi 35 numaralı GPIO'ya taşı.

Eğer yanlış DIO pini verilirse, SX127x alıcı moduna hiç geçmiyor ya da kendi TX sinyalini "RX" gibi okuyor.

### 2. SPI Hattını ve Board'u Doğrula

- Kartın donanımdaki MOSI/MISO/SCK pinlerinin ESP32'nin VSPI (GPIO 23/19/18) mı yoksa HSPI (GPIO 13/12/14) mı kullandığını üretici dokümanından kontrol et.
- LMIC kütüphanesi varsayılan olarak VSPI'yı kullanır:
  - SCK = 18, MOSI = 23, MISO = 19
- Bu hatlardan biri farklıysa, ya board'u kendi ayarlarına göre yeniden kablolamalı ya da özel SPI başlatma yapmalısın:
  ```cpp
  SPIClass hspi = SPIClass(HSPI); 
  hspi.begin(SCK, MISO, MOSI, NSS);
  ```

### 3. Basit Alıcı (RX-only) Test Kodu

LMIC'le uğraşmak yerine doğrudan SX127x kütüphanesiyle bir "paket dinle" örneği çalıştır. Örnek (RadioHead kullanarak):

```cpp
#include <RH_RF95.h>
#define RFM95_CS 18
#define RFM95_RST 14
#define RFM95_INT 26

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(115200);
  delay(10);
  rf95.init();
  rf95.setFrequency(868.3);
  rf95.setSpreadingFactor(9);  // SF9 deneyin
  rf95.setTxPower(14, false);
  Serial.println("RX mode aktif");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      Serial.print("RX: ");
      for (int i=0;i<len;i++) Serial.printf("%02X ", buf[i]);
      Serial.println();
    }
  }
}
```

### 4. JoinAccept Payload'unu Manuel Test Et

ChirpStack'in gönderdiği JoinAccept paketini alıp RX-only sketch'inle yakalayabiliyor musun kontrol et. Eğer yakalarsan, LMIC'in LMIC_decodeJoinAccept() fonksiyonunu çağırarak sonuç alıp alamadığını test edebilirsin.

### 5. Anten, Güç ve Ortam Şartları

- Antenin doğru takıldığından ve metal kısımlarda deformasyon olmadığından emin ol.
- Cihaz ile gateway arası mesafe çok yakınsa, gateway receiver saturasyona uğruyor olabilir—birkaç metre uzağa taşıyıp tekrar dene.
- Farklı SF'ler (SF9, SF10) dene; düşük veri hızları (SF10, SF11) alıcı hassasiyetini artırır.

## Lisans

MIT 