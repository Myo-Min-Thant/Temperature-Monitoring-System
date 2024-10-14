/*
 * Temperature Monitoring System with LCD Display and LED Indicator
 * 
 * Description:
 * This program reads temperature and humidity data from a DHT22 sensor and displays the results on a 16x2 LCD screen.
 * It also provides visual feedback on temperature changes using custom arrows (up/down) and adjusts the brightness
 * of an LED according to the temperature value using Pulse Width Modulation (PWM).
 * 
 * Hardware Components:
 * - DHT22 (AM2302) Temperature and Humidity Sensor
 * - 16x2 LCD Display (Hitachi HD44780 driver)
 * - LED for visual indication of temperature
 * 
 * Connections:
 * - DHT22 Sensor:
 *   - Data pin connected to GPIO 4
 *   - Power (VCC) and Ground (GND) connected to the appropriate rails
 * 
 * - LCD Display:
 *   - RS (Register Select): GPIO 12
 *   - EN (Enable): GPIO 14
 *   - D4: GPIO 26
 *   - D5: GPIO 27
 *   - D6: GPIO 25
 *   - D7: GPIO 33
 *   - Power (VCC) and Ground (GND) connected to the appropriate rails
 * 
 * - LED:
 *   - Positive (Anode) connected to GPIO 2
 *   - Negative (Cathode) connected to Ground (GND)
 * 
 * Custom LCD Characters:
 * - Degree symbol (°)
 * - Up arrow (for increasing temperature indication)
 * - Down arrow (for decreasing temperature indication)
 * 
 * Features:
 * - Displays real-time temperature in Celsius and Fahrenheit, and indicates temperature trend.
 * - Displays "Failed to read" if sensor communication fails.
 * - Adjusts LED brightness based on temperature (using PWM).
 * - Uses custom characters to display up/down arrows or equal signs for temperature trend.
 * 
 * Libraries:
 * - LiquidCrystal.h: For controlling the 16x2 LCD.
 * - DHT.h: For reading temperature and humidity data from the DHT22 sensor.
 * 
 * Author: Myo Min Thant
 * Date: 10 Oct 2024
 */

// Rest of the code goes here...

#include <LiquidCrystal.h>
#include "DHT.h"

// Define the pin for the DHT sensor and type
#define DHTPIN 4
#define DHTTYPE DHT22  // DHT 22 (AM2302), AM2321

// Define the LCD pin connections
const int rs = 12, en = 14, d4 = 26, d5 = 27, d6 = 25, d7 = 33;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // Initialize the LCD

DHT dht(DHTPIN, DHTTYPE);  // Initialize the DHT sensor

// Custom characters for degree symbol, up arrow, and down arrow
byte degree[8] = {
  0b00111,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte upArrow[8] = { 
  B00000, 
  B00100, 
  B01110, 
  B11111, 
  B00100, 
  B00100, 
  B00100, 
  B00100 
};

byte downArrow[8] = { 
  B00100, 
  B00100, 
  B00100, 
  B00100, 
  B00100, 
  B11111, 
  B01110, 
  B00100 
};

// Global variables to track temperature changes and display state
bool display_count = false;
int previousTemp = 0;
int tempEqualCount = 0;

// LED pin for PWM control
const int ledPin = 2;

void setup() {
  Serial.begin(9600);           // Initialize serial communication
  lcd.begin(16, 2);             // Initialize the LCD with 16 columns and 2 rows
  dht.begin();                  // Initialize the DHT sensor

  // Create custom characters on the LCD
  lcd.createChar(0, degree);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  pinMode(ledPin, OUTPUT);      // Set LED pin as output
}

void loop() {
  delay(100);

  // Read humidity and temperature from the DHT sensor
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();     // Temperature in Celsius
  float tempF = dht.readTemperature(true); // Temperature in Fahrenheit

  // Check if the reading failed and handle error display
  if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    
    if (display_count) {
      lcd.clear();
      display_count = false;
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Failed to read");
      lcd.setCursor(0, 1);
      lcd.print("from DHT sensor!");
      display_count = true;
    }
    delay(200);
    return;
  }

  // Calculate the heat index for both Celsius and Fahrenheit
  float heatIndexC = dht.computeHeatIndex(tempC, humidity, false);
  float heatIndexF = dht.computeHeatIndex(tempF, humidity);

  // Print sensor data to the Serial monitor
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(tempC);
  Serial.print(F("°C "));
  Serial.print(tempF);
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexC);
  Serial.print(F("°C "));
  Serial.print(heatIndexF);
  Serial.println(F("°F"));

  // Display temperature on the LCD in both Celsius and Fahrenheit
  lcd.setCursor(0, 0);
  lcd.print("Temperature ");

  lcd.setCursor(0, 1);
  displayFormattedTemperature(int(tempC), "C");
  lcd.setCursor(6, 1);
  displayFormattedTemperature(int(tempF), "F");

  // Handle temperature trend display (up/down arrow or equal sign)
  handleTemperatureTrend(int(tempC));

  // Adjust LED brightness based on temperature using PWM
  int pwmValue = map(int(tempC), -40, 80, 0, 255);
  analogWrite(ledPin, pwmValue);
}

// Function to display temperature with proper spacing and units
void displayFormattedTemperature(int temp, const char* unit) {
  if (temp < 0 && temp > -10) lcd.print(" ");
  else if (temp >= 0 && temp < 10) lcd.print("  ");
  else if (temp > 9 && temp < 100) lcd.print(" ");

  lcd.print(temp);
  lcd.write(byte(0));  // Display degree symbol
  lcd.print(unit);     // Display unit (C or F)
}

// Function to display arrows based on temperature change
void handleTemperatureTrend(int currentTemp) {
  if (currentTemp < previousTemp) {
    lcd.setCursor(14, 0);
    lcd.write(" ");
    lcd.setCursor(14, 1);
    lcd.write(byte(2));  // Down arrow
    previousTemp = currentTemp;
    tempEqualCount = 0;
  } else if (currentTemp > previousTemp) {
    lcd.setCursor(14, 1);
    lcd.write(" ");
    lcd.setCursor(14, 0);
    lcd.write(byte(1));  // Up arrow
    previousTemp = currentTemp;
    tempEqualCount = 0;
  } else if (currentTemp == previousTemp) {
    if (tempEqualCount > 500) tempEqualCount = 0;

    if (tempEqualCount > 100 && tempEqualCount < 150) {
      lcd.setCursor(14, 1);
      lcd.write(" ");
      lcd.setCursor(14, 0);
      lcd.write("=");
    } else if (tempEqualCount > 200) {
      lcd.setCursor(14, 1);
      lcd.write(" ");
      lcd.setCursor(14, 0);
      lcd.write(" ");
    }

    tempEqualCount++;
  }
}
