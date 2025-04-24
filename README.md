# TTGO LoRa32 LoRaWAN Bağlantı Projesi

Bu proje, TTGO LoRa32 V2.1 geliştirme kartı kullanarak ChirpStack LoRaWAN ağına bağlanma ve "Hello World" mesajı gönderme işlemini gerçekleştirir.

## Donanım Gereksinimleri

- TTGO LoRa32 V2.1 (ESP32 + SX1276 LoRa)
- USB Kablo (programlama için)
- LoRaWAN Ağ Geçidi (Gateway)

## Yazılım Bağımlılıkları

Bu proje aşağıdaki kütüphanelere bağımlıdır:

- [MCCI LoRaWAN LMIC library](https://github.com/mcci-catena/arduino-lmic) v4.0.0 veya daha yenisi
- [Arduino ESP32](https://github.com/espressif/arduino-esp32) 2.0.0 veya daha yenisi

## Kurulum

1. Arduino IDE'yi yükleyin
2. Kart Yöneticisi'nden ESP32 desteğini ekleyin:
   - Tercihler -> Ek Kartlar Yöneticisi URL'leri -> `https://dl.espressif.com/dl/package_esp32_index.json` ekleyin
   - Araçlar -> Kart -> Kart Yöneticisi -> "ESP32" aratın ve yükleyin
3. Gerekli Arduino kütüphanelerini yükleyin:
   - Taslak -> Kütüphane Ekle -> Kütüphane Yönet -> "MCCI LoRaWAN LMIC" aratın ve yükleyin
4. Kart ayarlarını yapın:
   - Araçlar -> Kart -> ESP32 Arduino -> "TTGO LoRa32-OLED" seçin
   - Araçlar -> Flash Frekansı -> 80MHz
   - Araçlar -> Upload Speed -> 921600
5. `libraries/arduino-lmic/src/lmic/config.h` dosyasını düzenleyin:
   - `#define CFG_eu868 1` satırını etkinleştirin (Avrupa bandı için)
   - `#define LMIC_ENABLE_arbitrary_clock_error 1` satırını etkinleştirin
   - Diğer bölgesel bantları devre dışı bırakın

## Kullanım

1. `AppConfig.h` dosyasından kendi LoRaWAN kimlik bilgilerinizi (DevEUI, AppEUI, AppKey) ayarlayın
2. Kodu derleyin ve TTGO LoRa32 kartına yükleyin
3. Serial Monitor'ü 115200 baud hızında açın
4. Cihazın ChirpStack ağına bağlanmasını izleyin
5. Başarılı bağlantı sonrası cihaz her dakika bir "Hello World" mesajı gönderecektir

## Proje Yapısı

```
.
├── TTGOLoRaWAN.ino          # Ana Arduino taslak dosyası
├── Core/                    # Çekirdek bileşenler
│   ├── Config/              # Yapılandırma dosyaları
│   │   └── AppConfig.h      # Uygulama sabitleri ve yapılandırması
│   ├── Utils/               # Yardımcı fonksiyonlar
│   │   └── Utils.h          # Genel yardımcı fonksiyonlar
│   └── Lora/                # LoRa işleme kodu
│       ├── LoraManager.h    # LoRa bağlantı yöneticisi header
│       └── LoraManager.cpp  # LoRa bağlantı yöneticisi uygulaması
└── Features/                # Uygulama özellikleri
    └── Messaging/           # Mesajlaşma işlevleri
        ├── MessageService.h    # Mesaj servisi header
        └── MessageService.cpp  # Mesaj servisi uygulaması
```

## Sorun Giderme

- Cihaz ağa bağlanamıyorsa:
  - DevEUI, AppEUI ve AppKey değerlerinin ChirpStack sunucusuyla eşleştiğinden emin olun
  - Cihazın LoRaWAN ağ geçidi kapsama alanında olduğundan emin olun
  - Frekans bandının doğru yapılandırıldığından emin olun (EU868)

- Mesajlar gönderilmiyorsa:
  - Cihazın ağa başarıyla katıldığını kontrol edin
  - ChirpStack sunucu yapılandırmasını kontrol edin 