#include "Core/Config/AppConfig.h"
#include "Core/Lora/LoraManager.h"
#include "Features/Messaging/MessageService.h"
#include "Core/Display/DisplayManager.h"

// Nesneler
LoraManager loraManager;
MessageService messageService;
DisplayManager displayManager;

// Zaman yönetimi
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 60000; // 60 saniyede bir veri gönder

void setup() {
  // Serial başlat
  Serial.begin(115200);
  Serial.println(F("TTGO LoRa32 LoRaWAN Başlatılıyor"));
  
  // Ekranı başlat
  if (displayManager.begin()) {
    displayManager.showStartupScreen();
    delay(2000); // Başlangıç ekranını görüntüleme süresi
  } else {
    Serial.println(F("Ekran başlatılamadı!"));
  }
  
  // LoraManager'a DisplayManager'ı bağla
  loraManager.setDisplayManager(&displayManager);
  
  // LoRa'yı başlat
  loraManager.setup();
  
  // Mesaj servisini başlat
  messageService.setup(&loraManager);
  
  Serial.println(F("Mesaj Servisi başlatıldı"));
  Serial.println(F("Setup tamamlandı - ChirpStack'e bağlanıyor"));
  
  // Başlangıç durumunu ekranda göster
  displayManager.showConnectionStatus(false);
}

void loop() {
  // LoRa işlemlerini yürüt
  loraManager.loop();
  
  // Belirli aralıklarla veri gönder (bağlandıysa)
  if (loraManager.isJoined() && millis() - lastSendTime > sendInterval) {
    lastSendTime = millis();
    
    // Test mesajı gönder
    String message = "Hello World";
    Serial.print(F("Gönderiliyor: "));
    Serial.println(message);
    
    // Veriyi doğrudan const char* olarak gönder
    messageService.sendMessage(message.c_str());
    
    // Ekranı güncelle
    displayManager.showSendStatus(message.c_str(), true);
  }
} 