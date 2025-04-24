#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class Utils {
public:
  // Debug mesajlarını yazdırma fonksiyonu
  static void printDebug(const char* message, bool newLine = true) {
    #if defined(DEBUG_ENABLED)
      if (newLine) {
        Serial.println(message);
      } else {
        Serial.print(message);
      }
    #endif
  }
  
  // Byte dizisini hex formatında yazdırma
  static void printHex(const uint8_t* buffer, size_t size) {
    #if defined(DEBUG_ENABLED)
      for (size_t i = 0; i < size; i++) {
        if (buffer[i] < 16) {
          Serial.print('0');
        }
        Serial.print(buffer[i], HEX);
      }
      Serial.println();
    #endif
  }
  
  // Mevcut zaman damgasını alma (milisaniye)
  static uint32_t getTimestamp() {
    return millis();
  }
};

#endif // UTILS_H 