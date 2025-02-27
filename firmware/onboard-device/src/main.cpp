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
float pitch, roll, f_pitch, f_roll;
float currentSampleMillis = 0;
float lastSampleMillis = 0;
unsigned long sample_interval = 10; // 10 ms -> 100Hz

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

    // get sample time for the IMU data at 100HZ sample rate
    currentSampleMillis = millis();
    if(currentSampleMillis-lastSampleMillis >= sample_interval) {
        lastSampleMillis = currentSampleMillis;

        // filtered angles 
        f_pitch = imu.filterPitch(sample_interval);
        f_roll = imu.filterRoll(sample_interval);
        sprintf(angles_buffer, "%.2f,%.2f\n", pitch, f_pitch);

    }

    // sprintf(acc_buffer, "%.2f,%.2f,%.2f\r\n",ax,ay,az);
    

    //Serial.print(acc_buffer);
    Serial.println(angles_buffer);

}