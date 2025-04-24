#include "../Core/Display/DisplayManager.h"

DisplayManager::DisplayManager() : currentLogLine(0) {
  oled = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
  
  // Log satırlarını başlangıçta temizle
  for (int i = 0; i < 4; i++) {
    memset(logLines[i], 0, 32);
  }
}

bool DisplayManager::begin() {
  // I2C başlat
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Ekranı başlat
  if (!oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 ekranı başlatılamadı"));
    return false;
  }
  
  // Ekranı ayarla
  oled->clearDisplay();
  oled->setTextSize(1);
  oled->setTextColor(SSD1306_WHITE);
  oled->setCursor(0, 0);
  oled->cp437(true); // Tam karakter seti kullan
  
  return true;
}

void DisplayManager::clear() {
  oled->clearDisplay();
  oled->setCursor(0, 0);
}

void DisplayManager::display() {
  oled->display();
}

void DisplayManager::showStartupScreen() {
  clear();
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->println(F("KEPMARK LoRaWAN"));
  oled->println(F("Baslatiliyor..."));
  oled->println();
  oled->print(F("DevEUI: "));
  
  // DevEUI'yi ekranda göster
  char devEuiStr[20] = {0};
  for (int i = 0; i < 8; i++) {
    snprintf(devEuiStr + strlen(devEuiStr), sizeof(devEuiStr) - strlen(devEuiStr),
             "%02X", DEVEUI[7-i]); // MSB formatında
  }
  oled->println(devEuiStr);
  display();
}

void DisplayManager::showConnectionStatus(bool isConnected) {
  clear();
  oled->setTextSize(1);
  oled->setCursor(0, 0);
  oled->println(F("LoRaWAN Baglantisi:"));
  oled->println();
  
  if (isConnected) {
    oled->println(F("* BAGLI *"));
  } else {
    oled->println(F("Baglaniliyor..."));
  }
  
  // Son log satırlarını göster
  oled->println();
  oled->println(F("Son Olaylar:"));
  
  for (int i = 0; i < 4; i++) {
    int idx = (currentLogLine + i) % 4;
    if (strlen(logLines[idx]) > 0) {
      oled->println(logLines[idx]);
    }
  }
  
  display();
}

void DisplayManager::showLoRaStatus(const char* status, bool connected) {
  oled->clearDisplay();
  oled->setCursor(0, 0);
  oled->println(F("LoRaWAN Durumu:"));
  oled->println();
  oled->println(status);
  oled->println();
  
  if (connected) {
    oled->println(F("Aga baglandi"));
  } else {
    oled->println(F("Aga baglanamadi"));
  }
  
  oled->display();
}

void DisplayManager::showSendStatus(const char* message, bool success) {
  oled->clearDisplay();
  oled->setCursor(0, 0);
  oled->println(F("Veri Gonderimi:"));
  oled->println();
  oled->println(message);
  oled->println();
  
  if (success) {
    oled->println(F("BASARILI"));
  } else {
    oled->println(F("BASARISIZ"));
  }
  
  oled->display();
}

void DisplayManager::showLastValues(const char* sensorData) {
  oled->clearDisplay();
  oled->setCursor(0, 0);
  oled->println(F("Son Olcumler:"));
  oled->println();
  oled->println(sensorData);
  oled->display();
}

void DisplayManager::showDebugInfo(const char* info) {
  oled->clearDisplay();
  oled->setCursor(0, 0);
  oled->println(F("Debug Bilgisi:"));
  oled->println();
  oled->println(info);
  oled->display();
}

void DisplayManager::addLogLine(const char* logLine) {
  // Yeni log satırını kaydet
  strncpy(logLines[currentLogLine], logLine, 31);
  logLines[currentLogLine][31] = '\0'; // Null-terminate
  
  // Log satırı indeksini güncelle
  currentLogLine = (currentLogLine + 1) % 4;
} 