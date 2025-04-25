#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    16 // Reset pin
#define SCREEN_ADDRESS 0x3C // OLED I2C adresi

class DisplayManager {
public:
  DisplayManager();
  bool begin();
  void showStartupScreen();
  void showConnectionStatus(bool connected);
  void showSendStatus(const char *message, bool success);
  void showJoinStatus(bool joined);
  void clear();
  void turnOn();
  void turnOff();
  bool isOn() const;

private:
  Adafruit_SSD1306 display;
  bool displayOn;
};

#endif // DISPLAY_MANAGER_H 