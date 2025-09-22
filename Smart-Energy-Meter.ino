#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === GSM Module (SIM800L) ===
SoftwareSerial sim800(10, 11); // RX, TX

// === Relay Pins ===
#define RELAY_BULB1 2
#define RELAY_BULB2 3
#define RELAY_FAN1  4
#define RELAY_FAN2  5

// === Current Sensor Pins (ACS712 outputs) ===
#define ACS_BULB1 A0
#define ACS_BULB2 A1
#define ACS_FAN1  A2
#define ACS_FAN2  A3

// === System Variables ===
String incomingSMS = "";
bool bulb1State = false, bulb2State = false, fan1State = false, fan2State = false;
float currentBulb1, currentBulb2, currentFan1, currentFan2;
float powerConsumption = 0.0; // in Watt-Hour (Wh)
unsigned long lastMillis = 0;

// === Calibration Values for ACS712 (adjust depending on module) ===
float sensitivity = 0.185; // V/A for ACS712-5A (change if using 20A or 30A version)
float Vref = 2.5;          // mid-point voltage at 0A

// === Function to read current from ACS712 ===
float readCurrent(int pin) {
  long sum = 0;
  for (int i = 0; i < 200; i++) {
    sum += analogRead(pin);
  }
  float adcValue = sum / 200.0;
  float voltage = (adcValue * 5.0) / 1023.0;
  float current = (voltage - Vref) / sensitivity;
  if (current < 0) current = -current; // absolute value
  return current;
}

// === Function to send SMS ===
void sendSMS(String text) {
  sim800.println("AT+CMGF=1"); // SMS text mode
  delay(500);
  sim800.println("AT+CMGS=\"+1234567890\""); // <-- Replace with your phone number
  delay(500);
  sim800.print(text);
  sim800.write(26); // CTRL+Z to send
  delay(2000);
}

// === Function to update LCD ===
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("B1:");
  lcd.print(bulb1State ? "ON" : "OFF");
  lcd.print(" B2:");
  lcd.print(bulb2State ? "ON" : "OFF");

  lcd.setCursor(0, 1);
  lcd.print("F1:");
  lcd.print(fan1State ? "ON" : "OFF");
  lcd.print(" F2:");
  lcd.print(fan2State ? "ON" : "OFF");
}

// === Function to update LCD with currents (alternate display) ===
void updateLCDcurrent() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("B1:");
  lcd.print(currentBulb1, 1);
  lcd.print("A B2:");
  lcd.print(currentBulb2, 1);

  lcd.setCursor(0, 1);
  lcd.print("F1:");
  lcd.print(currentFan1, 1);
  lcd.print(" F2:");
  lcd.print(currentFan2, 1);
}

// === Function to control loads ===
void controlLoad(String cmd) {
  if (cmd.indexOf("Power On light bulb1") >= 0) {
    digitalWrite(RELAY_BULB1, LOW); bulb1State = true;
    sendSMS("Bulb1 is ON");
  } else if (cmd.indexOf("Power Off light bulb1") >= 0) {
    digitalWrite(RELAY_BULB1, HIGH); bulb1State = false;
    sendSMS("Bulb1 is OFF");
  }
  else if (cmd.indexOf("Power On light bulb2") >= 0) {
    digitalWrite(RELAY_BULB2, LOW); bulb2State = true;
    sendSMS("Bulb2 is ON");
  } else if (cmd.indexOf("Power Off light bulb2") >= 0) {
    digitalWrite(RELAY_BULB2, HIGH); bulb2State = false;
    sendSMS("Bulb2 is OFF");
  }
  else if (cmd.indexOf("Power On fan1") >= 0) {
    digitalWrite(RELAY_FAN1, LOW); fan1State = true;
    sendSMS("Fan1 is ON");
  } else if (cmd.indexOf("Power Off fan1") >= 0) {
    digitalWrite(RELAY_FAN1, HIGH); fan1State = false;
    sendSMS("Fan1 is OFF");
  }
  else if (cmd.indexOf("Power On fan2") >= 0) {
    digitalWrite(RELAY_FAN2, LOW); fan2State = true;
    sendSMS("Fan2 is ON");
  } else if (cmd.indexOf("Power Off fan2") >= 0) {
    digitalWrite(RELAY_FAN2, HIGH); fan2State = false;
    sendSMS("Fan2 is OFF");
  }
  else if (cmd.indexOf("Get power consumption") >= 0) {
    sendSMS("Total Power Consumption: " + String(powerConsumption, 2) + " Wh");
  }
  updateLCD();
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(RELAY_BULB1, OUTPUT);
  pinMode(RELAY_BULB2, OUTPUT);
  pinMode(RELAY_FAN1, OUTPUT);
  pinMode(RELAY_FAN2, OUTPUT);

  digitalWrite(RELAY_BULB1, HIGH);
  digitalWrite(RELAY_BULB2, HIGH);
  digitalWrite(RELAY_FAN1, HIGH);
  digitalWrite(RELAY_FAN2, HIGH);

  updateLCD();

  sim800.println("AT+CMGF=1");  
  delay(1000);
  sim800.println("AT+CNMI=1,2,0,0,0"); // Forward SMS to serial
}

// === Loop ===
void loop() {
  // 1. Read current values
  currentBulb1 = readCurrent(ACS_BULB1);
  currentBulb2 = readCurrent(ACS_BULB2);
  currentFan1  = readCurrent(ACS_FAN1);
  currentFan2  = readCurrent(ACS_FAN2);

  // 2. Update power consumption (approximate, every second)
  unsigned long now = millis();
  if (now - lastMillis >= 1000) {
    lastMillis = now;
    float totalCurrent = currentBulb1 + currentBulb2 + currentFan1 + currentFan2;
    float voltage = 220.0; // Adjust for your mains
    float power = voltage * totalCurrent; // Instantaneous power in Watts
    powerConsumption += power / 3600.0;   // Convert W → Wh each second
    updateLCDcurrent();
  }

  // 3. Check for incoming SMS
  if (sim800.available()) {
    char c = sim800.read();
    incomingSMS += c;
    if (c == '\n') {
      Serial.println("SMS Received: " + incomingSMS);
      controlLoad(incomingSMS);
      incomingSMS = "";
    }
  }
}
