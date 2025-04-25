#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../Config/AppConfig.h"

class DisplayManager {
public:
  DisplayManager();
  
  // Temel fonksiyonlar
  bool begin();
  void clear();
  void display();
  
  // Ekran güncelleme fonksiyonları
  void showStartupScreen();
  void showConnectionStatus(bool isConnected);
  void showLoRaStatus(const char* status, bool connected);
  void showSendStatus(const char* message, bool success);
  void showLastValues(const char* sensorData);
  
  // Debug fonksiyonları
  void showDebugInfo(const char* info);
  void addLogLine(const char* logLine);
  
  // Ekran kontrolü
  void turnOn();
  void turnOff();
  bool isDisplayOn();
  
private:
  Adafruit_SSD1306* oled;
  char logLines[4][32]; // Son 4 log satırını saklamak için
  int currentLogLine;
  bool displayOn;  // Ekranın durumunu takip etmek için
};

#endif // DISPLAY_MANAGER_H 