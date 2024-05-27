/*This is an example used SerialBT,you can can view gps data by connecting 
 * to Bluetooth assistant on your mobilephone or Serial Monitor
 * the GPS log will be written to SD card
 * 
 */

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
#include "GPSAnalyse.h"
#include <SPI.h>
#include "FS.h"
#include <SD.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;
static char sendloratext[100] = "";

BluetoothSerial SerialBT;
GPSAnalyse GPS;

uint64_t chipid;
char chipname[256];

// const char filename[] = "/GPSdata.txt";
// File txtFile;

float Lat;
float Lon;
String Utc;

/*
bool writeLog(String filename) {  //Write GPSdata to SDcard
  txtFile = SD.open(filename, FILE_APPEND);
  if (txtFile) {
    txtFile.print(Lat);
    txtFile.print(", ");
    txtFile.print(Lon);
    txtFile.print(", ");
    txtFile.println(Utc);
    txtFile.close();
  } else {
    return false;
  }
  return true;
}
*/

unsigned long prev, next, interval;

void setup() {

  M5.begin(true, false, true);

  LoRaInit(RX_pin, TX_pin, RESET_pin, BOOT_pin);

  chipid = ESP.getEfuseMac();
  sprintf(chipname, "SerialBT_%04X", (uint16_t)(chipid >> 32));
  Serial.printf("Bluetooth: %s\n", chipname);
  // SPI.begin(23, 33, 19, -1);
  // if (!SD.begin(-1, SPI, 40000000)) {
  //   Serial.println("initialization failed!");
  // }
  // sdcard_type_t Type = SD.cardType();

  // Serial.printf("SDCard Type = %d \r\n", Type);
  // Serial.printf("SDCard Size = %d \r\n", (int)(SD.cardSize() / 1024 / 1024));

  // //M5.dis.fillpix(0x00004f);
  // M5.Lcd.fillScreen(0x00004f);

  Serial1.begin(9600, SERIAL_8N1, 22, -1); //gps unit
  SerialBT.begin(chipname);
  GPS.setTaskName("GPS");
  GPS.setTaskPriority(2);
  GPS.setSerialPtr(Serial1);
  GPS.start();

  prev = 0;        // 前回実行時刻を初期化
  interval = 5000;  // 実行周期を設定 5秒ごと
}

void loop() {

  unsigned long curr = millis();    // 現在時刻を取得
  if ((curr - prev) >= interval) {  // 前回実行時刻から実行周期以上経過していたら
    // do periodic tasks            // 周期処理を実行
    GPS.upDate();
    Lat = GPS.s_GNRMC.Latitude;
    Lon = GPS.s_GNRMC.Longitude;
    Utc = GPS.s_GNRMC.Utc;
    SerialBT.printf("Latitude= %.5f \r\n", Lat);
    SerialBT.printf("Longitude= %.5f \r\n", Lon);
    SerialBT.printf("DATA= %s \r\n", Utc);
    Serial.printf("Latitude= %.5f \r\n", Lat);
    Serial.printf("Longitude= %.5f \r\n", Lon);
    Serial.printf("DATA= %s \r\n", Utc);
    doc["Latitude"] = Lat;
    doc["Longitude"] = Lon;
    doc["DATA"] = Utc;
    serializeJson(doc, sendloratext);
    LoRaCommand(sendloratext);
    // writeLog(filename);
    prev += interval;  // 前回実行時刻に実行周期を加算
  }
  // do other tasks                 // その他の処理を実行

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
