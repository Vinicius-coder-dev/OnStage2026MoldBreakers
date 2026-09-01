#include "BLEDevice.h"

#include <Wire.h>

#define bleServerName "Esp_princ"

#define RXD2 16
#define TXD2 17
String bluetooth_command;

String command_detected;
String command_toSend;
String last_command;

static BLEUUID PicoProjection_UUID("72eb11d6-b7ae-45a6-ba1e-8ce8c7dbf8f0");
static BLEUUID PicoProjectionCharacteristicsUUID("e0a2a0ef-5b17-4777-bcf3-e8900fb30636");


static BLERemoteCharacteristic* pRemoteCharacteristic;

static boolean doConnect = false;
static boolean connected = false;

static BLEAddress *pServerAddress;
 
static BLERemoteCharacteristic* PicoProjectionCharacteristic = NULL;

char* TesteBittoChar;

boolean newTesteBitto = false;

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
 
 
static void TesteBittescoNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store command value
  TesteBittoChar = (char*)pData;
  newTesteBitto = true;
}

bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(PicoProjection_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(PicoProjection_UUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.

  PicoProjectionCharacteristic = pRemoteService->getCharacteristic(PicoProjection_UUID);



  if (PicoProjectionCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  return true;
}

void setup() {
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Arduino BLE Client application...");


  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(255);
} // End of setup.

void loop(){
  
  if (Serial.available()) {
    command_toSend = Serial.readStringUntil('\n');
  }
  if (Serial2.available() > 0) {
    command_detected = Serial2.readStringUntil('\n');
    //Serial.println(command_detected);
  }
  if (command_toSend != "") {
    Serial2.println(command_toSend);
    Serial.println(command_toSend);
    command_toSend = "";
  }
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      PicoProjectionCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2901));
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  if(PicoProjectionCharacteristic !=NULL){
    
    Serial.println(PicoProjectionCharacteristic->readValue());
    if (last_command !=  PicoProjectionCharacteristic->readValue()){
    last_command = PicoProjectionCharacteristic->readValue();
    bluetooth_command = PicoProjectionCharacteristic->readValue();
    }
    if (bluetooth_command == "20")
    {
      command_toSend = "0";
      bluetooth_command = "0";
    }

    if (bluetooth_command == "21")
    {
      command_toSend = "1";

      bluetooth_command = "0";
    }

    if (bluetooth_command == "22")
    {
      command_toSend = "2";

      bluetooth_command = "0";
    }

    if (bluetooth_command == "23")
    {
      command_toSend = "3";
      
      bluetooth_command = "0";
    }
    
    if (bluetooth_command == "24")
    {
      command_toSend = "4";
      
      bluetooth_command = "0";
    }

    if (bluetooth_command == "25")
    {
      command_toSend = "5";
      
      bluetooth_command = "0";
    }

    if (bluetooth_command == "26")
    {
      command_toSend = "6";
      
      bluetooth_command = "0";
    }

    if (bluetooth_command == "27")
    {
      command_toSend = "7";
      
      bluetooth_command = "0";
    }
    //Bitto, coloque os ifs aqui, repetindo a estrutura acima
  } 
  
}




