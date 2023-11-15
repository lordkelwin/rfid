#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN D0
#define SS_PIN D8

const char* ssid = "LORDKELWIN 5685";
const char* pass = "0123456789";
String dataRFID;
int i = 0;

AsyncWebServer server(80);
MFRC522 mfrc522(SS_PIN, RST_PIN);

struct roomTime {
  String room;
  String timeLength;
};

void setupWifi() {
  byte buffer[30];
  byte block;
  char name_arr[20];
  char pass_arr[20];
  byte len;

  Serial.println(ssid);
  Serial.println(pass);
  delay(100);
  //WiFi.config(local_IP, gateway, subnet);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  pinMode(D3, 1);
  setupWifi();
  
  AsyncElegantOTA.begin(&server);
  server.begin();

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("RFID initialized."));
}

void loop() {
  AsyncElegantOTA.loop();
  roomTime dataRF = readRFID();
  Serial.println(dataRF.room);
  Serial.println(dataRF.timeLength);
/*
  if (dataRF.room == "108") {
    digitalWrite(D3, 0);
    i = 0;
  } else {
    i++;
    if (i >= 5) {
      digitalWrite(D3, 1);
    }
  }
  Serial.println(i);
  */
}

roomTime readRFID() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  roomTime cardInfo;
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent() && !mfrc522.PICC_IsNewCardPresent()) {
    cardInfo.room = 0;
    cardInfo.timeLength = 0;
    delay(2000);
    return cardInfo;   
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //return "0";
  }

  //Serial.println(F("**Card Detected:**"));
  //-------------------------------------------
  //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex
  //-------------------------------------------

  //Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("Authentication failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    //return mfrc522.GetStatusCodeName(status);
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("Reading failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    //return mfrc522.GetStatusCodeName(status);
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] > 32)
    {
      //Serial.write(buffer1[i]);
      cardInfo.room += char(buffer1[i]);
    }
  }
  //Serial.print(" ");

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("Authentication failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    //return mfrc522.GetStatusCodeName(status);
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("Reading failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    //return mfrc522.GetStatusCodeName(status);
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    //Serial.write(buffer2[i] );
    if (buffer2[i] > 32) {
      cardInfo.timeLength += char(buffer2[i]);
    }    
  }

  //----------------------------------------
  
  //Serial.println(F("\n**End Reading**\n"));

  delay(1000); //change value if you want to read cards faster

  //mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return cardInfo;
}