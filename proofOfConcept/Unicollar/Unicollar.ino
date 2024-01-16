

/**
* SocialPonyCollar
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Adafruit_NeoPixel.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
                  //10000000-636f-6c6c-6172-000000000000
#define SERVICE_UUID "10000000-636f-6c6c-6172-000000000000"
#define CHARACTERISTIC_UUID "20000000-636f-6c6c-6172-123456789000"



unsigned long scanMillis = 0;        // will store last time LED was updated
unsigned long SCAN_INTERVAL = 5000;  // will store last time LED was updated

unsigned long blinkMillis = 0;        // will store last time LED was updated
unsigned long BLINK_INTERVAL = 500;  // will store last time LED was updated

const int MAXPONIES = 20;
int devices = 0;
int scanCount = 0;

int ledState=0;

int scanTime = 2;  //In seconds
BLEScan *pBLEScan;

struct ponyContact {
  int lastscan;
  int rssi;
  String addr;
  ponyContact():lastscan(0),rssi(0),addr(""){}
};

struct ponyContact ponylist[MAXPONIES];
int ponyCount=0;
int lastPonyCount=0;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(advertisedDevice.getServiceUUID().equals(BLEUUID(SERVICE_UUID)) && ponyCount<MAXPONIES){
      //Serial.print("HELO PONY: ");
      //Serial.println(advertisedDevice.getAddress().toString().c_str());
      ponylist[ponyCount].lastscan = scanCount;
      ponylist[ponyCount].addr = advertisedDevice.getAddress().toString().c_str();
      ponylist[ponyCount].rssi = advertisedDevice.getRSSI();
      ponyCount++;
    }
    
  }
};


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        2 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 16 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("SOCIAL-PONY-COLLAR");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  // FastLED.setBrightness(100);

  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(50);  // less or equal setInterval value
  pBLEScan->start(scanTime,doneScan, true);
  pixels.begin();
  pixels.setBrightness(100);
  pixels.clear();
}

void doneScan(BLEScanResults foundDevices){

    Serial.println(foundDevices.getCount());
    
    devices = foundDevices.getCount();
    
    pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory

    Serial.print("Found ");
    Serial.print(ponyCount);
    Serial.println(" Ponies!");
        for(int i = 0; i<ponyCount;i++){
      Serial.print("Helo: ");
      Serial.print(ponylist[i].rssi);
      Serial.print("-");
      Serial.print(ponylist[i].addr);
      Serial.println(";");
    }
    lastPonyCount=ponyCount;
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - scanMillis >= SCAN_INTERVAL) {
    scanMillis = currentMillis;
    pixels.fill(pixels.Color(0, 0, 0),1,15);
    pixels.fill(pixels.Color(0, 0, 50),1,lastPonyCount);

    scanCount++;
    ponyCount=0;
    pBLEScan->start(scanTime,doneScan, true);
  }


  if (currentMillis - blinkMillis >= BLINK_INTERVAL) {
    if(ledState == 0){
      ledState=1;
      pixels.setPixelColor(0, pixels.Color(150, 0, 0));
    }else{
      ledState=0;
      pixels.setPixelColor(0, pixels.Color(0, 150, 0));
    }   
    
    blinkMillis = currentMillis;
    pixels.show(); 
  }
  
  delay(1);
}