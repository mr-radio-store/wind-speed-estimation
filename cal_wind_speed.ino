/*
Calculate wind speed based on fan-motor rotation

***
🌬️ Wind Speed from Fan RPM
✅ General Formula:
Wind Speed (m/s)=π×D×RPM60
Wind Speed (m/s)=60π×D×RPM​

Where:
    DD = Fan diameter (in meters)
    ππ ≈ 3.1416
    RPM = Rotations per minute
This assumes the tip of the fan blade moves at wind speed (which is a good approximation for small axial fans).
***

Wire connection
OLED Display (I2C) – SSD1306
OLED Pin	Arduino Pin
VCC	5V
GND	GND
SDA	A4 (UNO) / 20 (Mega)
SCL	A5 (UNO) / 21 (Mega)
🔌 2. ACS712 Current Sensor
ACS712 Pin	Arduino Pin
VCC	5V
GND	GND
OUT	A1

OLED wire connecction
 Wiring: I2C OLED
OLED Pin	Arduino Uno / Mega
VCC	5V
GND	GND
SDA	A4 (Uno) / 20 (Mega)
SCL	A5 (Uno) / 21 (Mega)

*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CURRENT_PIN A0               // ACS712 analog output
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MIN_PULSE_INTERVAL 5         // Minimum ms between pulses
#define TRIGGER_OFFSET 10            // Current rise above baseline to count as pulse
#define FAN_DIAMETER_M 0.1           // Fan diameter in meters (10 cm)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
unsigned long lastTime = 0;
unsigned long lastPulseTime = 0;
int pulseCount = 0;
float rpm = 0;
float windSpeed = 0;
bool lastAbove = false;

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Fan RPM & Wind Speed");
  display.display();
  delay(2000);
  lastTime = millis();
}

void loop() {
  int currentVal = analogRead(CURRENT_PIN);
  static int baseline = 0;

  // Smooth moving average
  baseline = (baseline * 9 + currentVal) / 10;
  unsigned long now = millis();

  // Detect rising edge past baseline + offset
  if (currentVal > baseline + TRIGGER_OFFSET &&
      !lastAbove &&
      (now - lastPulseTime > MIN_PULSE_INTERVAL)) {

    pulseCount++;
    lastPulseTime = now;
    lastAbove = true;

    Serial.println("Pulse detected");
  }

  if (currentVal <= baseline + TRIGGER_OFFSET) {
    lastAbove = false;
  }

  // Update every second
  if (now - lastTime >= 1000) {
    rpm = pulseCount * 60.0;  // 1 pulse = 1 rotation
    windSpeed = (3.1416 * FAN_DIAMETER_M * rpm) / 60.0;  // m/s

    // Serial debug
    Serial.print("Pulses: ");
    Serial.print(pulseCount);
    Serial.print("  RPM: ");
    Serial.print(rpm);
    Serial.print("  Wind: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");

    // OLED display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Fan Speed Monitor");

    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print((int)rpm);
    display.print(" RPM");

    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Wind: ");
    display.print(windSpeed, 2);
    display.print(" m/s");

    display.display();

    pulseCount = 0;
    lastTime = now;
  }
}
