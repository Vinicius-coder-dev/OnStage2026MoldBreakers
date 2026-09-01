#include "BLEDevice.h"
#include <ESP32Encoder.h>
#include <textparser.h>
#include <Wire.h>

#define bleServerName "Esp_princ"

TextParser parser {","};
ESP32Encoder encoders[2];
long lastPositions[2] = {0, 0};

bool positionChanged = false;
long currentPositions[2];
static BLEUUID SERVICE_UUID("3127cf94-c264-4df7-a1fe-1c95ead9c788");
static BLEUUID Robot1CharacteristicsUUID("ff349e4d-b4a3-4694-b90a-fcee077fa0a1");

static BLERemoteCharacteristic* pRemoteCharacteristic;

static boolean doConnect = false;
static boolean connected = false;

static BLEAddress *pServerAddress;

static BLERemoteCharacteristic* Robot1Characteristic = NULL;

char* Robot1Char;

boolean newRobot1 = false;


bool debug_switch = true;
int debug_led = true;
int x1,x2;
#define RXD2 16
#define TXD2 17


uint16_t values[4];  // sempre dividir por 100
uint16_t values_checking[4];
String command_detected;
String command_toSend;
String bluetooth_command;
String last_command;
String cmd;
String last_cmd;

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
 
static void Robot1NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  Robot1Char = (char*)pData;
  newRobot1 = true;
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
  
  Robot1Characteristic = pRemoteService->getCharacteristic(Robot1CharacteristicsUUID);

  if (Robot1Characteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  return true;
}
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

DCMotor MotorWheelRight;
DCMotor MotorWheelLeft;
DCMotor MotorArmLeft;
DCMotor MotorArmRight;



bool is_digits(const std::string& str) {
  return std::all_of(str.begin(), str.end(), ::isdigit);  // C++11
}
  
void setup() {
  values[1] = 100000;
  values[2] = 0;
  values[3] = 0;
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  delay(1500);
  Serial.println("EE");
  Serial.println("Starting Arduino BLE Client application...");
  Serial.println("AEFFDD");//Scraping my face in the sky
  pinMode(2,OUTPUT);
  digitalWrite(2,0);
  MotorWheelRight.Pinout(19, 21);
  MotorWheelLeft.Pinout(18, 5);
  MotorArmLeft.Pinout(22,23);
  pinMode(13, OUTPUT); 
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoders[0].attachHalfQuad(33, 32);
  encoders[1].attachHalfQuad(27, 26);
  digitalWrite(13,0);
  MotorWheelLeft.Speed(200);
  MotorWheelRight.Speed(200);
  MotorWheelLeft.Stop();

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(20);
}
void WalkTo(int distance_max, int min_velocity ,int max_velocity) {
  Serial.println("Ad");
  values[1] = 80000;
  while (values[1] / 100 > distance_max){
    
  SerialReading();
  Following();
    x1 = map(values[2],2000, 5000, min_velocity, max_velocity);
    x2 = map(values[3],2000, 5000, min_velocity, max_velocity);
    Serial.println("Keep");
    Serial.println(x1);
    Serial.println(x2);
    MotorWheelRight.Speed(x2);
    MotorWheelRight.Forward();
    MotorWheelLeft.Speed(x1-30);
    MotorWheelLeft.Backward();
  
  
  }

    Serial.println(values[1]/100);
    digitalWrite(2,0);
    MotorWheelRight.Speed(0);
    MotorWheelRight.Stop();
    MotorWheelLeft.Speed(0);
    MotorWheelLeft.Stop();
    bluetooth_command = "";
    values[1] = 10000;

}
void sendCommand(String command){

  if (last_command != command){
    command_toSend = command;
    last_command = command;
  }
}
void encoderReading(){
    for (int i = 0; i < 2; i++) {
    currentPositions[i] = encoders[i].getCount();
    if (currentPositions[i] != lastPositions[i]) {
      positionChanged = true;
    }
  }

  if (positionChanged) {
    for (int i = 0; i < 2; i++) {
      lastPositions[i] = currentPositions[i];
      Serial.println(lastPositions[i]);
    }
  }
}
int x = 0;
void Following(){
  if (command_detected != "")
  {
    Serial.println(command_detected);
    parser.parseLine(command_detected.c_str(), values_checking);
    if ( values_checking[3] > 500 && values_checking[3] < 8000 && values_checking[0] == 0){
      for(int i = 0; i <4; i++){
        values[i] = values_checking[i];
      }
    }
    command_detected = "";
  }
}

void SerialReading(){

  if (Serial.available()) {
    command_toSend = Serial.readStringUntil('\n');
  }
  if (Serial2.available() > 0) {
    command_detected = Serial2.readStringUntil('\n');

  }
  }

void loop() {
  
  
  SerialReading();
  Following();
  encoderReading();
   if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      Robot1Characteristic->getDescriptor(BLEUUID((uint16_t)0x2901));
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  if(Robot1Characteristic !=NULL){

    last_cmd = Robot1Characteristic->readValue();

    if(last_cmd != cmd){
      digitalWrite(2,1);
      cmd = last_cmd;
      bluetooth_command = last_cmd;

    }
  } 
  if (command_toSend != "") {
    Serial2.println(command_toSend);
    Serial.println(command_toSend);
    command_toSend = "";
  }

  if (bluetooth_command != "") {
    Serial.println(bluetooth_command);
    if (bluetooth_command == "1") {
        sendCommand("0");
        WalkTo(15,150,235);
    }
    if (bluetooth_command == "2") {
        //delay(3000);
        sendCommand("1");
        WalkTo(30,120,160);
    }
    if(bluetooth_command == "3") {


      for (int i = 0; i < 2; i++) { 
        encoders[i].clearCount();
      }
      encoderReading();
      while(lastPositions[0] > -3900) {
        encoderReading();
        MotorWheelRight.Speed(160);
        MotorWheelRight.Backward();
        MotorWheelLeft.Speed(130);
        MotorWheelLeft.Backward();
      }
        MotorWheelLeft.Stop();
        MotorWheelRight.Stop();
        sendCommand("2");
        values[1] = 7000;
        delay(1000);
        WalkTo(30,120,180);
        Serial.println(values[1]);
        bluetooth_command = "";
        delay(1000);
        MotorWheelRight.Stop();
        MotorWheelLeft.Stop();
        MotorWheelRight.Speed(130);
        MotorWheelLeft.Speed(100);
        MotorWheelLeft.Backward();
        MotorWheelRight.Forward();
        delay(2000);
        MotorWheelRight.Stop();
        MotorWheelLeft.Stop();
        delay(6000);
        MotorWheelRight.Speed(130);
        MotorWheelLeft.Speed(100);
        MotorWheelLeft.Forward();
        MotorWheelRight.Backward();
        delay(2000);
        MotorWheelRight.Stop();
        MotorWheelLeft.Stop();
          }
    
    if(bluetooth_command == "4")
    {
        MotorArmLeft.Speed(150);//arrumar valores
        MotorArmLeft.Backward();
        delay(6000);
        MotorArmLeft.Stop();
        digitalWrite(13,1);
        delay(1000);
        MotorArmLeft.Speed(150);
        MotorArmLeft.Forward();
        delay(5000);
        MotorArmLeft.Stop();
    }
    if(bluetooth_command == "5"){
      for (int i = 0; i < 2; i++) { 
        encoders[i].clearCount();
      }
      encoderReading();
        while(lastPositions[0] < -89) {
          encoderReading();
          MotorWheelRight.Speed(230);
          MotorWheelRight.Backward();
          MotorWheelLeft.Speed(230);
          MotorWheelLeft.Backward();
        }
        MotorWheelRight.Stop();
        MotorWheelLeft.Stop();
    }
    if(bluetooth_command == "6"){
        digitalWrite(13,1);
        MotorArmLeft.Speed(150);
        MotorArmLeft.Forward();
        delay(6000);
        MotorArmLeft.Stop();
        digitalWrite(13,0);
        delay(1000);
        MotorArmLeft.Speed(150);//arrumar valores
        MotorArmLeft.Backward();
        delay(5000);
        MotorArmLeft.Stop();
        
    }
  
  
}
  



