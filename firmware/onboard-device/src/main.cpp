#include <Arduino.h>
#include "defines.h"
#include "mpu.h"

unsigned long now=0;
unsigned long previous = 0;
unsigned long interval = 500;
bool led_state = 0;

char acc_buffer[20];
char angles_buffer[20];
#define LEDPIN 5

MPU6050 imu(MPU_ADDRESS, MPU_ACCEL_RANGE, GYRO_RANGE); 
float pitch, roll;

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    imu.init();
    pinMode(LEDPIN, OUTPUT);
}

void blink() {
    now = millis();
    if((now - previous) > interval) {
        led_state = !led_state;
        previous=now;
        digitalWrite(LEDPIN, led_state);   
    }

}
    

void loop() {
    blink();
    // float ax = imu.readXAcceleration();
    // float ay = imu.readYAcceleration();
    // float az = imu.readZAcceleration();
    roll = imu.getRoll();
    pitch = imu.getPitch();

    // sprintf(acc_buffer, "%.2f,%.2f,%.2f\r\n",ax,ay,az);
    sprintf(angles_buffer, "%.2f,%.2f\n", roll, pitch);

    Serial.print(acc_buffer);
    Serial.println(angles_buffer);

}