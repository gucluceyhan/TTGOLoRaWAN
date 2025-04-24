#ifndef MESSAGE_SERVICE_H
#define MESSAGE_SERVICE_H

#include <Arduino.h>
#include "../../Core/Lora/LoraManager.h"

class MessageService {
public:
  MessageService();
  
  // Servis başlatma
  void setup(LoraManager* loraManager);
  
  // Metin mesajı gönderme
  bool sendMessage(const char* message);
  
  // Özel veri formatı gönderme
  bool sendData(uint8_t* data, uint8_t size, uint8_t port = 1);
  
private:
  LoraManager* loraManager;
  
  // Mesaj gönderim durumu
  bool messagePending;
  
  // TX tamamlandı geri çağırma işlevi
  static void onTxComplete(bool success);
  
  // LoRa olayları geri çağırma işlevi
  static void onLoraEvent(ev_t event);
  
  // Static pointer to MessageService instance for callbacks
  static MessageService* messageServiceInstance;
};

#endif // MESSAGE_SERVICE_H 