#include <Arduino.h>
#include <SPI.h>

#define EPD_CS    27
#define EPD_DC    33
#define EPD_RST   26
#define EPD_MOSI  13
#define EPD_CLK   14

#define EPD_W 400
#define EPD_H 300

SPIClass hspi(HSPI);

void cmd(uint8_t c) {
  digitalWrite(EPD_DC, LOW);
  digitalWrite(EPD_CS, LOW);
  hspi.transfer(c);
  digitalWrite(EPD_CS, HIGH);
}
void dat(uint8_t d) {
  digitalWrite(EPD_DC, HIGH);
  digitalWrite(EPD_CS, LOW);
  hspi.transfer(d);
  digitalWrite(EPD_CS, HIGH);
}

void hardReset() {
  digitalWrite(EPD_RST, HIGH); delay(50);
  digitalWrite(EPD_RST, LOW);  delay(50);
  digitalWrite(EPD_RST, HIGH); delay(200);
}

// === V2 / SSD1683 init sequence ===
void initV2() {
  hardReset();
  delay(100);

  cmd(0x12);                        // SW reset
  delay(20);

  cmd(0x01); dat(0x2B); dat(0x01); dat(0x00);   // driver output control (300 lines)

  cmd(0x11); dat(0x03);             // data entry mode: X+ Y+

  cmd(0x44); dat(0x00); dat(0x31);  // RAM X start=0, end=49 (50 bytes = 400 px)

  cmd(0x45); dat(0x00); dat(0x00);  // RAM Y start=0
            dat(0x2B); dat(0x01);   // RAM Y end=299

  cmd(0x3C); dat(0x05);             // border waveform

  cmd(0x21); dat(0x00); dat(0x80);  // display update ctrl 1

  cmd(0x4E); dat(0x00);             // X counter = 0
  cmd(0x4F); dat(0x00); dat(0x00);  // Y counter = 0
}

void fillV2(uint8_t value) {
  cmd(0x24);                        // write RAM (BW)
  uint32_t bytes = (uint32_t)EPD_W * EPD_H / 8;
  for (uint32_t i = 0; i < bytes; i++) dat(value);

  cmd(0x22); dat(0xF7);             // display update ctrl 2 — full refresh
  cmd(0x20);                        // master activation — start refresh
  delay(4000);                      // wait for refresh
}

void setup() {
  Serial.begin(115200); delay(1000);
  Serial.println("=== V2 (SSD1683) init test ===");

  pinMode(EPD_CS, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_RST, OUTPUT);
  digitalWrite(EPD_CS, HIGH);

  hspi.begin(EPD_CLK, -1, EPD_MOSI, EPD_CS);
  hspi.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  initV2();
  Serial.println("Init done");

  Serial.println("Fill WHITE..."); fillV2(0xFF);
  Serial.println("Should be WHITE — wait 3s...");
  delay(3000);

  Serial.println("Fill BLACK..."); fillV2(0x00);
  Serial.println("Should be BLACK — wait 3s...");
  delay(3000);

  Serial.println("Fill WHITE..."); fillV2(0xFF);
  Serial.println("Should be WHITE again");
  delay(2000);

  // deep sleep
  cmd(0x10); dat(0x01);
  Serial.println("Done!");
}

void loop() {}