#include "MIDIUSB.h"

String command_detected;
String command_toSend;

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
  //Serial.println("on");
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
  //Serial.println("off");
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(8, INPUT);
  noteOn(0, 36, 127);
}

void loop() {
  
  if (Serial1.available() > 0) {
    command_detected = Serial1.readStringUntil('\n');
    Serial.println(command_detected);
  }



  if(command_detected == "123")
  {
    command_detected = "";
    noteOn(0, 36, 127);
    MidiUSB.flush();
    delay(150);
    noteOn(0, 36, 0);
    MidiUSB.flush();
    delay(300);
  }




  bool bs;
  bs = digitalRead(8);
  if (bs == HIGH) {
    noteOn(0, 36, 127);
    MidiUSB.flush();
    delay(150);
    noteOn(0, 36, 0);
    MidiUSB.flush();
    delay(300);
  }
  /*else{
  noteOff(0,36, 127);
  MidiUSB.flush();
  }*/
}