#include <LiquidCrystal.h> // LCD library

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

const int SW_pin = 2;   // Joystick button (digital input)
const int X_pin = 0;    // Joystick X-axis (analog)
const int Y_pin = 1;    // Joystick Y-axis (analog)

const int RED = 6;      // RGB LED (red channel - PWM)
const int GREEN = 5;    // RGB LED (green channel - PWM)
const int BLUE = 3;     // RGB LED (blue channel - PWM)

const int potPin = A3;  // Potentiometer (brightness control)

// Mode meanings:
// 0 → Red/Green joystick control
// 1 → Red/Blue joystick control
// 2 → Automatic RGB rainbow fade
const int numModes = 3;

int mode = 0;
int lastButtonState = 1;
int lastMode = -1;

float hue = 0;

// Converts HSV to RGB (0–255)
void hsvToRgb(float h, float s, float v, int &r, int &g, int &b) {
  int i = int(h / 60) % 6;
  float f = (h / 60) - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  float r_f, g_f, b_f;

  switch(i) {
    case 0: r_f = v; g_f = t; b_f = p; break;
    case 1: r_f = q; g_f = v; b_f = p; break;
    case 2: r_f = p; g_f = v; b_f = t; break;
    case 3: r_f = p; g_f = q; b_f = v; break;
    case 4: r_f = t; g_f = p; b_f = v; break;
    case 5: r_f = v; g_f = p; b_f = q; break;
  }

  r = r_f * 255;
  g = g_f * 255;
  b = b_f * 255;
}

void setup() {
  pinMode(SW_pin, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(SW_pin, HIGH); // enable pull-up

  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop() {

  int horizontalValue = map(analogRead(X_pin), 0, 1023, 0, 255);
  int verticalValue   = map(analogRead(Y_pin), 0, 1023, 0, 255);
  int potValue        = map(analogRead(potPin), 0, 1023, 0, 100);

  int buttonState = digitalRead(SW_pin);

  // Mode switching (button press)
  if(lastButtonState == HIGH && buttonState == LOW) {
    mode = (mode + 1) % numModes;
    delay(200); // debounce
  }
  lastButtonState = buttonState;

  // LCD update only when mode changes
  if(mode != lastMode) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Mode: ");
    lcd.print(mode);

    lcd.setCursor(0, 1);
    switch(mode) {
      case 0:
        lcd.print("R/G w Bright");
        break;
      case 1:
        lcd.print("R/B w Bright");
        break;
      case 2:
        lcd.print("RGB Fade");
        break;
    }

    lastMode = mode;
  }

  // Mode behavior
  switch(mode) {

    case 0: // Red + Green
      analogWrite(RED, horizontalValue * potValue / 100);
      analogWrite(GREEN, verticalValue * potValue / 100);
      analogWrite(BLUE, 0);
      break;

    case 1: // Red + Blue
      analogWrite(RED, horizontalValue * potValue / 100);
      analogWrite(BLUE, verticalValue * potValue / 100);
      analogWrite(GREEN, 0);
      break;

    case 2: { // Rainbow fade
      static unsigned long lastUpdate = 0;

      if(millis() - lastUpdate > 10) {
        hue += 1;
        if(hue >= 360) hue = 0;

        int r, g, b;
        hsvToRgb(hue, 1.0, potValue / 100.0, r, g, b);

        analogWrite(RED, r);
        analogWrite(GREEN, g);
        analogWrite(BLUE, b);

        lastUpdate = millis();
      }
      break;
    }
  }
}