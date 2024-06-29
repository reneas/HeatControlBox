#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>

// If the hardware supports SPI (Serial Peripheral Interface), include the SPI library
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

// If the hardware supports I2C (Inter-Integrated Circuit), include the Wire library
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// DS18B20 temperature sensor pin
#define TEMPPIN 5 // GPIO5/D1 / DS18B20
#define HEATPIN 4 // GPIO4/D2 / Relay for heating element

// for buttons
#define UpButtonPin 3   // GPIO3/RX / Button to increase // GPIO15 is always LOW on bootup that's why we use GPIO3
#define DownButtonPin 13 // GPIO13/D7 / Button to decrease

// DON'T CHANGE THESE PINS! They are fixed for the I2C OLED display
#define SCL_PIN 14 // GPIO14/D5
#define SDA_PIN 12 // GPIO12/D6

// OLED display object declaration
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);

// temp sensor object declaration
OneWire oneWire(TEMPPIN);
DallasTemperature sensors(&oneWire);

// Define variables
float t;                 // the read temperature
float target_t = 28.0;   // Initial target temperature
float target_max = 30.0; // Maximum temperature
float target_min = 25.0; // Minimum temperature
float target_range = 0.4;  // Temperature range around the target temperature
float step_size = 0.5;   // Step size for changing

// Define functions

bool isUpButtonPressed()
{                                         // Check if the UpButton is pressed
  return digitalRead(UpButtonPin) == LOW; // If pressed, digitalRead(UpButtonPin) returns LOW -> return true / if not, then HIGH -> return false
}

bool isDownButtonPressed()
{                                           // Check if the DownButton is pressed
  return digitalRead(DownButtonPin) == LOW; // similar logic as above. This logic avoids "if condition = true, return true / if condition = false, return false"
}

void increaseTargetTemp()
{ // Increase the target temperature
  if (target_t < target_max)
  {                        // If target temperature is less than maximum temperature
    target_t += step_size; // increase target temperature by predefined step size
  }
}

void decreaseTargetTemp()
{ // Decrease the target temperature
  if (target_t > target_min)
  {                        // If target temperature is more than minimum temperature
    target_t -= step_size; // decrease target temperature by predefined step size
  }
}

void setup()
{
  Serial.begin(9600);
  sensors.begin();

  // pin modes
  pinMode(TEMPPIN, INPUT_PULLUP);
  pinMode(HEATPIN, OUTPUT);
  pinMode(UpButtonPin, INPUT_PULLUP);   // UpButtonPin defined as input with pull-up resistor
  pinMode(DownButtonPin, INPUT_PULLUP); // DownButtonPin defined as input with pull-up resistor
                                        // INPUT_PULLUP sets the pin by default to HIGH -> to activate it must be set to LOW by connecting to GND
                                        // This reduces/prevents disturbances in the button signal, as these can easily be "falsely on" at low voltages
                                        // see function "isUpButtonPressed()" and "isDownButtonPressed()" -> switch to LOW

  u8g2.begin();
}

void loop()
{
  // read temperature
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(target_t);
  Serial.println("°C");

  if (isUpButtonPressed())
  {                       // If the UpButton is pressed
    increaseTargetTemp(); // increase target temperature
  }
  if (isDownButtonPressed())
  {                       // If the DownButton is pressed
    decreaseTargetTemp(); // decrease target temperature
  }

  if (t < (target_t - (target_range / 2)))
  {                              // If current temperature is less than target temperature - half the temperature range
    digitalWrite(HEATPIN, HIGH); // turn the HEATPIN on
  }
  if (t > (target_t + (target_range / 2)))
  {                             // If current temperature is more than target temperature + half the temperature range
    digitalWrite(HEATPIN, LOW); // turn the HEATPIN off
  }

  // Convert 'target_t' and 't' to a string before passing it to 'drawStr'
  char target_t_str[6];
  dtostrf(target_t, 4, 1, target_t_str);
  char t_str[6];
  dtostrf(t, 4, 1, t_str);

  // Display has a resolution of 128x64 pixels
  u8g2.clearBuffer();
  // Font has a height of 8 pixels
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.drawUTF8(35, 12, "FERMENTATION BOX :)");
  u8g2.drawFrame(0, 0, 128, 16); // Box around the first row
  
  u8g2.drawStr(5, 28, "Target Temperature :");
  u8g2.drawStr(5, 38, target_t_str);
  u8g2.drawFrame(0, 17, 128, 24); // Box around the second and third rows
  
  u8g2.drawStr(5, 52, "Current Temperature :");
  u8g2.drawStr(5, 62, t_str);
  u8g2.drawFrame(0, 41, 128, 23); // Box around the fourth and fifth rows
  
  u8g2.sendBuffer();

  // wait a bit
  delay(200);
}