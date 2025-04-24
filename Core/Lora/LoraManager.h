#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "../Config/AppConfig.h"
#include "../Utils/Utils.h"

// Forward declaration
class DisplayManager;

typedef void (*LoraEventCallback)(ev_t event);
typedef void (*LoraTxCompleteCallback)(bool success);

class LoraManager {
public:
  LoraManager();
  
  void setup();
  void loop();
  
  // Veri gönderme fonksiyonu
  bool sendData(uint8_t* data, uint8_t size, uint8_t port = 1, bool confirmed = false);
  
  // Olay işleme fonksiyonlarını ayarlama
  void setEventCallback(LoraEventCallback callback);
  void setTxCompleteCallback(LoraTxCompleteCallback callback);
  
  // Cihaz durumunu alma
  bool isJoined() const;
  lmic_t* getLMIC();
  
  // Ekran yöneticisini ayarla
  void setDisplayManager(DisplayManager* display);
  
private:
  // Durum değişkenleri
  bool joined;
  uint32_t lastJoinAttempt;
  
  // Geri çağırma işlevi işaretçileri
  LoraEventCallback eventCallback;
  LoraTxCompleteCallback txCompleteCallback;
  
  // LMIC olay işleme işlevi
  static void onEvent(ev_t ev);
};

#endif // LORA_MANAGER_H 