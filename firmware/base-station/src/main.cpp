#include <Arduino.h>
#include <LoRa.h>

const int cs_pin = 4;
const int reset_pin = 2;
const int irq_pin = 22; // DIO0 on HOPE RF LORA MODULE - pin must have hardware interrupt
int LED = 26;
const int BUZZER_PIN = 25;

unsigned long now= 0;
unsigned long last_blink_time = 0;
unsigned long blink_interval = 100;
boolean led_state = LOW;
/**
* Initialize LORA module
 */
void initLORA() {
  LoRa.setPins(cs_pin, reset_pin, irq_pin);
  if(!LoRa.begin(433E6)) {
    Serial.println("Lora failed to start");
  } else {
    Serial.println("[+]Lora started OK!");
  }
}

void  blinkLED() {
  digitalWrite(LED, HIGH);
  delay(blink_interval);
  digitalWrite(LED, LOW);
  delay(blink_interval);
}

void decode_lora_msg() {

}

void setup() {
  Serial.begin(115200);
  initLORA();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
}

void loop() {

  now = millis();
  if((now - last_blink_time) > blink_interval) {
    last_blink_time = now;
    led_state = !led_state;
    digitalWrite(LED, led_state);
  }

  /*
   * Receive LORA packet
   *
   */
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String packetData = "";

    while (LoRa.available()) {
      packetData += (char)LoRa.read();
    }

    Serial.println("Recvd: " + packetData);

    // decode message
//    char pckt_arr[packetData.length()+1];
//    strcpy(pckt_arr, packetData.c_str());
//
//    decode_msg(pckt_arr);

  }

  /* check the proximity to fence in levels
   * level 1 -> <1km (OK)
   * level 2 -> <4km ( OK)
   * level 3 -> <10km (unsafe) buzz slow, blink fast
   * level 4 -> >10km (stray animal) buzz fast, blink faster
   * */

}