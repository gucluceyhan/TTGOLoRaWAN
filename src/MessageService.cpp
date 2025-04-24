#include "../Features/Messaging/MessageService.h"

// Static instance pointer for callback işlemleri
MessageService* MessageService::messageServiceInstance = nullptr;

MessageService::MessageService() : loraManager(nullptr), messagePending(false) {
  messageServiceInstance = this;
}

void MessageService::setup(LoraManager* manager) {
  loraManager = manager;
  
  // Geri çağırma işlevlerini kaydet
  if (loraManager) {
    loraManager->setTxCompleteCallback(onTxComplete);
    loraManager->setEventCallback(onLoraEvent);
  }
  
  Serial.println(F("Mesaj Servisi başlatıldı"));

  // Kanal kısıtlamasını kaldır - Tüm kanalları kullan
  // Artık kanal kapatma kodu yok - tüm kanallar açık kalacak
  
  // Her JOIN cevabı gelebilecek olan kanalları spesifik olarak etkinleştir
  for (uint8_t i = 0; i < 9; i++) {
    LMIC_enableChannel(i);
  }
  
  Serial.println(F("JOIN için tüm kanallar etkinleştirildi (868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9, 868.8 MHz)"));
}

bool MessageService::sendMessage(const char* message) {
  if (!loraManager || !message) {
    return false;
  }
  
  // Mesaj uzunluğunu kontrol et
  uint8_t length = strlen(message);
  if (length == 0 || length > 51) { // LoRaWAN max payload boyutu için sınırlama
    Serial.println(F("Mesaj boş veya çok uzun"));
    return false;
  }
  
  // Zaten bekleyen bir mesaj var mı kontrol et
  if (messagePending) {
    Serial.println(F("Zaten bekleyen bir mesaj var, lütfen tamamlanmasını bekleyin"));
    return false;
  }
  
  // Metin mesajını gönder
  Serial.print(F("Gönderiliyor: "));
  Serial.println(message);
  
  // Metin mesajını gönderme işlemi
  bool result = loraManager->sendData((uint8_t*)message, length, 1, true); // Onaylı mesaj
  if (result) {
    messagePending = true;
  }
  
  return result;
}

bool MessageService::sendData(uint8_t* data, uint8_t size, uint8_t port) {
  if (!loraManager || !data || size == 0) {
    return false;
  }
  
  // Zaten bekleyen bir mesaj var mı kontrol et
  if (messagePending) {
    Serial.println(F("Zaten bekleyen bir veri var, lütfen tamamlanmasını bekleyin"));
    return false;
  }
  
  // Veriyi gönder
  Serial.print(F("Veri gönderiliyor, boyut: "));
  Serial.println(size);
  
  bool result = loraManager->sendData(data, size, port, true); // Onaylı mesaj
  if (result) {
    messagePending = true;
  }
  
  return result;
}

void MessageService::onTxComplete(bool success) {
  if (messageServiceInstance == nullptr) return;
  
  if (success) {
    Serial.println(F("Mesaj başarıyla gönderildi"));
  } else {
    Serial.println(F("Mesaj gönderimi başarısız"));
  }
  
  messageServiceInstance->messagePending = false;
}

void MessageService::onLoraEvent(ev_t event) {
  // Gelecekteki genişletmeler için LoRa olaylarını işleyin
  // Örneğin, ağdan gelen mesajları işleme
  if (event == EV_TXCOMPLETE && LMIC.dataLen > 0) {
    Serial.println(F("Downlink verisi alındı"));
    // Downlink verilerini işleme kodu buraya eklenebilir
  }
} 