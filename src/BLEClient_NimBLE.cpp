#include <Arduino.h>
#include <NimBLEDevice.h>
#include <time.h>
#include "BLEClient_NimBLE.h"



 // UUIDs for Current Time Service and Current Time Characteristic
static NimBLEUUID currentTimeServiceUUID((uint16_t)0x1805);
static NimBLEUUID currentTimeCharacteristicUUID((uint16_t)0x2A2B);

static NimBLEAdvertisedDevice* myDevice = nullptr;
static NimBLEClient* pClient = nullptr;

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->isAdvertisingService(currentTimeServiceUUID)) {
            Serial.println("Found device advertising Current Time Service");
            myDevice = advertisedDevice;
            NimBLEDevice::getScan()->stop();
        }
    }
};

void initBLEClient() {
    NimBLEDevice::init("ESP32_NimBLE_Client");
}

bool updateTimeFromBLEServer() {
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(10, false);

    if (myDevice == nullptr) {
        Serial.println("No Current Time Service found.");
        return false;
    }

    pClient = NimBLEDevice::createClient();
    if (!pClient->connect(myDevice)) {
        Serial.println("Failed to connect to BLE server.");
        return false;
    }

    NimBLERemoteService* pRemoteService = pClient->getService(currentTimeServiceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("Failed to find Current Time Service.");
        pClient->disconnect();
        return false;
    }

    NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(currentTimeCharacteristicUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("Failed to find Current Time Characteristic.");
        pClient->disconnect();
        return false;
    }

    std::string currentTimeData = pRemoteCharacteristic->readValue();
    if (currentTimeData.length() >= 10) {
        struct tm timeinfo;
        timeinfo.tm_year = (currentTimeData[0] | (currentTimeData[1] << 8)) - 1900;
        timeinfo.tm_mon  = currentTimeData[2] - 1;
        timeinfo.tm_mday = currentTimeData[3];
        timeinfo.tm_hour = currentTimeData[4];
        timeinfo.tm_min  = currentTimeData[5];
        timeinfo.tm_sec  = currentTimeData[6];
        time_t currentTime = mktime(&timeinfo);
        struct timeval now = { .tv_sec = currentTime };
        settimeofday(&now, nullptr);
        Serial.println("Time updated from BLE server.");
        pClient->disconnect();
        return true;
    } else {
        Serial.println("Failed to read valid Current Time.");
        pClient->disconnect();
        return false;
    }
}
 