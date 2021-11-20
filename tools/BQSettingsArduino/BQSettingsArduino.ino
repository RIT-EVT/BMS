/**
 * This provides an interface for sending the BQ chip settings out over CAN.
 * The sending of the settings takes place over a segmented SDO request to
 * the BMS. The settings themselves are stored as an array of settings in
 * a format described in the BMS read-the-docs.
 * 
 * @author Collin Bolles
 */
#include <CAN.h>

#define MAX_DATA_BYTES 4
#define BMS_NODE_ID 0x05

/**
 * Represents the data that will be stored in a BQSetting.
 * 
 * @controlByte The byte that determines the type of command, and the amount
 *  of data contained
 * @address The address in memory that the setting is targeting
 * @data The data (may not be used) that is going to be sent over as part of
 *  the command. The data is LITTLE ENDIAN
 */
struct BQSetting {
  uint8_t controlByte;
  uint16_t address;
  uint8_t data[MAX_DATA_BYTES];
};

/**
 * TEMPORARY: Settings to send to the BMS for testing. Later this should be
 * handled via sending of these settings over the serial interface
 */
struct BQSetting settings[] = {
  // Direct commands
  { 0x04, 0x0001, { 0xAA, 0x00, 0x00, 0x00 }}, // Direct command to address 0x01 with data 0xAA
  { 0x04, 0x0002, { 0xBB, 0x00, 0x00, 0x00 }}, // Direct command to address 0x02 with data 0xBB
  { 0x04, 0x0003, { 0xCC, 0x00, 0x00, 0x00 }}, // Direct command to address 0x03 with data 0xCC
  { 0x04, 0x0004, { 0xDD, 0x00, 0x00, 0x00 }}, // Direct command to address 0x04 with data 0xDD
  { 0x04, 0x0005, { 0xEE, 0x00, 0x00, 0x00 }}, // Direct command to address 0x05 with data 0xEE

  // Subcommands
  { 0x11, 0x0000, { 0x01, 0x02, 0x03, 0x04 }}, // Subcommand to address 0x0000 with data 0x04030201
  { 0x11, 0x0102, { 0x05, 0x06, 0x07, 0x08 }}, // Subcommand to address 0x0102 with data 0x08070605
  { 0x11, 0x0304, { 0x0A, 0x0B, 0x0C, 0x0D }}, // Subcommand to address 0x0304 with data 0x0D0C0B0A
  { 0x01, 0x0506, { 0x00, 0x00, 0x00, 0x00 }}, // Subcommand to address 0x0506 with no data
  { 0x01, 0x0708, { 0x00, 0x00, 0x00, 0x00 }}, // Subcommand to address 0x0708 with no data

  // RAM
  { 0x12, 0x0000, { 0x0A, 0x0B, 0x0C, 0x0D }}, // RAM request to address 0x0000 with data 0x0D0C0B0A
  { 0x12, 0x0102, { 0x08, 0x07, 0x06, 0x05 }}, // RAM request to address 0x0102 with data 0x05060708
};
uint8_t numSettings = sizeof(settings) / sizeof(settings[0]);
uint16_t totalBytes = numSettings * 7;

/** Used to represent if the user typed in a character to send the bq setting stream over CAN */
bool shouldSend = false;

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

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Sender");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // register the receive callback
  CAN.onReceive(onReceive);
}

/**
 * Logic for sending the BQSetting data over CAN via the CANopen
 * segmented SDO request.
 */
void sendBQSettings() {
   Serial.print("Num Settings: ");
   Serial.println(numSettings);

   Serial.print("Total Bytes: ");
   Serial.println(totalBytes);
   // Set the new size
   CAN.beginPacket(0x600 + BMS_NODE_ID);  // SDO of the BMS
   CAN.write(0x2B);                       // Command: 001, n: 00, e: 1, s: 1
   CAN.write(0x00);                       // LSB of index
   CAN.write(0x21);                       // MSB of index
   CAN.write(0x00);                       // Subindex
   CAN.write(numSettings);                // Number of settings that are used
   CAN.write(0x00);
   CAN.write(0x00);
   CAN.write(0x00);
   CAN.endPacket();

  
   // Initiate download
   CAN.beginPacket(0x600 + BMS_NODE_ID);  // SDO of the BMS
   CAN.write(0x21);                       // Command: 001, n: 00, e: 0, s: 1
   CAN.write(0x00);                       // Index LBS
   CAN.write(0x21);                       // Index MSB
   CAN.write(0x01);                       // Subindex
   CAN.write(totalBytes & 0xFF);          // LSB of data size
   CAN.write((totalBytes >> 8) & 0xFF);   // MSB of data size
   CAN.endPacket();

   delay(10);

   // Send each piece of data in its own packet
   for(int i = 0; i < numSettings; i++) {
      uint8_t toggleBit = i % 2 != 0;
      uint8_t isLastSegment = i == numSettings - 1;
      uint8_t command = (toggleBit << 4) | (0 << 1) | isLastSegment;
      struct BQSetting* setting = &settings[i];
      
      CAN.beginPacket(0x600 + BMS_NODE_ID); // SDO of the BMS
      CAN.write(command);                   // Command: 000, t: toggles, n: 0, c: isLastSegment
      CAN.write(setting->controlByte);
      CAN.write(setting->address & 0xFF);
      CAN.write((setting->address >> 8) & 0xFF);
      CAN.write(setting->data[0]);
      CAN.write(setting->data[1]);
      CAN.write(setting->data[2]);
      CAN.write(setting->data[3]);
      CAN.endPacket();
      delay(10);
   }
}

void loop() {

  if (shouldSend) {
    sendBQSettings();
    shouldSend = false;
    Serial.println("Press any key to send the CAN messages...");
  }

  if (Serial.available() > 0) {
    Serial.read();
    Serial.read();
    shouldSend = true;
  }



  delay(500);
  // Serial.println("done");
}
