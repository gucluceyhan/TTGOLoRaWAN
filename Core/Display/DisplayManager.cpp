#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), displayOn(true) {
}

bool DisplayManager::begin() {
  // SSD1306 OLED displayi başlat
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    return false;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
  
  displayOn = true;
  return true;
}

void DisplayManager::clear() {
  display.clearDisplay();
}

void DisplayManager::showStartupScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("TTGO LoRaWAN"));
  display.println(F("Baslatiliyor..."));
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println(F("ChirpStack baglantiyor"));
  display.display();
}

void DisplayManager::showConnectionStatus(bool connected) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("LoRaWAN Durumu:"));
  
  display.setCursor(0, 16);
  if (connected) {
    display.println(F("Baglandi"));
  } else {
    display.println(F("Baglaniliyor..."));
  }
  
  display.display();
}

void DisplayManager::showSendStatus(const char *message, bool success) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Son Mesaj:"));
  
  display.setCursor(0, 16);
  display.println(message);
  
  display.setCursor(0, 32);
  if (success) {
    display.println(F("Gonderildi"));
  } else {
    display.println(F("Gonderme hatasi!"));
  }
  
  display.display();
}

void DisplayManager::showJoinStatus(bool joined) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("LoRaWAN Join:"));
  
  display.setCursor(0, 16);
  if (joined) {
    display.println(F("Basarili!"));
    display.setCursor(0, 32);
    display.println(F("Cihaz ag'a baglandi"));
  } else {
    display.println(F("Basarisiz!"));
    display.setCursor(0, 32);
    display.println(F("Ag'a baglanamiyor"));
  }
  
  display.display();
}

void DisplayManager::turnOn() {
  if (!displayOn) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    displayOn = true;
  }
}

void DisplayManager::turnOff() {
  if (displayOn) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOn = false;
  }
}

bool DisplayManager::isOn() const {
  return displayOn;
} 