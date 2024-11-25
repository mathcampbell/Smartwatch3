#include <Wire.h>
#include <Arduino.h>


// Define the I2C pins for your device
#define I2C_SDA_PIN 11
#define I2C_SCL_PIN 10

// Function prototypes (declarations)
void scanI2CDevices();
void interrogateDevice(uint8_t address);
byte readRegister(uint8_t address, uint8_t reg);

void setup() {
  // Initialize the I2C bus on specified SDA and SCL pins
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Serial.begin(115200);
  delay(5000);
  Serial.println("Starting scan");

  
  while (!Serial); // Wait for serial to initialize
  Serial.println("\nI2C Scanner with device interrogation");

  // Start scanning I2C devices
  scanI2CDevices();
}

void loop() {
  // Empty loop - scanning is done in setup
}

// Scan for I2C devices and interrogate them
void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus for devices...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");

      // Interrogate the device by reading common registers
      interrogateDevice(address);

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Scan complete\n");
}

// Attempt to read common registers from the device
void interrogateDevice(uint8_t address) {
  Serial.println("Attempting to read common registers...");

  // Try reading register 0x00
  byte reg0 = readRegister(address, 0x00);
  Serial.print("Register 0x00: 0x");
  Serial.println(reg0, HEX);

  // Try reading register 0x01
  byte reg1 = readRegister(address, 0x01);
  Serial.print("Register 0x01: 0x");
  Serial.println(reg1, HEX);

  // Try reading register 0x0F (common for device IDs)
  byte reg0F = readRegister(address, 0x0F);
  Serial.print("Register 0x0F: 0x");
  Serial.println(reg0F, HEX);

  // Try reading register 0xA7 (used for some touch controllers)
  byte regA7 = readRegister(address, 0xA7);
  Serial.print("Register 0xA7: 0x");
  Serial.println(regA7, HEX);
}

// Function to read a register from the device
byte readRegister(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) {
    Serial.println("I2C Read failed.");
    return 0xFF; // Return error value if transmission fails
  }

  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    return Wire.read(); // Read the byte from the device
  }
  return 0xFF; // Return error value if no data is received
}
