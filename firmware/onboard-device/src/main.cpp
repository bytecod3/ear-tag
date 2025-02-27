#include <Arduino.h>
#include "defines.h"
#include "mpu.h"

unsigned long now=0;
unsigned long previous = 0;
int interval = 500;
bool led_state = LOW;

char acc_buffer[20];
char angles_buffer[50];
char f_p_str[10];
#define LEDPIN 5

MPU6050 imu(MPU_ADDRESS, MPU_ACCEL_RANGE, GYRO_RANGE); 
float pitch, roll, f_pitch, f_roll;
float acc_mag_raw, acc_mag_filtered;
// unsigned long currentSampleMillis = 0;
unsigned long lastSampleMillis = 0;
unsigned long sample_interval = 10; // 10 ms -> 100Hz

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    imu.init();
    pinMode(LEDPIN, OUTPUT);
} 

void loop() {

    // simple blink for status indication
    now = millis();
    if((now - previous) >= interval) {
        previous=now;
        led_state = !led_state;
        digitalWrite(LEDPIN, led_state);  
    }

    /**
     * ACTIVITY MONITOR
     */
    // read raw acceleration values 
    float ax = imu.readXAcceleration();
    // float ay = imu.readYAcceleration();
    // float az = imu.readZAcceleration();

    // filter acceleration values 
    float ax_filtered = imu.movingAverageFilter(ax);
    //Serial.print(ax); Serial.print(",");Serial.println(ax_filtered);

    // get sample time for the IMU data at 100HZ sample rate
    if((now-lastSampleMillis) >= sample_interval) {
        lastSampleMillis = now;

        // filtered angles 
        f_pitch = imu.filterPitch(sample_interval);
        f_roll = imu.filterRoll(sample_interval);
        sprintf(angles_buffer, "%.2f,%.2f,%.2f,%.2f\n", pitch, f_pitch, roll, f_roll);
        // dtostrf(f_pitch, 4, 2, f_p_str);
    }

    // compute magnitude of acceleration 
    acc_mag_raw = imu.computeRawAccelerationMagnitude();
    acc_mag_filtered =  imu.computeAccelerationMagnitude();
    Serial.print(acc_mag_raw); Serial.print(","); Serial.println(acc_mag_filtered);

    /**
     * END OF ACTIVITY MONITOR
     */
    

}