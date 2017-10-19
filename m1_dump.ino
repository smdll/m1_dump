/**
   RST/Reset   RST          D9
   SPI SS      SDA(SS)      D10
   SPI MOSI    MOSI         D11
   SPI MISO    MISO         D12
   SPI SCK     SCK          D13
*/

#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

byte def_keys[3][6] = {{0x79, 0x07, 0x8e, 0x0a, 0xb5, 0x0a}, {0x78, 0xd9, 0xa4, 0xdc, 0xdd, 0xf5}, {0xb4, 0xfd, 0xd9, 0x8a, 0x9d, 0xe1}};

void setup() {
  SPI.begin();        // Init SPI bus
  Serial.begin(115200);
  for (byte i = 3; i <= 6; i++)
    pinMode(i, OUTPUT);
  digitalWrite(3, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

void loop() {
  digitalWrite(4, HIGH);
  status_Wait();
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println('1');
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  Serial.println('2');
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println('3');
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  for (byte sector = 0; sector < 3; sector ++) {
    for (byte i = 0; i < 6; i ++) {
      key.keyByte[i] = def_keys[sector][i];
    }
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector * 4 + 3, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Auth error");
      status_Fail();
      return;
    }
    Serial.print("Sector ");
    Serial.println(sector);
    // Read data from the blocks
    for (byte i = 0; i < 3; i ++) {
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(sector * 4 + i, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.println("Read error");
        status_Fail();
        return;
      }
      status_OK();
      dump_byte_array(buffer, 16);
      Serial.println();
    }
    Serial.println();
  }
  Serial.println('4');
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(500);
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i ++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void status_OK() {
  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
}

void status_Wait() {
  digitalWrite(3, LOW);
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
}

void status_Fail() {
  digitalWrite(3, HIGH);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}
