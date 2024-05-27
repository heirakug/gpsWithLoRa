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


#include <M5Unified.h>

#define VIA_GROVE // LoRa UNITを使用する場合は先頭の//を削除

// 以下は機種に合わせてコメントアウトを解除してください


//// * M5Stack Core    with LoRa Module / LoRa UNIT
#define ROT 1
#define TSIZE 4
#define RPOS 180
#define SX 270
#define SY 35
#define SR 15
#ifdef VIA_GROVE
#define RX_pin 22 // LoRa UNIT via GROVE
#define TX_pin 21 // LoRa UNIT via GROVE
#define RESET_pin -1 // No RESET Pin
#define BOOT_pin -1  // No Boot Pin
#else
#define RX_pin 16 // stack LoRa MODULE
#define TX_pin 17 // stack LoRa MODULE
#define RESET_pin 13 // stack LoRa MODULE
#define BOOT_pin 22  // stack LoRa MODULE
#endif

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
//#define ROT 3
//#define TSIZE 3
//#define RPOS 100
//#define SX 210
//#define SY 30
//#define SR 12
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

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
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

void loop(){
  if (Serial2.available() > 0) {
    String rxs = Serial2.readString();
    Serial.print(rxs);
    char Buf[5];
    rxs.toCharArray(Buf, 5);
    int16_t rssi = strtol(Buf,NULL,16);
    rxs = rxs.substring(4);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.fillCircle(SX, SY, SR, GREEN);
    M5.Lcd.setCursor(0, 8);
    if (rxs.length() > 12){
      M5.Lcd.println("T:" + rxs.substring( 2, 6 ) + " 'C" );
      M5.Lcd.println("H:" + rxs.substring( 8,12 ) + " %"  );
      if (rxs.length() > 16){
        M5.Lcd.println("P:" + rxs.substring(14,18 ) + " hPa");
      }
    } else {
      M5.Lcd.println(rxs);
    }
    M5.Lcd.setCursor(0, RPOS);
    M5.Lcd.print("R: ");
    M5.Lcd.print(rssi);
    M5.Lcd.print(" dB");
    M5.Speaker.tone(4000, 100);
    delay(400);
    M5.Lcd.fillCircle(SX, SY, SR, BLACK);
  }
}
