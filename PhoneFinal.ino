#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

#define bleServerName "Esp_princ"

#define SERVICE_UUID "ac26250d-e29a-4dea-aec7-40d9abc69173"
#define PICO_SERVICE_UUID "72eb11d6-b7ae-45a6-ba1e-8ce8c7dbf8f0"
#define ROBOT_1_SERVICE_UUID "3127cf94-c264-4df7-a1fe-1c95ead9c788"
#define ROBOT_2_SERVICE_UUID "0c5161b0-66a3-4445-8bef-be2312f751a9"

BLECharacteristic bmeEspServerCharacteristics("ac26250d-e29a-4dea-aec7-40d9abc69173",19);
BLEDescriptor bmeEspServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmePicoServerCharacteristics("72eb11d6-b7ae-45a6-ba1e-8ce8c7dbf8f0",19);
BLEDescriptor bmePicoServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmeRobot1ServerCharacteristics("ff349e4d-b4a3-4694-b90a-fcee077fa0a1",19);
BLEDescriptor bmeRobot1ServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmeRobot2ServerCharacteristics("d9335013-af23-4135-8576-20f37bd4c6ce",19);
BLEDescriptor bmeRobot2ServerDescriptor(BLEUUID((uint16_t)0x2901));

bool deviceConnected = false;
volatile int pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
unsigned long currentTime = 0;
int currentDigit = 0;
bool digitComplete = false;
const int digitTimeout = 150;

  int MagnetPin = 7;
  int ButtonPin = 8;
  bool ButtonState;

String commandReceived;
String commandSended;
String lastCommandSended;
String lastCommandReceived;
int i = 1;

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


String Bitto_detected;
void countPulse() {
  if (currentTime - lastPulseTime > 75) {
    pulseCount++;
    lastPulseTime = millis();
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    pServer->getAdvertising()->start();
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pServer->getAdvertising()->start();
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks{
  void onStatus(BLECharacteristic *pCharacteristic, Status s, uint32_t code){
    if(s==8){
      Serial.println("A visitor");
      bmeEspServerCharacteristics.indicate();
      bmePicoServerCharacteristics.indicate();
      bmeRobot1ServerCharacteristics.indicate();
      bmeRobot2ServerCharacteristics.indicate();
    }
  }
};

void SendCommandPico(int command){
    commandSended = String(command);
    lastCommandSended = commandSended;
    bmePicoServerCharacteristics.setValue(commandSended);
    bmePicoServerCharacteristics.indicate();
    bmePicoServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
}
void SendCommandRobot1(int command){
    commandSended = String(command);
    lastCommandSended = commandSended;
    bmeRobot1ServerCharacteristics.setValue(commandSended);
    bmeRobot1ServerCharacteristics.indicate();
    bmeRobot1ServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
}

void SendCommandRobot2(int command){
    commandSended = String(command);
    lastCommandSended = commandSended;
    bmeRobot2ServerCharacteristics.setValue(commandSended);
    bmeRobot2ServerCharacteristics.indicate();
    bmeRobot2ServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
}
void setup() {
  Serial.begin(115200);
  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *bmeServiceEsp = pServer->createService(SERVICE_UUID);
  BLEService *bmeServicePico = pServer->createService(PICO_SERVICE_UUID);
  BLEService *bmeServiceRobot1 = pServer->createService(ROBOT_1_SERVICE_UUID);
  BLEService *bmeServiceRobot2 = pServer->createService(ROBOT_2_SERVICE_UUID);

  bmeServiceEsp->addCharacteristic(&bmeEspServerCharacteristics);
  bmeEspServerDescriptor.setValue("Bitto factory 2000");
  bmeEspServerCharacteristics.addDescriptor(new BLE2902());

  bmeServicePico->addCharacteristic(&bmePicoServerCharacteristics);
  bmePicoServerDescriptor.setValue("Vitor factory 2000");
  bmePicoServerCharacteristics.addDescriptor(new BLE2902());

  bmeServiceRobot1->addCharacteristic(&bmeRobot1ServerCharacteristics);
  bmeRobot1ServerDescriptor.setValue("Leandro factory 2000");
  bmeRobot1ServerCharacteristics.addDescriptor(new BLE2902());

  bmeServiceRobot2->addCharacteristic(&bmeRobot2ServerCharacteristics);
  bmeRobot2ServerDescriptor.setValue("Rosca factory 2000");
  bmeRobot2ServerCharacteristics.addDescriptor(new BLE2902());

  bmeServiceEsp->start();
  bmeServicePico->start();
  bmeServiceRobot1->start();
  bmeServiceRobot2->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  

  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  bmeEspServerCharacteristics.setValue("Esp32");
  bmePicoServerCharacteristics.setValue("Wrong Bitto");
  bmeRobot1ServerCharacteristics.setValue("Virtual insanity");
  bmeRobot2ServerCharacteristics.setValue("Reach to heaven and fall again");
  pinMode(MagnetPin, OUTPUT);   // Magnet send
  pinMode(ButtonPin, INPUT_PULLUP);    // Button(inv) "gancho"
  digitalWrite(MagnetPin, LOW);
  delay(1000);
  Serial.println("teste");
  pinMode(1, INPUT);
  attachInterrupt(digitalPinToInterrupt(1), countPulse, FALLING);  //WE ALL FALL DOWN
  for(int i = 0; i < 3; i++){
    digitalWrite(MagnetPin, HIGH);
    delay(20);
    digitalWrite(MagnetPin, LOW);
    delay(20);
    }
}

void loop() {
  
  if (deviceConnected) {
    // PHONE ACTION
    //a = 1;
    // SERVER ACTION
    //e = 2;
      currentTime = millis();
  if (pulseCount > 0 && (currentTime - lastPulseTime > digitTimeout)) {
    currentDigit = pulseCount - 1;
    if (currentDigit == 0) currentDigit = 10;
    Serial.println(currentDigit);


    pulseCount = 0;
  }
  if (currentDigit == 1){
    currentDigit = -1;
    delay(7000);
    SendCommandPico(21);
    ButtonState = digitalRead(ButtonPin);
    while(ButtonState == HIGH){   
      digitalWrite(MagnetPin, HIGH);
      delay(20);
      digitalWrite(MagnetPin, LOW);
      delay(20);
      ButtonState = digitalRead(ButtonPin);
   }
    SendCommandPico(26);
    while(ButtonState == LOW){
      ButtonState = digitalRead(ButtonPin);
    }
    
    SendCommandPico(22);
    delay(14000);
    SendCommandRobot1(2);
    SendCommandRobot2(1);
    delay(12800);
    SendCommandPico(27);
    delay(100);
  }
  if (currentDigit == 2){
    currentDigit = -1;
    SendCommandRobot1(3);
    SendCommandRobot2(2);
    BLEDevice::init(bleServerName);
  }
  if (currentDigit == 3){
    currentDigit = -1;
    SendCommandPico(23);
    BLEDevice::init(bleServerName);
  }
  if (currentDigit == 4){
    currentDigit = -1;
    SendCommandPico(24);
    SendCommandRobot1(5);
    SendCommandRobot2(4);
    BLEDevice::init(bleServerName);
  }
  
  if (currentDigit == 5){
    currentDigit = -1;
    SendCommandPico(25);
    SendCommandRobot1(6);
    BLEDevice::init(bleServerName);
  }
  if (currentDigit == 8){
    currentDigit = -1;
    SendCommandRobot1(1);
  }
      
    if (bmeRobot1ServerCharacteristics.getValue() != ""){
      if (bmeRobot1ServerCharacteristics.getValue() == "Ready"){
        SendCommandRobot2(2);
        SendCommandRobot1(4);
        Serial.println("Years");
        Serial.println(bmeRobot1ServerCharacteristics.getValue());
      } 
    }

}
}
