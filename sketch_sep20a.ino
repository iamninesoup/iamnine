#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE MIDI 표준 UUID
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

unsigned long debounceDuration = 10; // millis
unsigned long lastTimeButton1StateChanged = 0;
unsigned long lastTimeButton2StateChanged = 0;
unsigned long lastTimeButton3StateChanged = 0;
unsigned long lastTimeButton4StateChanged = 0;
unsigned long lastTimeButton5StateChanged = 0;
unsigned long lastTimeButton6StateChanged = 0;
unsigned long lastTimeButton7StateChanged = 0;
unsigned long lastTimeButton8StateChanged = 0;

byte lastState1 = LOW;
byte lastState2 = LOW;
byte lastState3 = LOW;
byte lastState4 = LOW;
byte lastState5 = LOW;
byte lastState6 = LOW;
byte lastState7 = LOW;
byte lastState8 = LOW;

// 드럼용 MIDI 노트 (GarageBand용)
byte note1  = 0x2E; // D42
byte note2  = 0x2C; // D41
byte note3  = 0x2A; // D40
byte note4  = 0x28; // D39
byte note5  = 0x26; // D38
byte note6  = 0x24; // D37
byte note7  = 0x22; // D36
byte note8  = 0x20; // D35

uint8_t midiPacket[] = {0x80, 0x80, 0x00, 0x3c, 0x00};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; };
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; };
};

void setup() {
  Serial.begin(115200);

  pinMode(42, INPUT_PULLUP);
  pinMode(41, INPUT_PULLUP);
  pinMode(40, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(38, INPUT_PULLUP);
  pinMode(37, INPUT_PULLUP);
  pinMode(36, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);

  BLEDevice::init("Blue ESP32");
    
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

  pCharacteristic = pService->createCharacteristic(
    BLEUUID(CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ   |
    BLECharacteristic::PROPERTY_WRITE  |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE_NR
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();
}

void handleButton(byte buttonPin, byte note, byte &lastState, unsigned long &lastTimeChanged) {
  byte state = digitalRead(buttonPin);
  if (millis() - lastTimeChanged >= debounceDuration) {
    if (state != lastState) {
      lastTimeChanged = millis();
      lastState = state;
      if (state == LOW) {
        midiPacket[2] = 0x90;  // note on
        midiPacket[3] = note;
        midiPacket[4] = 127;
      } else {
        midiPacket[2] = 0x80;  // note off
        midiPacket[3] = note;
        midiPacket[4] = 0;
      }
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
    }
  }
}

void loop() {
  if (deviceConnected) {
    handleButton(42, note1, lastState1, lastTimeButton1StateChanged);
    handleButton(41, note2, lastState2, lastTimeButton2StateChanged);
    handleButton(40, note3, lastState3, lastTimeButton3StateChanged);
    handleButton(39, note4, lastState4, lastTimeButton4StateChanged);
    handleButton(38, note5, lastState5, lastTimeButton5StateChanged);
    handleButton(37, note6, lastState6, lastTimeButton6StateChanged);
    handleButton(36, note7, lastState7, lastTimeButton7StateChanged);
    handleButton(35, note8, lastState8, lastTimeButton8StateChanged);
  }
}
