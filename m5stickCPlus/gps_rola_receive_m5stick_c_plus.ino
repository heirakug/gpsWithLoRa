// M5Basic 2.7
// developed by Arduino
// select device: M5Stack-Core2
// LoRa module :  MAIAR-010
// reference : sample >> http://ikkei.akiba.coocan.jp/ikkei_Electronics/UNIT_LR3.html

//
//  920MHz LoRa/FSK RF module ES920LR3
//  receive data display
//
//  MaiaR Create 2022/06/05
//  add RSSI 2023/01/20
//
//
//  対応機種　Applicable model
//  * M5Stack Core     with LoRa Module/LoRa UNIT
//  * M5Stack Core2    with LoRa Module/LoRa UNIT
//  * M5Stack ATOM S3  with LoRa KIT/LoRa UNIT
//  * M5StickC Plus2   with LoRa HAT/LoRa UNIT
//  * M5StickC Plus    with LoRa HAT/LoRa UNIT
//  * M5StickC         with LoRa HAT/LoRa UNIT
//


// #include <M5Unified.h>
#include <M5StickCPlus.h>

#define VIA_GROVE  // LoRa UNITを使用する場合は先頭の//を削除

// 以下は機種に合わせてコメントアウトを解除してください


//// * M5Stack Core    with LoRa Module / LoRa UNIT
// #define ROT 1
// #define TSIZE 2
// #define RPOS 180
// #define SX 270
// #define SY 35
// #define SR 15
// #ifdef VIA_GROVE
// #define RX_pin 22     // LoRa UNIT via GROVE
// #define TX_pin 21     // LoRa UNIT via GROVE
// #define RESET_pin -1  // No RESET Pin
// #define BOOT_pin -1   // No Boot Pin
// #else
// #define RX_pin 16     // stack LoRa MODULE
// #define TX_pin 17     // stack LoRa MODULE
// #define RESET_pin 13  // stack LoRa MODULE
// #define BOOT_pin 22   // stack LoRa MODULE
// #endif

//// * M5Stack Core2    with LoRa Module / LoRa UNIT
//#define ROT 1
//#define TSIZE 4
//#define RPOS 180
//#define SX 270
//#define SY 35
//#define SR 15
//#ifdef VIA_GROVE
//#define RX_pin 33 // LoRa UNIT via Grove
//#define TX_pin 32 // LoRa UNIT via Grove
//#define RESET_pin -1 // No RESET Pin
//#define BOOT_pin -1  // No Boot Pin
//#else
//#define RX_pin 13 // stack LoRa MODULE
//#define TX_pin 14 // stack LoRa MODULE
//#define RESET_pin 19 // stack LoRa MODULE
//#define BOOT_pin 22  // stack LoRa MODULE
//#endif

//// * M5Stack ATOM S3  with LoRa KIT / LoRa UNIT
//#define ROT 1
//#define TSIZE 2
//#define RPOS 100
//#define SX 100
//#define SY 80
//#define SR 8
//#ifdef VIA_GROVE
//#define RX_pin 1 // LoRa UNIT via GROVE
//#define TX_pin 2 // LoRa UNIT via GROVE
//#define RESET_pin -1 // No RESET Pin
//#define BOOT_pin -1  // No Boot Pin
//#else
//#define RX_pin 5 // LoRa KIT
//#define TX_pin 6 // LoRa KIT
//#define RESET_pin 7 // LoRa KIT
//#define BOOT_pin 39 // LoRa KIT
//#endif

//// * M5StickC Plus / Plus2   with LoRa HAT / LoRa UNIT
#define ROT 0    // 縦向きに設定
#define TSIZE 2  // 文字サイズを大きく設定
#define RPOS 0
#define SX 120  // 中央付近の位置に配置
#define SY 10
#define SR 10

#ifdef VIA_GROVE
#define RX_pin 33     // LoRa UNIT via GROVE
#define TX_pin 32     // LoRa UNIT via GROVE
#define RESET_pin -1  // No RESET Pin
#define BOOT_pin -1   // No Boot Pin
#else
#define RX_pin 26     // LoRa HAT
#define TX_pin 0      // LoRa HAT
#define RESET_pin -1  // No RESET Pin
#define BOOT_pin -1   // No Boot Pin
#endif

//// * M5StickC         with LoRa HAT / LoRa UNIT
//#define ROT 3
//#define TSIZE 2
//#define RPOS 62
//#define SX 135
//#define SY 35
//#define SR 8
//#ifdef VIA_GROVE
//#define RX_pin 33 // LoRa UNIT via GROVE
//#define TX_pin 32 // LoRa UNIT via GROVE
//#define RESET_pin -1 // No RESET Pin
//#define BOOT_pin -1  // No Boot Pin
//#else
//#define RX_pin 26 // LoRa HAT
//#define TX_pin 0  // LoRa HAT
//#define RESET_pin -1 // No RESET Pin
//#define BOOT_pin -1  // No Boot Pin
//#endif



#include "LoRa.h"
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

float lat = -1;
float lon = -1;
float utc = -1;
float elv = -1;
float gsp = -1;
const char* type = "none";

void setup() {
  // auto cfg = M5.config();
  // M5.begin(cfg);
  M5.begin();
  LoRaInit(RX_pin, TX_pin, RESET_pin, BOOT_pin);
  M5.Lcd.setRotation(ROT);
  M5.Lcd.setTextSize(TSIZE);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("LoRa");
  M5.Lcd.println("Display");
  M5.Lcd.println("Stand-by");
}

// バッテリーのパーセンテージを計算する関数
int getBatteryPercentage() {
  float vbat = M5.Axp.GetBatVoltage();
  float percent = (vbat - 3.2) / (4.1 - 3.2);
  return round(percent * 100.0f);
}

// JST時間に変換する関数
String convertUTCtoJST(float utc) {
  int hours = int(utc / 10000) + 9;  // UTC+9
  int minutes = int((utc / 100)) % 100;
  int seconds = int(utc) % 100;

  if (hours >= 24) hours -= 24;

  char timeString[10];
  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
  return String(timeString);
}

void loop() {
  if (Serial2.available() > 0) {
    String rxs = Serial2.readString();
    Serial.print(rxs);
    char Buf[5];
    rxs.toCharArray(Buf, 5);
    int16_t rssi = strtol(Buf, NULL, 16);
    rxs = rxs.substring(4);

    DeserializationError error = deserializeJson(doc, rxs);
    if (error) Serial.println("Deserialization error.");
    else {
      utc = doc["Utc"];
      type = doc["Type"];
      lat = doc["Lat"];
      lon = doc["Lon"];
      elv = doc["Elv"];
      gsp = doc["Gsp"];
    }

    int batteryPercentage = getBatteryPercentage();
    String jstTime = convertUTCtoJST(utc);

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.fillCircle(SX, SY, SR, GREEN);
    M5.Lcd.setCursor(0, RPOS + 20 + 20);
    if (rxs.length() > 12) {
      M5.Lcd.println("Utc:" + String(utc));
      M5.Lcd.println("Type:" + String(type));
      M5.Lcd.println("Lat:" + String(lat));
      M5.Lcd.println("Lon:" + String(lon));
      M5.Lcd.println("Elv:" + String(elv));
      M5.Lcd.println("Gsp:" + String(gsp));
    } else {
      M5.Lcd.println(rxs);
    }
    M5.Lcd.setCursor(0, RPOS);
    M5.Lcd.print("R:");
    M5.Lcd.print(rssi);
    M5.Lcd.print(" dB");

    M5.Lcd.setCursor(0, RPOS + 20);  // バッテリー表示位置
    M5.Lcd.printf("Battery: %.0f%%", batteryPercentage);

    delay(400);
    M5.Lcd.fillCircle(SX, SY, SR, BLACK);
  }
}
