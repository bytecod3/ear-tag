#include <Arduino.h>

#define USER_LED 14

void setup() {
  pinMode(USER_LED, OUTPUT);
}

void loop() {
  digitalWrite(USER_LED, HIGH);
  delay(300);
  digitalWrite(USER_LED, LOW);
  delay(300);

}
