//#include <M5Unified.h>

#define VIA_GROVE  // LoRa UNITを使用する場合は先頭の//を削除

//// * M5Stack Core2    with LoRa Module / LoRa UNIT
#define ROT 1
#define TSIZE 4
#define RPOS 180
#define SX 270
#define SY 35
#define SR 15
#ifdef VIA_GROVE
#define RX_pin 32     // LoRa UNIT via Grove
#define TX_pin 26     // LoRa UNIT via Grove
#define RESET_pin -1  // No RESET Pin
#define BOOT_pin -1   // No Boot Pin
#else
#define RX_pin 13     // stack LoRa MODULE
#define TX_pin 14     // stack LoRa MODULE
#define RESET_pin 19  // stack LoRa MODULE
#define BOOT_pin 22   // stack LoRa MODULE
#endif

#include "LoRa.h"
#include "M5Atom.h"
#include <SPI.h>
#include "FS.h"
#include <SD.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;
static char sendloratext[200] = "";

BluetoothSerial SerialBT;

uint64_t chipid;
char chipname[256];

// const char filename[] = "/GPSdata.txt";
// File txtFile;

float Lat;
float Lon;
String Utc;

unsigned long prev, next, interval;

void setup() {

  M5.begin(true, false, true);

  LoRaInit(RX_pin, TX_pin, RESET_pin, BOOT_pin);

  chipid = ESP.getEfuseMac();
  sprintf(chipname, "SerialBT_%04X", (uint16_t)(chipid >> 32));
  Serial.printf("Bluetooth: %s\n", chipname);

  //Serial.begin(115200);
  Serial1.begin(19200, SERIAL_8N1, 22, -1);
  Serial.println(F("GPSを初期化しています..."));
  SerialBT.begin(chipname);

  prev = 0;         // 前回実行時刻を初期化
  interval = 5000;  // 実行周期を設定 5秒ごと
}

void loop() {

  unsigned long curr = millis();    // 現在時刻を取得
  if ((curr - prev) >= interval) {  // 前回実行時刻から実行周期以上経過していたら
    // do periodic tasks            // 周期処理を実行

    while (Serial1.available() > 0) {
      char c = Serial1.read();  // 1バイト読み込む
      String data = Serial1.readStringUntil('\n');

      // $GNRMCと$GNGGAと$GNGLLのNMEAデータをそれぞれの変数に格納
      if (data.length() > 0) {
        //Serial.println(data);
        if (data.substring(0, 6) == "GNGGA,") {
          // $GNGAAのNMEAデータを取得した場合、処理を行う
          parseGGA(data);
          data = "";
        } else if (data.substring(0, 6) == "GNRMC,") {
          // $GNRMCのNMEAデータを取得した場合、処理を行う
          parseRMC(data);
          data = "";
        } else if (data.substring(0, 6) == "GNGLL,") {
          // $GNGLLのNMEAデータを取得した場合、処理を行う
          parseGLL(data);
          data = "";
        }
      }
    }

    LoRaCommand(sendloratext);

    prev += interval;  // 前回実行時刻に実行周期を加算
  }

  // LoRa receive
  if (Serial2.available() > 0) {
    String rxs = Serial2.readString();
    Serial.print(rxs);
    char Buf[5];
    rxs.toCharArray(Buf, 5);
    int16_t rssi = strtol(Buf, NULL, 16);
    rxs = rxs.substring(4);
    // M5.Lcd.fillScreen(BLACK);
    // M5.Lcd.fillCircle(SX, SY, SR, GREEN);
    // M5.Lcd.setCursor(0, 8);
    if (rxs.length() > 12) {
      // M5.Lcd.println("T:" + rxs.substring(2, 6) + " 'C");
      // M5.Lcd.println("H:" + rxs.substring(8, 12) + " %");
      if (rxs.length() > 16) {
        // M5.Lcd.println("P:" + rxs.substring(14, 18) + " hPa");
      }
    } else {
      // M5.Lcd.println(rxs);
    }
    // M5.Lcd.setCursor(0, RPOS);
    // M5.Lcd.print("R: ");
    // M5.Lcd.print(rssi);
    // M5.Lcd.print(" dB");
    // M5.Speaker.tone(4000, 100);
    delay(400);
    // M5.Lcd.fillCircle(SX, SY, SR, BLACK);
  }

  // Lora send
  M5.update();
  if (M5.Btn.wasPressed()) {
    LoRaCommand(sendloratext);
    delay(2000);
  }

  delay(100);
}

void parseGGA(String nmea) {
  // GNGGAデータを解析し、必要な情報を取得
  // ここで緯度、経度、海抜高度、ジオイド高度、UTC時刻を取得する処理を行う
  // カンマ区切りで分割
  int index = 0;
  String data[13];
  int start = 6;
  for (int i = 0; i < 12; i++) {
    int end = nmea.indexOf(',', start);
    if (end == -1) end = nmea.length();
    data[i] = nmea.substring(start, end);
    start = end + 1;
  }

  // UTC時刻
  String utc = data[0];

  // 緯度と経度を取得し、度単位に変換
  double latitude = parseCoordinateLat(data[1]);
  double longitude = parseCoordinateLon(data[3]);
  String elevation = data[8];
  String geoseparation = data[10];

  // UTC時刻と緯度と経度をシリアルモニタに表示
  Serial.print(F("UTC: "));
  Serial.print(utc);
  Serial.print(F("緯度: "));
  Serial.print(latitude, 8);
  Serial.print(F(" 経度: "));
  Serial.println(longitude, 6);
  Serial.print(F("海抜高度: "));
  Serial.print(elevation);
  Serial.print(F(" ジオイド高: "));
  Serial.println(geoseparation);
  doc["Utc"] = Utc;
  doc["Type"] = "GNGGA";
  doc["Latitude"] = latitude;
  doc["Longitude"] = longitude;
  doc["Elevation"] = elevation;
  doc["GeoSeparation"] = geoseparation;
  serializeJson(doc, sendloratext);
}

void parseRMC(String nmea) {
  // GNRMCデータを解析し、必要な情報を取得
  // ここで緯度、経度、海抜高度、ジオイド高度、UTC時刻を取得する処理を行う
  // カンマ区切りで分割
  int index = 0;
  String data[13];
  int start = 6;
  for (int i = 0; i < 12; i++) {
    int end = nmea.indexOf(',', start);
    if (end == -1) end = nmea.length();
    data[i] = nmea.substring(start, end);
    start = end + 1;
  }

  // UTC時刻
  String utc = data[0];

  // 緯度と経度を取得し、度単位に変換
  double latitude = parseCoordinateLat(data[2]);
  double longitude = parseCoordinateLon(data[4]);

  // UTC時刻と緯度と経度をシリアルモニタに表示
  Serial.print(F("UTC: "));
  Serial.print(utc);
  Serial.print(latitude, 8);
  Serial.print(F("緯度: "));
  Serial.print(latitude, 8);
  Serial.print(F(" 経度: "));
  Serial.println(longitude, 6);
  doc["Utc"] = Utc;
  doc["Type"] = "GNRMC";
  doc["Latitude"] = latitude;
  doc["Longitude"] = longitude;
  serializeJson(doc, sendloratext);
}

void parseGLL(String nmea) {
  // GNGLLデータを解析し、必要な情報を取得
  // ここで緯度、経度、UTC時刻を取得する処理を行う
  // カンマ区切りで分割
  int index = 0;
  String data[13];
  int start = 6;
  for (int i = 0; i < 12; i++) {
    int end = nmea.indexOf(',', start);
    if (end == -1) end = nmea.length();
    data[i] = nmea.substring(start, end);
    start = end + 1;
  }

  // UTC時刻
  String utc = data[4];

  // 緯度と経度を取得し、度単位に変換
  double latitude = parseCoordinateLat(data[0]);
  double longitude = parseCoordinateLon(data[2]);

  // UTC時刻と緯度と経度をシリアルモニタに表示
  Serial.print(F("UTC: "));
  Serial.print(utc);
  Serial.print(F("緯度: "));
  Serial.print(latitude, 8);
  Serial.print(F(" 経度: "));
  Serial.println(longitude, 6);
  doc["Utc"] = Utc;
  doc["Type"] = "GNGLL";
  doc["Latitude"] = latitude;
  doc["Longitude"] = longitude;
  serializeJson(doc, sendloratext);
}

double parseCoordinateLat(String coord) {
  // 度と分に分割
  int degree = coord.substring(0, 2).toInt();
  double minute = coord.substring(2).toDouble();
  // 度単位に変換
  return degree + (minute / 60.0);
}

double parseCoordinateLon(String coord) {
  // 度と分に分割
  int degree = coord.substring(0, 3).toInt();
  double minute = coord.substring(3).toDouble();
  // 度単位に変換
  return degree + (minute / 60.0);
}
