#include <Arduino.h>
#include <TinyGPSPlus.h>
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

/**
 * GPS variables
 */
HardwareSerial gpsSerial(2); // GPS uses hardware serial on line 16 and 17
TinyGPSPlus gps;
typedef struct gpsPacket{
    double latitude;
    double longitude;
    uint8_t day;
    uint8_t month;
    uint8_t hr;
    uint8_t minute;
    uint8_t sec;
    uint16_t year;
} GPS_PACKET;

char gps_buffer[10];
unsigned long GPS_current_millis = 0;
unsigned long GPS_last_millis = 0;
unsigned long GPS_sample_interval = 5000; // TODO: set longer frequency

GPS_PACKET gps_packet;

/**
 * End of GPS variables
 */

/**
 * function prototypes
 */
 void GPS_init();
 void GPS_get_coordinates();

 /**
  * Function implementation
  */
  void GPS_init() {
      gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX, GPS_TX);
      debugln("GPS initialized OK");
  }

void GPS_get_coordinates() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());

        if (gps.location.isUpdated()){
            // Get location
            //debug("Location: ");
            if(gps.location.isValid()) {
                //latitude = gps.location.lat();
                //Serial.print(gps.location.lat(), 2);
                //debug(F(","));

                //longitude = gps.location.lng();
                //Serial.print(gps.location.lng(), 2);

                gps_packet.latitude = gps.location.lat();
                gps_packet.longitude = gps.location.lng();

            } else {
                //debug(F("INVALID"));
            }

            // Get time and date
            //debug(F("Date/time: "));
            if(gps.date.isValid()) {
                //month = gps.date.month();
                //debug(month);
                //debug(F("/"));

                //day = gps.date.day();
                //debug(day);
                //debug(F("/"));

                //year = gps.date.year();
                //debug(year);
                gps_packet.month = gps.date.month();
                gps_packet.day = gps.date.day();
                gps_packet.year = gps.date.year();
            } else {
                //debug(F("INVALID"));
            }

            // time
            //debug(F(" "));
            if (gps.time.isValid()) {

                //hr = gps.time.hour();
                //if (hr < 10) debug(F("0"));
                //debug(hr);

                //minute = gps.time.minute();
                //if (mint < 10) debug(F("0"));
                //debug(mint);

                //sec = gps.time.second();
                //if (sec < 10) debug(F("0"));
                //debug(sec);

                gps_packet.hr = gps.time.hour() + GMT_OFFSET;
                gps_packet.minute = gps.time.minute();
                gps_packet.sec = gps.time.sec();

            } else {
                //debug(F("INVALID"));
            }

            //debugln();
        }
    }

  }


void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    imu.init();
    pinMode(LEDPIN, OUTPUT);

    GPS_init();
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

    /**
     * GPS DATA COLLECTION
     * Data is read at a frequency defined by the DATA_UPDATE frequency value
     */
    GPS_current_millis = millis();
    if((GPS_current_millis - GPS_last_millis) >= GPS_sample_interval) {
        GPS_get_coordinates();
        GPS_last_millis = GPS_current_millis;
    }

    /**
     * END OF GPS DATA
     */

    

}