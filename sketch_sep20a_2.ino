#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE MIDI 표준 UUID
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// 버튼 핀(D42~D35 순서)과 드럼용 MIDI 노트(D37~D42 + D1~D2)
const byte BUTTON_PINS[8] = {42, 41, 40, 39, 38, 37, 36, 35};
const byte NOTES[8]       = {0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32}; 
// 예시: 킥(D37)=0x24, 스네어(D38)=0x26, 하이햇 클로즈(D39)=0x28 등

bool lastStates[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long lastTimeChanged[8] = {0,0,0,0,0,0,0,0};
unsigned long debounceDuration = 10;

// MIDI 패킷 (5바이트 고정)
uint8_t midiPacket[5] = {0x80, 0x80, 0x00, 0x00, 0x00};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; }
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

void setup() {
  Serial.begin(115200);

  for (int i=0; i<8; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  BLEDevice::init("ESP32-MIDI");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
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

void handleButton(int index) {
  byte state = digitalRead(BUTTON_PINS[index]);
  if (millis() - lastTimeChanged[index] >= debounceDuration) {
    if (state != lastStates[index]) {
      lastTimeChanged[index] = millis();
      lastStates[index] = state;

      if (state == LOW) {
        midiPacket[2] = 0x90; // Note On
        midiPacket[3] = NOTES[index];
        midiPacket[4] = 127;
      } else {
        midiPacket[2] = 0x80; // Note Off
        midiPacket[3] = NOTES[index];
        midiPacket[4] = 0;
      }
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
    }
  }
}

void loop() {
  if (!deviceConnected) return;

  for (int i=0; i<8; i++) {
    handleButton(i);
  }
}
