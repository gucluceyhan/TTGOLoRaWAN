#include "LoraManager.h"

// Static pointer to LoraManager instance for callbacks
static LoraManager* loraManagerInstance = nullptr;

// LMIC için pin konfigürasyonu
const lmic_pinmap LoraManager::lmic_pins = {
  .nss = LORA_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LORA_RST,
  .dio = {LORA_IRQ, LORA_IRQ, LORA_IRQ}, // DIO0, DIO1, DIO2 (hepsi aynı pine bağlı, TTGO'da IRQ multiplexed)
};

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
  // LMIC başlatma
  os_init();
  LMIC_reset();
  
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

  // Adaptive Data Rate'i etkinleştir
  LMIC_setAdrMode(1);
  
  // Otomatik kanal seçimini etkinleştir
  LMIC_setLinkCheckMode(1);
  
  // En yüksek çıkış gücünü ayarla (14 dBm)
  LMIC_setDrTxpow(DR_SF7, 14);
  
  // OTAA ile ağa katılma isteği gönder
  LMIC_startJoining();
  
  Serial.println(F("LoRa Manager başlatıldı, OTAA ile ağa katılma başlatılıyor"));
  Serial.print(F("DEVEUI: "));
  for (int i = 0; i < 8; i++) {
    if (DEVEUI[i] < 0x10) Serial.print('0');
    Serial.print(DEVEUI[i], HEX);
  }
  Serial.println();
}

void LoraManager::loop() {
  // LMIC işlemlerini yürüt
  os_runloop_once();
  
  // Ağa bağlanma durumunu kontrol et
  if (!joined && LMIC.opmode & OP_JOINING) {
    if (millis() - lastJoinAttempt > 60000) { // Her 60 saniyede bir yeni bağlantı denemesi
      lastJoinAttempt = millis();
      Serial.println(F("Ağa bağlantı yeniden deneniyor..."));
      LMIC_startJoining();
    }
  }
}

bool LoraManager::sendData(uint8_t* data, uint8_t size, uint8_t port, bool confirmed) {
  // Veri göndermek için önce ağa bağlı olduğumuzdan emin olalım
  if (!isJoined()) {
    Serial.println(F("Veri gönderilemiyor: Ağa bağlı değil"));
    return false;
  }
  
  // Başka bir iletim bekleniyorsa, gönderme
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("Veri gönderilemiyor: İşlem devam ediyor"));
    return false;
  }
  
  // Veri gönder
  if (confirmed) {
    LMIC_setTxData2(port, data, size, 1); // Onaylı mesaj
  } else {
    LMIC_setTxData2(port, data, size, 0); // Onaysız mesaj
  }
  
  Serial.println(F("Paket kuyruğa alındı"));
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

void LoraManager::onEvent(ev_t ev) {
  if (loraManagerInstance == nullptr) return;
  
  Serial.print(F("LoRa Olayı: "));
  
  switch(ev) {
    case EV_JOINING:
      Serial.println(F("Ağa katılma başlatıldı"));
      break;
    
    case EV_JOINED:
      Serial.println(F("Ağa katıldı"));
      loraManagerInstance->joined = true;
      
      // Veri gönderme modu için anahtar değişkenlerini sıfırla
      LMIC_setLinkCheckMode(0);
      LMIC_setDrTxpow(DR_SF7, 14);
      break;
    
    case EV_JOIN_FAILED:
      Serial.println(F("Ağa katılma başarısız"));
      break;
    
    case EV_REJOIN_FAILED:
      Serial.println(F("Yeniden bağlanma başarısız"));
      break;
    
    case EV_TXCOMPLETE:
      Serial.println(F("Veri gönderildi, iletim tamamlandı"));
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("ACK alındı"));
      }
      
      // Downlink mesajı varsa işle
      if (LMIC.dataLen) {
        Serial.print(F("Veri alındı: "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" byte"));
      }
      
      // Geri çağırma işlevini çağır
      if (loraManagerInstance->txCompleteCallback) {
        loraManagerInstance->txCompleteCallback(true);
      }
      break;
    
    case EV_TXSTART:
      Serial.println(F("İletim başlatıldı"));
      break;
    
    case EV_TXCANCELED:
      Serial.println(F("İletim iptal edildi"));
      if (loraManagerInstance->txCompleteCallback) {
        loraManagerInstance->txCompleteCallback(false);
      }
      break;
    
    case EV_LINK_DEAD:
      Serial.println(F("Bağlantı kesildi"));
      loraManagerInstance->joined = false;
      break;
    
    default:
      Serial.print(F("Bilinmeyen olay: "));
      Serial.println((unsigned) ev);
      break;
  }
  
  // Olay geri çağırma işlevini çağır
  if (loraManagerInstance->eventCallback) {
    loraManagerInstance->eventCallback(ev);
  }
} 