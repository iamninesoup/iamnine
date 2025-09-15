#include <MIDIUSB.h>

// 버튼 핀 & 노트 설정
const uint8_t buttonPins[2] = {2, 9};   // D2, D9
const uint8_t notes[2] = {60, 61};      // C4, C#4

bool lastState[2] = {HIGH, HIGH};  // HIGH = not pressed

void setup() {
  for (int i = 0; i < 2; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); 
  }
}

void loop() {
  for (int i = 0; i < 2; i++) {
    bool state = digitalRead(buttonPins[i]);

    // 버튼 눌림 감지
    if (state != lastState[i]) {
      if (state == LOW) {
        noteOn(0, notes[i], 100); // 눌렀을 때 Note On
      } else {
        noteOff(0, notes[i], 0);  // 뗐을 때 Note Off
      }
      MidiUSB.flush();
      lastState[i] = state;
    }
  }
  delay(5);
}

// MIDI Helper Functions
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}