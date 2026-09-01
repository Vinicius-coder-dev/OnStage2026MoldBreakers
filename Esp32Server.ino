#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

#define bleServerName "Esp_princ"


#define SERVICE_UUID "ac26250d-e29a-4dea-aec7-40d9abc69173"
#define PHONE_SERVICE_UUID "ac26250d-e29a-4dea-aec7-40d9abc69173"
#define PICO_SERVICE_UUID "72eb11d6-b7ae-45a6-ba1e-8ce8c7dbf8f0"
#define ROBOT_1_SERVICE_UUID "3127cf94-c264-4df7-a1fe-1c95ead9c788"
#define ROBOT_2_SERVICE_UUID "64825a3a-6c5d-4af1-910a-a036cb5a1b3d"

BLECharacteristic bmePhoneServerCharacteristics("f2dc9fb3-135c-4abd-9e50-d747b37c125e",19);
BLEDescriptor bmePhoneServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmeEspServerCharacteristics("ac26250d-e29a-4dea-aec7-40d9abc69173",19);
BLEDescriptor bmeEspServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmePicoServerCharacteristics("72eb11d6-b7ae-45a6-ba1e-8ce8c7dbf8f0",19);
BLEDescriptor bmePicoServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmeRobot1ServerCharacteristics("ff349e4d-b4a3-4694-b90a-fcee077fa0a1",19);
BLEDescriptor bmeRobot1ServerDescriptor(BLEUUID((uint16_t)0x2901));
BLECharacteristic bmeRobot2ServerCharacteristics("e61c15ca-86ba-442b-8f2f-41035ff6925c",19);
BLEDescriptor bmeRobot2ServerDescriptor(BLEUUID((uint16_t)0x2901));
bool deviceConnected = false;

int i = 1;

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
String Bitto_detected;
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
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
      bmePhoneServerCharacteristics.indicate();
      bmePicoServerCharacteristics.indicate();
      bmeRobot1ServerCharacteristics.indicate();
      bmeRobot2ServerCharacteristics.indicate();
    }
  }
};


void setup() {
  Serial.begin(115200);
  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *bmeServiceEsp = pServer->createService(SERVICE_UUID);
  BLEService *bmeServicePhone = pServer->createService(PHONE_SERVICE_UUID);
  BLEService *bmeServicePico = pServer->createService(PICO_SERVICE_UUID);
  BLEService *bmeServiceRobot1 = pServer->createService(ROBOT_1_SERVICE_UUID);
  BLEService *bmeServiceRobot2 = pServer->createService(ROBOT_2_SERVICE_UUID);

  bmeServicePhone->addCharacteristic(&bmePhoneServerCharacteristics);
  bmePhoneServerDescriptor.setValue("Olivari factory 2000");
  bmePhoneServerCharacteristics.addDescriptor(new BLE2902());

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
  bmeServicePhone->start();
  bmeServicePico->start();
  bmeServiceRobot1->start();
  bmeServiceRobot2->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  

  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  bmeEspServerCharacteristics.setValue("Esp32");
  bmePhoneServerCharacteristics.setValue("Wrong time");
  bmePicoServerCharacteristics.setValue("Wrong Bitto");
  bmeRobot1ServerCharacteristics.setValue("Virtual insanity");
}

void loop() {
  if (deviceConnected) {
    Serial.println(deviceConnected);
    // PHONE ACTION
    //a = 1;
    bmePhoneServerCharacteristics.setValue("1");
    bmePhoneServerCharacteristics.indicate();
    bmePhoneServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
    // SERVER ACTION
    //e = 2;
    bmeEspServerCharacteristics.setValue("1");
    bmeEspServerCharacteristics.indicate();
    bmeEspServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
    delay(500);
    // PICO ACTION
    if(Serial.available()){
    Bitto_detected = Serial.readStringUntil('\n');
    i = Bitto_detected.toInt();
    Serial.println(i);
    bmePicoServerCharacteristics.setValue(String(i));
    bmePicoServerCharacteristics.indicate();
    bmePicoServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
    delay(500); }
    // SERVER ACION
    bmeEspServerCharacteristics.setValue("1");
    bmeEspServerCharacteristics.indicate();
    bmeEspServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
    delay(500); 
    // ROBOT1 ACTION
    bmeRobot1ServerCharacteristics.setValue("2");
    bmeRobot1ServerCharacteristics.indicate();
    bmeRobot1ServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
    // ROBOT2 ACTION 

    bmeRobot2ServerCharacteristics.setValue("roscaGiveHeart");
    bmeRobot2ServerCharacteristics.indicate();
    bmeRobot2ServerCharacteristics.setCallbacks(new MyCharacteristicCallbacks());
      }

}
