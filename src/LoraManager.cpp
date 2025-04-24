#include "../Core/Lora/LoraManager.h"
#include "../Core/Display/DisplayManager.h"

// Static pointer to LoraManager instance for callbacks
static LoraManager* loraManagerInstance = nullptr;
static DisplayManager* displayInstance = nullptr;

// LMIC için pin konfigürasyonu - global değişken olarak tanımlanması gerekiyor
const lmic_pinmap lmic_pins = {
  .nss = LORA_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LORA_RST,
  .dio = {LORA_IRQ, LORA_IRQ, LORA_IRQ}, // TTGO'da IRQ multiplexed, tüm DIO pinleri aynı
};

// Protokol ve bant adını almak için
const char* LMIC_getNetworkName() {
  #if defined(CFG_eu868)
  return "EU868";
  #elif defined(CFG_us915)
  return "US915";
  #elif defined(CFG_as923)
  return "AS923";
  #elif defined(CFG_kr920)
  return "KR920";
  #elif defined(CFG_in866)
  return "IN866";
  #else
  return "UNKNOWN";
  #endif
}

// Callback fonksiyonları - LMIC kütüphanesi için gerekli
void os_getArtEui(u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

void os_getDevEui(u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

void os_getDevKey(u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

LoraManager::LoraManager() : 
  joined(false), 
  lastJoinAttempt(0),
  eventCallback(nullptr),
  txCompleteCallback(nullptr) {
  loraManagerInstance = this;
}

void LoraManager::setup() {
  // IRQ pin'i için input modunu ayarla (interrupt kullanmadan)
  pinMode(LORA_IRQ, INPUT);
  
  // LMIC başlatma - pin yapılandırması ile
  os_init_ex(&lmic_pins);
  
  // Debug bilgilerini yazdır
  Serial.println(F("LMIC kütüphanesi başlatıldı"));
  Serial.print(F("Debug seviyesi: "));
  Serial.println(LORA_DEBUG_LEVEL);
  
  if (displayInstance) {
    displayInstance->addLogLine("LMIC kutuphanesi");
    displayInstance->addLogLine("basladi");
  }
  
  // Tüm yapıyı sıfırla
  LMIC_reset();
  
  // ESP32 saat hatası düzeltmesi
  LMIC_setClockError(MAX_CLOCK_ERROR * CLOCK_ERROR_PERCENTAGE / 100);
  Serial.print(F("Saat hatası düzeltmesi: "));
  Serial.print(CLOCK_ERROR_PERCENTAGE);
  Serial.println(F("%"));
  
  // RX pencere zamanlamasını ayarla
  LMIC.rxDelay = 5;  // RX1 penceresi için 5 saniye gecikme (ChirpStack'in yapılandırması ile eşleşmeli)
  LMIC.rx1DrOffset = 0;  // SF7 ile alma için dr offset = 0 
  
  // Alıcı konfigürasyonunu agresif şekilde iyileştir
  LMIC.rxsyms = 50; // Uzun sembol bekleme süresi
  
  // Avrupa (EU868) frekans bandını ayarla
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);
  
  // JOIN için SF9 kullan - ChirpStack SF7 ile yanıt veriyor
  LMIC_setDrTxpow(DR_SF9, 14); // Önceki: SF7
  
  // Otomatik kanal seçimini devre dışı bırak
  LMIC_setLinkCheckMode(0);
  
  // Adr adaptif veri hızını devre dışı bırak
  LMIC_setAdrMode(0);
  
  // RX2 için SF9 kullan
  //LMIC.dn2Dr = DR_SF9;
  // RX2 için SF12 kullan (en güvenilir)
  LMIC.dn2Dr = DR_SF9;
  
  Serial.println(F("-----------------------------------"));
  Serial.print(F("Bant ayarı: "));
  Serial.println(LMIC_getNetworkName());
  
  // JOIN için SF9 kullan - ChirpStack SF7 ile yanıt veriyor
  LMIC_setDrTxpow(DR_SF9, 14);
  
  // RX1 için DR offset ayarla - ChirpStack SF7 ile yanıt verdiğinde alabilmek için
  LMIC.rx1DrOffset = 0; // SF7 için dr offset = 0
  
  // RX2 için standart SF9 kullan
  LMIC.dn2Dr = DR_SF9;
  
  // Ağa katılma isteği gönder
  LMIC_startJoining();
  
  Serial.println(F("LoRa Manager başlatıldı, OTAA ile ağa katılma başlatılıyor"));
  Serial.print(F("DEVEUI: "));
  for (int i = 0; i < 8; i++) {
    if (DEVEUI[i] < 0x10) Serial.print('0');
    Serial.print(DEVEUI[i], HEX);
  }
  Serial.println();
  Serial.println(F("-----------------------------------"));
}

void LoraManager::loop() {
  // RX Delay değerini her zaman kontrol et ve düzeltilmesini sağla
  if (LMIC.rxDelay != 5) {
    LMIC.rxDelay = 5; // ChirpStack'in yapılandırmasıyla eşleşmesi için sürekli 5 değerini zorla
    Serial.println(F("RX Delay değeri 5'e sıfırlandı"));
  }
  
  // RX1 ve RX2 DR değerlerini kontrol et
  if (LMIC.rx1DrOffset != 0) {  // SF7 ile almak için dr offset 0 olmalı
    LMIC.rx1DrOffset = 0;
    Serial.println(F("RX1 DR Offset değeri 0'a ayarlandı (SF7)"));
  }
  
  if (LMIC.dn2Dr != DR_SF9) {
    LMIC.dn2Dr = DR_SF9;
    Serial.println(F("RX2 SF değeri SF9'a ayarlandı"));
  }
  
  // MIC hatası kontrolü ve düzeltmesi
  // Bazen JOIN_ACCEPT alınsa bile MIC doğrulama hatası nedeniyle işlenmeyebilir
  // Bu özellikle OTAA JOIN sorunlarını çözmeye yardımcı olur
  static uint32_t lastJoinCheckTime = 0;
  if (!joined && (millis() - lastJoinCheckTime) > 10000) { // Her 10 saniyede bir
    lastJoinCheckTime = millis();
    
    // OP_JOINING modu açıksa ve 30 sn geçtiyse, JOIN yanıtı gelmiş olabilir ancak MIC hatası olmuş olabilir
    if ((LMIC.opmode & OP_JOINING) && (LMIC.txend != 0) && (os_getTime() - LMIC.txend > ms2osticks(30000))) {
      Serial.println(F("JOIN_ACCEPT işleme sorununa karşı önlem alınıyor"));
      Serial.println(F("JOIN yanıtı alınmış olabilir ancak MIC hatası nedeniyle işlenememiş olabilir"));
      
      if (displayInstance) {
        displayInstance->addLogLine("JOIN RESET");
      }
      
      // JOIN isteklerinin üst üste yeniden gönderimini sınırla
      // Eğer JOIN_TXCOMPLETE göremedik ve 30 saniyeden uzun sürdüyse, JOIN'i tekrar başlat
      LMIC_reset();
      delay(1000); // Kısa bekle
      
      // Ayarları tekrar yap
      LMIC_setClockError(MAX_CLOCK_ERROR * CLOCK_ERROR_PERCENTAGE / 100);
      LMIC.rxDelay = 5;
      LMIC.rx1DrOffset = 0;
      LMIC.rxsyms = 50;
      LMIC.dn2Dr = DR_SF9;
      LMIC_setDrTxpow(DR_SF9, 14);
      
      // ADR ve LinkCheck devre dışı
      LMIC_setAdrMode(0);
      LMIC_setLinkCheckMode(0);
      
      // OTAA JOIN'i tekrar başlat
      Serial.println(F("OTAA JOIN tekrar başlatılıyor"));
      LMIC_startJoining();
    }
  }
  
  // Alıcı ayarlarını agresif tutmak için
  static uint32_t lastRxAdjustTime = 0;
  if (millis() - lastRxAdjustTime > 1000) { // Her saniye
    lastRxAdjustTime = millis();
    
    // LoRa alıcı hassasiyetini artır
    LMIC.rxsyms = 50; // Sürekli uzun sembol süresi
    
    // Özellikle JOIN sürecinde, RXRX_PEND durumunda daha detaylı log
    if (!joined && (LMIC.opmode & OP_TXRXPEND)) {
      Serial.print(F("RX Modu Aktif, opmode=0x"));
      Serial.print(LMIC.opmode, HEX);
      Serial.print(F(", freq="));
      Serial.print(LMIC.freq);
      Serial.print(F(", dataLen="));
      Serial.print(LMIC.dataLen);
      
      if (displayInstance) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "RX: %u, %u", LMIC.freq, LMIC.dataLen);
        displayInstance->showDebugInfo(buffer);
      }
      
      // RX verisi varsa detaylı göster
      if (LMIC.dataLen > 0) {
        Serial.print(F("RX Data: "));
        for (int i = 0; i < LMIC.dataLen; i++) {
          if (LMIC.frame[LMIC.dataBeg + i] < 0x10) Serial.print('0');
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
          Serial.print(' ');
        }
        Serial.println();
        
        if (displayInstance) {
          displayInstance->addLogLine("RX DATA ALINDI!");
        }
      }
    }
  }
  
  // LMIC işlemlerini yürüt
  os_runloop_once();
  
  // Debug bilgilerini ekrana yazdır
  static uint32_t lastDebugTime = 0;
  if (millis() - lastDebugTime > 5000) {  // Her 5 saniyede bir
    lastDebugTime = millis();
    Serial.print("LMIC opmode: 0x");
    Serial.println(LMIC.opmode, HEX);
    
    if (displayInstance) {
      String debugInfo = "Opmode: 0x" + String(LMIC.opmode, HEX);
      displayInstance->showDebugInfo(debugInfo.c_str());
    }
    
    // RX pencereleri ve JOIN durumu hakkında ek bilgiler
    Serial.print("RX delay: ");
    Serial.println(LMIC.rxDelay);
    Serial.print("DN2Dr (RX2 SF): ");
    Serial.println(LMIC.dn2Dr);
    Serial.print("Joined: ");
    Serial.println(joined ? "Yes" : "No");
  }
  
  // JOIN işleminde RX pencereleri için özel mantık
  if (!joined && (LMIC.opmode & OP_JOINING)) {
    // JOIN_TX tamamlandıktan sonra, RX pencereleri için hazırlık
    if (LMIC.opmode & OP_TXRXPEND) {
      // Her 1 saniyede durum kontrol et
      static uint32_t lastRxCheckTime = 0;
      if (millis() - lastRxCheckTime > 1000) {
        lastRxCheckTime = millis();
        
        Serial.print(F("JOIN TX sonrası bekleme, opmode=0x"));
        Serial.println(LMIC.opmode, HEX);
        Serial.print(F("RX1 penceresi açılma zamanı: "));
        Serial.println(LMIC.rxtime);
        
        if (displayInstance) {
          char buffer[32];
          snprintf(buffer, sizeof(buffer), "RXT: %lu", LMIC.rxtime);
          displayInstance->showDebugInfo(buffer);
        }
        
        // Şu anki OSTIME ve RXTIME arasındaki farkı bul
        ostime_t now = os_getTime();
        Serial.print(F("Şu anki os_time: "));
        Serial.println(now);
        
        if (LMIC.rxtime > 0) {
          // RX1 penceresi açılana kadar kalan süre (tick olarak)
          ostime_t delta = LMIC.rxtime - now;
          Serial.print(F("RX1 penceresine kalan (tick): "));
          Serial.println(delta);
          
          // Saniye olarak dönüştür (yaklaşık)
          float seconds = osticks2ms(delta) / 1000.0; // ms -> saniye
          Serial.print(F("RX1 penceresine kalan (sn): "));
          Serial.println(seconds);
          
          if (displayInstance) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "RX1: %.2f sn", seconds);
            displayInstance->showDebugInfo(buffer);
          }
          
          // RX1 ile RX2 arasındaki zaman yaklaşık 1 saniyedir
          // RX1 zamanı geçtiyse
          if (delta <= 0) {
            Serial.println(F("RX1 penceresi açık veya geçti"));
            
            if (displayInstance) {
              displayInstance->addLogLine("RX1 acik/gecti");
            }
            
            // RX2 penceresi kontrol
            if (now >= LMIC.rxtime + ms2osticks(6000)) { // 5+1 sn = 6000 ms
              Serial.println(F("RX2 penceresi de geçti"));
              
              if (displayInstance) {
                displayInstance->addLogLine("RX2 gecti");
              }
            } else {
              Serial.println(F("RX2 penceresi için hazırlanıyor veya açık"));
              LMIC.dn2Dr = DR_SF9; // RX2 SF değerini ayarla
              
              if (displayInstance) {
                displayInstance->addLogLine("RX2 acik/hazirlaniyor");
              }
            }
          }
        }
        
        // RX pencerelerini açık tut
        LMIC.rxsyms = 50; // Önceki: 30 - şimdi 50 (daha uzun)
        
        // ÖNEMLİ: JOIN_ACCEPT'i alabilmek için RX2 DR değerini RX2 penceresi hemen açılmadan önce ayarla
        // RX1 zamanı geçtiyse (rxtime'dan 5 sn sonra), RX2 penceresi yaklaşıyor demektir
        if (LMIC.rxtime > 0 && now >= LMIC.rxtime + ms2osticks(5000)) { // 5 saniye = 5000 ms
          // RX1 penceresi geçti, RX2 penceresi açılmak üzere veya açık
          Serial.println(F("RX2 penceresi zamanı - RX2 SF değeri SF9 olarak ayarlanıyor"));
          LMIC.dn2Dr = DR_SF9;
          
          if (displayInstance) {
            displayInstance->addLogLine("RX2 SF9 ayarlandi");
          }
        }
        
        // RX durumunu yaz 
        Serial.print(F("RxDelay: "));
        Serial.print(LMIC.rxDelay);
        Serial.print(F(", RX1DrOffset: "));
        Serial.print(LMIC.rx1DrOffset);
        Serial.print(F(", DN2Dr: "));
        Serial.println(LMIC.dn2Dr);
      }
    }
  }
  
  // Ağa bağlanma durumunu kontrol et ve gerekirse yeniden başlat
  static uint32_t lastResetTime = 0;
  if (!joined) {
    // Eğer 90 saniye geçti ve hala bağlanmadıysak
    if (millis() - lastResetTime > 90000) {
      lastResetTime = millis();
      Serial.println(F("Yeniden LMIC reset ve JOIN başlatılıyor"));
      
      // Ekrana bilgi göster
      if (displayInstance) {
        displayInstance->addLogLine("LMIC reset yapiliyor");
      }
      
      // Kısa bir bekleme
      delay(1000);
      
      // LMIC'yi tamamen yeniden başlat
      LMIC_reset();
      
      // Kısa bir bekleme
      delay(500);
      
      // ESP32 saat hatası düzeltmesi
      LMIC_setClockError(MAX_CLOCK_ERROR * CLOCK_ERROR_PERCENTAGE / 100);
      
      // Kısa bir bekleme
      delay(500);
      
      // RX pencerelerini yapılandır - JOIN_ACCEPT zaman penceresi için önemli
      LMIC.rxDelay = 5;         // RX1 gecikmesi (saniye)
      LMIC.rx1DrOffset = 0;     // RX1 veri hızı ofseti - SF7 için 0
      LMIC.rxsyms = 50;         // RX sembolleri için ek bekleme süresi
      LMIC.dn2Dr = DR_SF9;      // RX2 için Spreading Factor
      
      // Tüm JOIN işlemleri için bu parametreleri sabit tut
      LMIC.dataBeg = 0;
      LMIC.dataLen = 0;
      LMIC.pendTxPort = 0;
      LMIC.pendTxConf = 0;
      
      // JOIN için SF9 kullan - Daha güvenilir iletişim
      LMIC_setDrTxpow(DR_SF9, 14); // Önceki: SF7
      
      // ADR ve LinkCheck devre dışı
      LMIC_setAdrMode(0);
      LMIC_setLinkCheckMode(0);
      
      // Veri gönderme hızını da SF9'a ayarla
      LMIC.datarate = DR_SF9;
      
      // Kısa bir bekleme
      delay(500);
      
      // JOIN isteği gönder
      LMIC_startJoining();
      
      Serial.println(F("JOIN yeniden başlatıldı"));
      
      if (displayInstance) {
        displayInstance->addLogLine("JOIN yeniden basladi");
      }
    }
  }
}

bool LoraManager::sendData(uint8_t* data, uint8_t size, uint8_t port, bool confirmed) {
  // Veri göndermek için önce ağa bağlı olduğumuzdan emin olalım
  if (!isJoined()) {
    Serial.println(F("Veri gönderilemiyor: Ağa bağlı değil"));
    
    // Ekrana bilgi göster
    if (displayInstance) {
      displayInstance->showSendStatus("Aga bagli degil", false);
    }
    
    return false;
  }
  
  // Başka bir iletim bekleniyorsa, gönderme
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("Veri gönderilemiyor: İşlem devam ediyor"));
    
    // Ekrana bilgi göster
    if (displayInstance) {
      displayInstance->showSendStatus("Islem devam ediyor", false);
    }
    
    return false;
  }
  
  // Veri gönder
  if (confirmed) {
    LMIC_setTxData2(port, data, size, 1); // Onaylı mesaj
  } else {
    LMIC_setTxData2(port, data, size, 0); // Onaysız mesaj
  }
  
  Serial.println(F("Paket kuyruğa alındı"));
  
  // Ekrana bilgi göster
  if (displayInstance) {
    displayInstance->showSendStatus("Paket kuyrukta", true);
  }
  
  return true;
}

void LoraManager::setEventCallback(LoraEventCallback callback) {
  eventCallback = callback;
}

void LoraManager::setTxCompleteCallback(LoraTxCompleteCallback callback) {
  txCompleteCallback = callback;
}

bool LoraManager::isJoined() const {
  return joined;
}

lmic_t* LoraManager::getLMIC() {
  return &LMIC;
}

void LoraManager::setDisplayManager(DisplayManager* display) {
  displayInstance = display;
}

void LoraManager::onEvent(ev_t ev) {
  if (loraManagerInstance == nullptr) return;
  
  Serial.print(F("LoRa Olayı: "));
  char logBuffer[32] = {0};
  
  switch(ev) {
    case EV_JOINING:
      Serial.println(F("Ağa katılma başlatıldı"));
      strncpy(logBuffer, "Aga katilma basladi", 31);
      break;
    
    case EV_JOINED:
      Serial.println(F("Ağa katıldı"));
      strncpy(logBuffer, "Aga katildi!", 31);
      loraManagerInstance->joined = true;
      
      // Veri gönderme modu için anahtar değişkenlerini ayarla
      LMIC_setLinkCheckMode(0);
      LMIC_setDrTxpow(DR_SF9, 14);
      
      // JOIN sonrası RX parametrelerini açıkça tekrar ayarla
      LMIC.rxDelay = 5;
      LMIC.rx1DrOffset = 0;  // SF7 için dr offset = 0
      LMIC.dn2Dr = DR_SF9;
      
      // RX sembol sayısını artır
      LMIC.rxsyms = 50;
      
      Serial.println(F("JOIN sonrası LMIC parametreleri tekrar yapılandırıldı"));
      break;
    
    case EV_JOIN_FAILED:
      Serial.println(F("Ağa katılma başarısız"));
      strncpy(logBuffer, "Katilma basarisiz", 31);
      break;
    
    case EV_REJOIN_FAILED:
      Serial.println(F("Yeniden bağlanma başarısız"));
      strncpy(logBuffer, "Yeniden baglanti basarisiz", 31);
      break;
    
    case EV_TXCOMPLETE:
      Serial.println(F("Veri gönderildi, iletim tamamlandı"));
      strncpy(logBuffer, "Veri gonderildi", 31);
      
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("ACK alındı"));
        strncat(logBuffer, " ACK alindi", 31 - strlen(logBuffer));
      }
      
      // Downlink mesajı varsa işle
      if (LMIC.dataLen) {
        Serial.print(F("Veri alındı: "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" byte"));
        
        // Alınan veriyi hexadecimal olarak yazdır
        Serial.print(F("Alınan veri (HEX): "));
        for (int i = 0; i < LMIC.dataLen; i++) {
          if (LMIC.frame[LMIC.dataBeg + i] < 0x10) Serial.print('0');
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
          Serial.print(' ');
        }
        Serial.println();
        
        if (displayInstance) {
          char hexData[16] = {0};
          int maxChars = (LMIC.dataLen < 3) ? LMIC.dataLen : 3;
          for (int i = 0; i < maxChars; i++) {
            sprintf(hexData + (i*2), "%02X", LMIC.frame[LMIC.dataBeg + i]);
          }
          
          char buffer[32];
          snprintf(buffer, sizeof(buffer), "RX: %s... %dB", hexData, LMIC.dataLen);
          displayInstance->addLogLine(buffer);
        }
      }
      
      // Geri çağırma işlevini çağır
      if (loraManagerInstance->txCompleteCallback) {
        loraManagerInstance->txCompleteCallback(true);
      }
      break;
    
    case EV_TXSTART:
      Serial.println(F("İletim başlatıldı"));
      strncpy(logBuffer, "Iletim basladi", 31);
      break;
    
    case EV_TXCANCELED:
      Serial.println(F("İletim iptal edildi"));
      strncpy(logBuffer, "Iletim iptal edildi", 31);
      if (loraManagerInstance->txCompleteCallback) {
        loraManagerInstance->txCompleteCallback(false);
      }
      break;
    
    case EV_LINK_DEAD:
      Serial.println(F("Bağlantı kesildi"));
      strncpy(logBuffer, "Baglanti kesildi", 31);
      loraManagerInstance->joined = false;
      break;
      
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("Join TX tamamlandı (yanıt bekliyor)"));
      strncpy(logBuffer, "Join TX tamamlandi", 31);
      Serial.println(F("JOIN_TXCOMPLETE - Gelen yanıt dinleniyor"));
      
      // JOIN_ACCEPT işleme sürecini iyileştir
      Serial.print(F("RxDelay: "));
      Serial.println(LMIC.rxDelay);
      Serial.print(F("RX1DrOffset: "));
      Serial.println(LMIC.rx1DrOffset);
      Serial.print(F("RxSyMs: "));
      Serial.println(LMIC.rxsyms);
      Serial.print(F("DN2Dr: "));
      Serial.println(LMIC.dn2Dr);
      
      // JOIN_ACCEPT işleme kapasitesini artır
      LMIC.rxsyms = 50; // Çok daha fazla sembol bekleme
      
      // RXMODE'u agresif olarak izle
      Serial.print(F("RXMODE ayarlandı, freq="));
      Serial.println(LMIC.freq);
      
      if (displayInstance) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "JOIN TX OK, f=%u", LMIC.freq);
        displayInstance->addLogLine(buffer);
      }
      break;
      
    // RX olaylarını ekleyelim
    case EV_RXSTART:
      Serial.println(F("RX başladı - JOIN_ACCEPT dinleniyor"));
      
      if (displayInstance) {
        displayInstance->addLogLine("RX basladi");
      }
      break;
    
    default:
      Serial.print(F("Bilinmeyen olay: "));
      Serial.println((unsigned) ev);
      snprintf(logBuffer, 32, "Bilinmeyen olay: %d", (int)ev);
      break;
  }
  
  // Ekrana log satırı ekle
  if (displayInstance && logBuffer[0] != '\0') {
    displayInstance->addLogLine(logBuffer);
    displayInstance->showConnectionStatus(loraManagerInstance->joined);
  }
  
  // Olay geri çağırma işlevini çağır
  if (loraManagerInstance->eventCallback) {
    loraManagerInstance->eventCallback(ev);
  }
} 