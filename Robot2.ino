#include "BLEDevice.h"
#include <Wire.h>
#include <ESP32Servo.h> 

#define bleServerName "Esp_princ"

static BLEUUID SERVICE_UUID("0c5161b0-66a3-4445-8bef-be2312f751a9");
static BLEUUID Robot2CharacteristicsUUID("d9335013-af23-4135-8576-20f37bd4c6ce");

static BLERemoteCharacteristic* pRemoteCharacteristic;

static boolean doConnect = false;
static boolean connected = false;

static BLEAddress *pServerAddress;

static BLERemoteCharacteristic* Robot2Characteristic = NULL;

char* Robot2Char;

boolean newRobot2 = false;

String command_detected;
String command_toSend;
String bluetooth_command;
String last_command;
String cmd;
String last_cmd;

Servo servoHU = Servo();
Servo servoHV = Servo();
const int pinServoHU = 12;
const int pinServoHV = 14;
const int pinMagnet = 32;

const float Angle_A = 10;  
const float Angle_B = 150;  
const float Angle_C = 20;   
const float Angle_D = 50;

bool invertServoHU = false;
bool invertServoHV = false;
int calibU = 0;
int calibV = 0;

class DCMotor {
private:
  int spd = 255;
  int pin1;
  int pin2;

public:

  void Pinout(int in1, int in2) {
    pin1 = in1;
    pin2 = in2;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
  }
  void Speed(int in1) {
    spd = in1;
  }
  void Forward() {
    analogWrite(pin1, spd);
    analogWrite(pin2, 0);
  }
  void Backward() {
    analogWrite(pin1, 0);
    analogWrite(pin2, spd);
  }
  void Stop() {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  }
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};

static void Robot2NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  Robot2Char = (char*)pData;
  newRobot2 = true;
}

void moveHead(int yaw, int pitch) {
  unsigned long currentMillis = millis();

  int UTarget = 90 + yaw + pitch;
  int VTarget = 90 + yaw - pitch;

  if (invertServoHU) {
    UTarget = 180 - UTarget;
  }
  if (invertServoHV) {
    VTarget = 180 - VTarget;
  }

  UTarget += calibU;
  VTarget += calibV;

  UTarget = constrain(UTarget, 0, 180);
  VTarget = constrain(VTarget, 0, 180);

  servoHU.write(UTarget);
  servoHV.write(VTarget);
}

bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(SERVICE_UUID.toString().c_str());
    return (false);
  }
  
  Robot2Characteristic = pRemoteService->getCharacteristic(Robot2CharacteristicsUUID);

  if (Robot2Characteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  return true;
}
void Blink(){
  for(int i = 0;i < 10;i++){
    
  digitalWrite(13,1);
  delay(300);
  digitalWrite(13,0);
  delay(300);
  }
}
DCMotor MotorArm;
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("EE");
  Serial.println("Starting Arduino BLE Client application...");
  Serial.println("AEFFDD");
  MotorArm.Pinout(27, 28);
  pinMode(pinMagnet, OUTPUT);
  digitalWrite(pinMagnet, HIGH);
  pinMode(26,OUTPUT);
  digitalWrite(26,0);
  servoHU.attach(pinServoHU);
  servoHV.attach(pinServoHV);
  pinMode(13,OUTPUT);
  delay(2000);
  pinMode(2,OUTPUT);
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(20);

}

void loop() {
   if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      Robot2Characteristic->getDescriptor(BLEUUID((uint16_t)0x2901));
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  
  if(Robot2Characteristic !=NULL){
    digitalWrite(2,1);
    last_cmd = Robot2Characteristic->readValue();

    if(last_cmd != cmd){
      cmd = last_cmd;
      bluetooth_command = last_cmd;
    }
  } 


  if (bluetooth_command != "") {
    Serial.println(bluetooth_command);
    if (bluetooth_command == "1") {
      for(int i = 0; i < 3; i++)
    {
      Blink();
      moveHead(-15, 0);
      delay(1500);
      moveHead(0, 0);
      delay(1500);
      moveHead(15, 0);
      delay(1500);

      }
      bluetooth_command = "Bitto";
    }
    if (bluetooth_command == "2") {
      delay(10000);
      MotorArm.Speed(200);
      MotorArm.Backward();
      delay(3000);
      MotorArm.Stop();
      digitalWrite(pinMagnet,0);

    }
    if(bluetooth_command == "3") {
      delay(6500);
      digitalWrite(pinMagnet,LOW);
    }
    if(bluetooth_command == "4"){
      moveHead(0,20);

    }
    if(bluetooth_command == "5"){
      MotorArm.Speed(200);
      MotorArm.Forward();
      delay(6000);
      MotorArm.Stop();
      digitalWrite(pinMagnet,1);
      delay(1500);
      MotorArm.Speed(200);
      MotorArm.Backward();
      delay(6000);
      MotorArm.Stop();
    }
  }      
}
