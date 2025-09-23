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
#define ACS_BULBS A0   // ACS712 for bulbs (220V)
#define ACS_FANS  A1   // ACS712 for fans (5V)

// === Load States ===
bool bulb1State = false, bulb2State = false, fan1State = false, fan2State = false;

// === Measurement Variables ===
float currentBulbs, currentFans;
float powerBulbs, powerFans, totalPower;
float energyConsumed = 0.0; // in Wh
unsigned long lastMillis = 0;

// === ACS712 Calibration Values (adjust to your module) ===
float sensitivity = 0.185; // V/A for ACS712-5A
float Vref = 2.5;          // midpoint voltage

// === Helper function: read current from ACS712 ===
float readCurrent(int pin) {
  long sum = 0;
  for (int i = 0; i < 200; i++) {
    sum += analogRead(pin);
  }
  float adcValue = sum / 200.0;
  float voltage = (adcValue * 5.0) / 1023.0;
  float current = (voltage - Vref) / sensitivity;
  return abs(current);
}

// === Function to send SMS ===
void sendSMS(String text) {
  sim800.println("AT+CMGF=1"); // SMS text mode
  delay(500);
  sim800.println("AT+CMGS=\"+1234567890\""); // <-- Replace with your phone number
  delay(500);
  sim800.print(text);
  sim800.write(26); // CTRL+Z
  delay(2000);
}

// === Function to update LCD with states ===
void updateLCDstates() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("B1:");
  lcd.print(bulb1State ? "ON" : "OFF");
  lcd.print(" B2:");
  lcd.print(bulb2State ? "ON" : "OFF");

  lcd.setCursor(0,1);
  lcd.print("F1:");
  lcd.print(fan1State ? "ON" : "OFF");
  lcd.print(" F2:");
  lcd.print(fan2State ? "ON" : "OFF");
}

// === Function to update LCD with power ===
void updateLCDpower() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Bulbs:");
  lcd.print(powerBulbs,1);
  lcd.print("W");

  lcd.setCursor(0,1);
  lcd.print("Fans:");
  lcd.print(powerFans,1);
  lcd.print("W");
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
    sendSMS("Power bulbs: " + String(powerBulbs,1) + "W, Fans: " + String(powerFans,1) + "W, Total: " + String(totalPower,1) + "W, Energy: " + String(energyConsumed,2) + "Wh");
  }
  updateLCDstates();
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

  updateLCDstates();

  sim800.println("AT+CMGF=1");  
  delay(1000);
  sim800.println("AT+CNMI=1,2,0,0,0"); // Forward SMS
}

// === Loop ===
String incomingSMS = "";

void loop() {
  // 1. Read currents
  currentBulbs = readCurrent(ACS_BULBS);
  currentFans  = readCurrent(ACS_FANS);

  // 2. Calculate power
  powerBulbs = 220.0 * currentBulbs;
  powerFans  = 5.0   * currentFans;
  totalPower = powerBulbs + powerFans;

  // 3. Energy accumulation every second
  unsigned long now = millis();
  if (now - lastMillis >= 1000) {
    lastMillis = now;
    energyConsumed += totalPower / 3600.0; // W → Wh each second
    updateLCDpower();
  }

  // 4. Check for incoming SMS
  if (sim800.available()) {
    char c = sim800.read();
    incomingSMS += c;
    if (c == '\n') {
      Serial.println("SMS: " + incomingSMS);
      controlLoad(incomingSMS);
      incomingSMS = "";
    }
  }
}
