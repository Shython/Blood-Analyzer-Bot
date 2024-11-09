#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 or 0x3F for a 16 chars and 2 line display

int optPin = A0;  
float voltage = 0.0;
float maxVoltage = 0.0;
float glucoseLevel = 0.0;
String bloodGroup = "";
bool validMeasurement = true;  
unsigned long startTime;
const unsigned long cooldownPeriod = 5000; 

void setup() {
  Serial.begin(9600); 
  lcd.begin(16, 2);  // Initialize the LCD
  lcd.backlight();   // Turn on the LCD backlight
  lcd.print("Initializing..."); // Display initial message
  pinMode(optPin, INPUT);  
  startTime = millis(); 
}

void loop() {
  int sensorValue = analogRead(optPin);
  voltage = sensorValue * (5000.0 / 1023.0);

  // Error handling for different conditions
  if (voltage == 0) {
    Serial.println("Error: Sensor blocked");
    lcd.clear();
    lcd.print("Error: Blocked"); // Display error on LCD
    delay(1000);  
    return;
  } else if (voltage > 100) {
    Serial.println("Error: No finger placed");
    lcd.clear();
    lcd.print("No finger placed"); // Display error on LCD
    validMeasurement = false;
    delay(1000);
    return;
  }

  // Measurement phase (5 seconds)
  if (millis() - startTime < 5000) {  
    if (validMeasurement) {
      if (voltage > maxVoltage) {
        maxVoltage = voltage;
      }
      Serial.print("Current Voltage: ");
      Serial.print(voltage); 
      Serial.println(" mV");
    }
    delay(500);  

  } else {  
    // End of Measurement Phase
    if (validMeasurement) {
      // Determine the blood group based on maximum voltage observed
      if (maxVoltage >= 14 && maxVoltage <= 29.33) {
        bloodGroup = "O";
      } else if (maxVoltage >= 29.33 && maxVoltage <= 45) {
        bloodGroup = "A";
      } else if (maxVoltage >= 39.33 && maxVoltage <= 99) {
        bloodGroup = "B";
      } else {
        bloodGroup = "Unknown";
      }

      // Calculate glucose level using the formula
      glucoseLevel = (8e-5 * maxVoltage * maxVoltage) + (0.1873 * maxVoltage) + 46.131;

      // Display final results on Serial Monitor
      Serial.println("===== Final Results After 5 Seconds =====");
      Serial.print("Max Voltage: ");
      Serial.print(maxVoltage);  
      Serial.print(" V, Blood Group: ");
      Serial.print(bloodGroup);
      Serial.print(", Glucose Level: ");
      Serial.print(glucoseLevel);
      Serial.println(" mg/dL");
      Serial.println("=========================================");

      // Display final results on LCD
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("BG: ");
      lcd.print(bloodGroup);   // Display blood group on LCD
      lcd.setCursor(0, 1);
      lcd.print("Glucose: ");
      lcd.print(glucoseLevel); // Display glucose level on LCD
      lcd.print(" mg/dL");

    } else {
      Serial.println("Measurement invalid: No finger detected at some point during measurement window.");
      lcd.clear();
      lcd.print("Invalid: No finger"); // Display invalid message on LCD
    }

    // Cooldown Phase (5 seconds)
    unsigned long cooldownStartTime = millis();
    while (millis() - cooldownStartTime < cooldownPeriod) {
      sensorValue = analogRead(optPin);
      voltage = sensorValue * (5000.0 / 1023.0);
      Serial.print("Cooldown - Voltage: ");
      Serial.print(voltage);
      Serial.println(" mV");
      delay(500);
    }

    // Reset for the next measurement cycle
    maxVoltage = 0.0;
    validMeasurement = true;
    startTime = millis(); 
    delay(2000); 
  }
}
