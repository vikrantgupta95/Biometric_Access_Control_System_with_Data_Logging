/*
  Author: Vikrant Gupta
  Email: vikrantgupta95@gmail.com
  LinkedIn: https://www.linkedin.com/in/vikrantgupta95/
  Instagram: https://www.instagram.com/crazyvikku/
  GitHub: https://github.com/vikrantgupta95

  Copyright (c) 2024 Vikrant Gupta

  Description:
  This code interfaces with a DS1307 RTC via I2C and Wire library to utilize date and time functions. 
  Additionally, it communicates with an SD card using SPI to log data. It also incorporates an Adafruit 
  fingerprint sensor module to detect fingerprints and perform various actions based on the detected 
  fingerprint ID. When a valid fingerprint is detected, it logs corresponding data to an SD card. The 
  code continuously monitors the fingerprint sensor for input and takes appropriate actions based on the 
  detected fingerprint ID.

  Last updated: 2024-03-25
*/

#include <SPI.h>
#include <SD.h>
int count = 0;                              // Variable to count fingerprint matches
const int chipSelect = 4;                   // Chip select pin for SD card

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

int getFingerprintIDez();                   // Forward declaration of function

SoftwareSerial mySerial(2, 3);              // Software serial communication with fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{
  Serial.begin(9600);                       // Initialize serial communication
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {              // Initialize SD card
    Serial.println("Card failed, or not present");
    // return;
  }
  Serial.println("card initialized.");

  finger.begin(57600);                      // Initialize fingerprint sensor
  
  if (finger.verifyPassword()) {            // Verify fingerprint sensor password
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);                               // Loop indefinitely if fingerprint sensor not found
  }
  
  Serial.println("Waiting for valid finger...");
  
  pinMode(5, OUTPUT);                       // Pin to control external device upon fingerprint match
  digitalWrite(5, LOW);                     // Ensure the device is initially off
  pinMode(6, OUTPUT);                       // Pin to trigger action when fingerprint not found
  digitalWrite(6, LOW);                     // Ensure the action is initially inactive
}

void loop() {
  getFingerprintID();                       // Continuously monitor fingerprint sensor
  delay(50);                                // Reduce CPU usage
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();           // Capture fingerprint image
  
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.image2Tz();                   // Convert image to template
  
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  p = finger.fingerFastSearch();          // Search for fingerprint match
  
  if (p == FINGERPRINT_OK) {              // Fingerprint match found
    Serial.println("Found a print match!");
    digitalWrite(5, HIGH);                // Trigger action for matched fingerprint ID
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) { // Fingerprint not found
    Serial.println("Did not find a match");
    digitalWrite(6, HIGH);                // Trigger action for unmatched fingerprint ID
    delay(2000);                           // Wait before resetting action
    digitalWrite(6, LOW);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  // Perform actions based on detected fingerprint ID
  if (finger.fingerID == 12) {            // Example action for fingerprint ID 12
    count++;                               // Increment count
    String str = String(count);             // Convert count to string
    String datastr = ". Vikrant Gupta";    // Additional data
    str += String(datastr);                 // Concatenate strings
    Serial.println("Vikrant data loaded");  // Debug message
    delay(2000);                            // Wait before proceeding
    digitalWrite(5, LOW);                   // Reset external device
    File dataFile = SD.open("datalog.txt", FILE_WRITE);  // Open data log file

    if (dataFile) {                         // Check if file is available
      dataFile.println(str);                // Write data to file
      dataFile.close();                     // Close file
    } else {
      Serial.println("Error opening datalog.txt");  // Error message if file not available
    }
  }
}
