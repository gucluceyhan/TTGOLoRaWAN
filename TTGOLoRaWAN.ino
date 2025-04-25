#include "Core/Config/AppConfig.h"
#include "Core/Lora/LoraManager.h"
#include "Features/Messaging/MessageService.h"
#include "Core/Display/DisplayManager.h"

// Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

// Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Nesneler
LoraManager loraManager;
MessageService messageService;
DisplayManager displayManager;

// Zaman yönetimi
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 60000; // 60 saniyede bir veri gönder

// define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define DIO1 33  // TTGO LoRa32 V2.1 için DIO pinleri
#define DIO2 32

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 868E6

// OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

String inputString = "";        // Serial'dan gelen komutları tutmak için
bool stringComplete = false;    // Serial komutun tamamlandığını belirtmek için
bool displayOn = true;          // Ekranın açık olup olmadığını takip etmek için
int transmitCounter = 0;        // Gönderilen paket sayacı

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  // initialize Serial Monitor
  Serial.begin(115200);
  inputString.reserve(200);
  
  Serial.println("TTGO LoRaWAN Test");
  Serial.println("Komutlar:");
  Serial.println("DISPLAY_ON - Ekranı açar");
  Serial.println("DISPLAY_OFF - Ekranı kapatır");
  Serial.println("STATUS - Cihaz durumunu gösterir");
  Serial.println("TRANSMIT - Bir LoRa paketi gönderir");
  Serial.println("SF9 - Spreading Factor'ü 9 yapar");
  Serial.println("SF10 - Spreading Factor'ü 10 yapar");
  Serial.println("SF11 - Spreading Factor'ü 11 yapar");

  // reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 başlatma hatası"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("TTGO LoRaWAN Test");
  display.display();
  
  // SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  // setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa başlatılamadı!");
    display.setCursor(0,10);
    display.print("LoRa başlatılamadı!");
    display.display();
    while (1);
  }
  
  Serial.println("LoRa başarıyla başlatıldı!");
  display.setCursor(0,10);
  display.print("LoRa hazır!");
  display.display();
  
  // LoRa parametrelerini ayarla
  LoRa.setSpreadingFactor(9);      // SF9
  LoRa.setSignalBandwidth(125E3);  // 125 kHz
  LoRa.setCodingRate4(5);          // 4/5 coding rate
  LoRa.setTxPower(14);             // 14 dBm
  LoRa.enableCrc();                // CRC etkinleştir
  
  delay(2000);
  updateDisplay();
}

void loop() {
  // Seri porttan gelen komutları işle
  if (stringComplete) {
    processCommand();
    inputString = "";
    stringComplete = false;
  }
  
  // LoRa paketlerini dinle
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    receiveMessage(packetSize);
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void processCommand() {
  inputString.trim();
  
  if (inputString.equals("DISPLAY_ON")) {
    displayOn = true;
    display.ssd1306_command(SSD1306_DISPLAYON);
    Serial.println("Ekran açıldı");
    updateDisplay();
  } 
  else if (inputString.equals("DISPLAY_OFF")) {
    displayOn = false;
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.println("Ekran kapatıldı");
  }
  else if (inputString.equals("STATUS")) {
    Serial.println("Cihaz Durumu:");
    Serial.print("Ekran: ");
    Serial.println(displayOn ? "AÇIK" : "KAPALI");
    Serial.print("LoRa Frekansı: ");
    Serial.print(BAND / 1E6);
    Serial.println(" MHz");
    Serial.print("Spreading Factor: ");
    Serial.println(LoRa.getSpreadingFactor());
  }
  else if (inputString.equals("TRANSMIT")) {
    sendPacket();
  }
  else if (inputString.equals("SF9")) {
    LoRa.setSpreadingFactor(9);
    Serial.println("Spreading Factor 9 olarak ayarlandı");
    updateDisplay();
  }
  else if (inputString.equals("SF10")) {
    LoRa.setSpreadingFactor(10);
    Serial.println("Spreading Factor 10 olarak ayarlandı");
    updateDisplay();
  }
  else if (inputString.equals("SF11")) {
    LoRa.setSpreadingFactor(11);
    Serial.println("Spreading Factor 11 olarak ayarlandı");
    updateDisplay();
  }
  else {
    Serial.println("Tanınmayan komut. Kullanılabilir komutlar:");
    Serial.println("DISPLAY_ON - Ekranı açar");
    Serial.println("DISPLAY_OFF - Ekranı kapatır");
    Serial.println("STATUS - Cihaz durumunu gösterir");
    Serial.println("TRANSMIT - Bir LoRa paketi gönderir");
    Serial.println("SF9/SF10/SF11 - Spreading Factor'ü değiştirir");
  }
}

void sendPacket() {
  transmitCounter++;
  
  // Paket gönder
  LoRa.beginPacket();
  LoRa.print("TTGO LoRa Test Paket #");
  LoRa.print(transmitCounter);
  LoRa.endPacket();
  
  Serial.print("Paket gönderildi: #");
  Serial.println(transmitCounter);
  
  if (displayOn) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("PAKET GONDERILDI");
    display.setCursor(0,10);
    display.print("Paket #: ");
    display.println(transmitCounter);
    display.setCursor(0,30);
    display.println("Gateway'e gonderiliyor...");
    display.display();
  }
}

void updateDisplay() {
  if (!displayOn) return;
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("TTGO LoRaWAN");
  display.setCursor(0,10);
  display.println("LoRa Hazır");
  display.setCursor(0,20);
  display.print("Freq: ");
  display.print(BAND / 1E6);
  display.println(" MHz");
  display.setCursor(0,30);
  display.print("SF: ");
  display.println(LoRa.getSpreadingFactor());
  display.setCursor(0,40);
  display.print("Son Paket: #");
  display.println(transmitCounter);
  display.display();
}

void receiveMessage(int packetSize) {
  String message = "";
  
  // Mesajı oku
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  
  // RSSI (Sinyal gücü) ve SNR (Sinyal-gürültü oranı) değerlerini al
  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();
  
  Serial.println("Gelen LoRa paketi:");
  Serial.print("Mesaj: ");
  Serial.println(message);
  Serial.print("RSSI: ");
  Serial.println(rssi);
  Serial.print("SNR: ");
  Serial.println(snr);
  
  if (displayOn) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("GELEN PAKET");
    display.setCursor(0,10);
    display.print("Mesaj: ");
    display.println(message);
    display.setCursor(0,30);
    display.print("RSSI: ");
    display.println(rssi);
    display.setCursor(0,40);
    display.print("SNR: ");
    display.println(snr);
    display.display();
  }
} 