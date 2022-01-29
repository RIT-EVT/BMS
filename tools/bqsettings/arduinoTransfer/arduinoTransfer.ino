/**
 * This utility acts as a medium between a host system and the CANopen network. Specifically,
 * this tool allows users to transfer BQ settings to a BMS system.
 * 
 * The tool will accept the first two bytes as the number of BQ settings, then all other
 * bytes will be considered settings.
 * 
 * First the tool sends over the number of settings over CANopen, then the settings themselves
 * are transferred.
 * 
 * @author Collin Bolles
 */
#include <CAN.h>
#include <Wire.h>

#define NUM_BYTES_PER_SETTING 7
#define BMS_NODE_ID 0x05

#define I2C_ADDR 0x08

/**
 * Temporary storage for settings, once all bytes for a setting has been received, it can
 * be sent out
 */
uint8_t settingBuffer[NUM_BYTES_PER_SETTING];
uint8_t bufferIndex = 0;
uint16_t numSettings = 0;
uint16_t settingsSent = 0;

enum State {
  WAITING_FOR_NUM_SETTINGS_BYTE0,
  WAITING_FOR_NUM_SETTINGS_BYTE1,
  RECEIVING_SETTINGS,
};

enum State currentState = WAITING_FOR_NUM_SETTINGS_BYTE0;

#if 0
void onReceive(int packetSize) {
  // if(CAN.packetId() != 0x180)
  //  return;
  
  // received a packet
  Serial.print("Received ");

  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(CAN.packetId(), HEX);

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.print(packetSize);

    // only print packet data for non-RTR packets
    while (CAN.available()) {
      Serial.print(" ");
      Serial.print(CAN.read(), HEX);
    }
    Serial.println();
  }

  Serial.println();
}
#endif

void i2c_receive_handler(int count) {
  Serial.print("Request received: ");
  
  while (Wire.available()) {
    char data = Wire.read();
    Serial.print(data, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void i2c_request_handler() {
  
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Sender");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // Join I2C bus
  Wire.begin(I2C_ADDR);
  Wire.onReceive(i2c_receive_handler);
  Wire.onRequest(i2c_request_handler);

  // register the receive callback
  // CAN.onReceive(onReceive);
}

/**
 * Send the number of settings that will be transfered over CANopne
 */
void sendNumSettings() {
  CAN.beginPacket(0x600 + BMS_NODE_ID);  // SDO of the BMS
  CAN.write(0x2B);                       // Command: 001, n: 00, e: 1, s: 1
  CAN.write(0x00);                       // LSB of index
  CAN.write(0x21);                       // MSB of index
  CAN.write(0x00);                       // Subindex
  CAN.write(numSettings & 0xff);                // Number of settings that are used
  CAN.write(numSettings >> 8);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}

/**
 * Send the initialize download request over CANopen
 */
void initBQSettingDownload() {
  uint32_t totalBytes = numSettings * NUM_BYTES_PER_SETTING;
  
  // Initiate download
  CAN.beginPacket(0x600 + BMS_NODE_ID);  // SDO of the BMS
  CAN.write(0x21);                       // Command: 001, n: 00, e: 0, s: 1
  CAN.write(0x00);                       // Index LBS
  CAN.write(0x21);                       // Index MSB
  CAN.write(0x01);                       // Subindex
  CAN.write(totalBytes & 0xFF);          // LSB of data size
  CAN.write((totalBytes >> 8) & 0xFF);   // MSB of data size
  CAN.endPacket();
}

/**
 * Logic for sending a single BQ setting over CANopen.
 */
void sendBQSetting() {  
  uint8_t toggleBit = (settingsSent % 2) != 0;
  uint8_t isLastSegment = settingsSent == (numSettings - 1);
  uint8_t command = (toggleBit << 4) | (0 << 1) | isLastSegment;

  // command = settingsSent & 0xFF;
  
  CAN.beginPacket(0x600 + BMS_NODE_ID); // SDO of the BMS
  CAN.write(command);                   // Command: 000, t: toggles, n: 0, c: isLastSegment
  CAN.write(settingBuffer[0]);
  CAN.write(settingBuffer[1]);
  CAN.write(settingBuffer[2]);
  CAN.write(settingBuffer[3]);
  CAN.write(settingBuffer[4]);
  CAN.write(settingBuffer[5]);
  CAN.write(settingBuffer[6]);
  CAN.endPacket();
}

void loop() {
  uint8_t incomingByte = 0;
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    // Handle the first byte of the number of settings
    if (currentState == WAITING_FOR_NUM_SETTINGS_BYTE0) {
      numSettings = incomingByte;
      currentState = WAITING_FOR_NUM_SETTINGS_BYTE1;
    }
    // Handle the second byte of the number of settings
    else if(currentState == WAITING_FOR_NUM_SETTINGS_BYTE1) {
      numSettings |= ((uint16_t)incomingByte << 8);
      
      sendNumSettings();
      initBQSettingDownload();
      
      currentState = RECEIVING_SETTINGS;
    }
    // Handle recieving a byte of the setting
    else if(currentState == RECEIVING_SETTINGS) {
      settingBuffer[bufferIndex] = incomingByte;
      bufferIndex++;
  
      // Buffer filled, ready to send whole setting over CANopen
      if (bufferIndex >= NUM_BYTES_PER_SETTING) {
        sendBQSetting();
        bufferIndex = 0;
  
        settingsSent++;
      }
      // All settings sent, reset state machine
      if (settingsSent >= numSettings) {
        settingsSent = 0;
        bufferIndex = 0;
  
        currentState = WAITING_FOR_NUM_SETTINGS_BYTE0;
      }
    }
  }
}
