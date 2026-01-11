#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define LED_PIN 2

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();
      Serial.print("Odebrano: ");
      Serial.println(value);
      if (value == "1") digitalWrite(LED_PIN, HIGH);
      else if (value == "0") digitalWrite(LED_PIN, LOW);
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Połączono!");
    };
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Rozłączono. Wznawiam rozgłaszanie...");
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  BLEDevice::init("NieKradnijTegoESP");
  Serial.println("Start BLE: NieKradnijTegoESP");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         "beb5483e-36e1-4688-b7f5-ea07361b26a8",
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();
  Serial.println("Rozgłaszanie uruchomione.");
}

void loop() {}
