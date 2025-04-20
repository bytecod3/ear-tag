#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <LoRa.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <math.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "defines.h"
#include "mpu.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

char tag_id[] = "TAG_001";
char data_packet[200]; // to hold the data packet for transmission

unsigned long now=0;
unsigned long led1_previous = 0;
int led1_interval = 500;
bool led1_state = LOW;

unsigned long led2_previous = 0;
int led2_interval = 100; // determined by the state we are in
bool led2_state = LOW;

// data sending timing variables
unsigned long last_http_time = 0;
unsigned long current_http_time = 0;
unsigned long http_send_interval = 5000;

char acc_buffer[20];
char angles_buffer[50];
char f_p_str[10];

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
float base_lat = -1.50;
float base_long = 37.0144;

GPS_PACKET gps_packet;

uint8_t geo_fence_proximity = 0;

const float simulatedCoordinates[][2] = {
    {-1.2921, 36.8319},  // center point
    { -1.2921, 36.8219 },  // simulated point 1 -> 1.17km from center
    { -1.3000, 36.8000 },  // Simulated Point 2 -> 3.65km from center
    { -1.3100, 36.9100 },   // Simulated Point 3 -> 8.9km from center
    { -1.3100, 36.9900 }   // Simulated Point 3 -> 17.68km from center
};

/**
 * These represent the radiuses we need to check every minute the animal
 * is grazing
 *
 * distance in KM
 */
enum GEO_FENCE_LIMITS {
  FENCE_1 = 2,
  FENCE_2 = 5,
  FENCE_3 = 10,
  OUTSIDE_FENCE = 15
};

/**
 * End of GPS variables
 */

/*
* LORA variables
 */
char lora_msg[100];

/**
* Other variables
*/
enum DEVICE_STATES{
  OPERATIONAL = 1,
  SIMULATION
};

int current_state;
int button_state = HIGH;
int last_button_state = HIGH;
unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 30;
int press_count = 0;

const char* ssid = "Eduh";
const char* password = "password2";

//const char* ssid = "Gakibia unit 3";
//const char* password = "password";

const char* server_url = "http://192.168.46.1:3000/api/location";

/**
 * function prototypes
 */
 void WIFI_configure();
 void GPS_init();
 void GPS_get_coordinates();
 void LORA_init();
 void LORA_send_packet(char* msg);
 void WIFI_basic_connection();
 void send_packet_to_server(double lat, double lng);

 /**
  *
  * TODO: add multiple network support
  * add static IP address
  * add reconnection logic
  * add signal strength monitoring
  * add connection timeout
  */

 void WIFI_basic_connection() {
   debugln();
   debug("Connecting to Wifi");
   //debugln(ssid);

   WiFi.begin(ssid, password);

   int attempts = 0;
   while ((WiFi.status() != WL_CONNECTED) && (attempts < 100)) {
     delay(500);
     debugln("connecting...");
     attempts++;
   }

   if (WiFi.status() == WL_CONNECTED) {
     debugln("");
     debugln("WiFi connected");
     debugln("IP address: ");
     debugln(WiFi.localIP());
   } else {
     debugln("");
     debugln("Failed to connect to WiFi");
   }
 }

 void WIFI_provisioning() {
   WiFi.mode(WIFI_STA);
   WiFiManager wm;

   // automatically connect using stored settings
   bool res;
   res = wm.autoConnect("EAR-TAG-WIFI", "password");

   if(!res) {
     debugln("Failed to connect");
   } else {
     debugln("Connected to WIFI");
   }
 }

 /**
  * Send location to server
  */
 void send_packet_to_server(double lat, double lng) {
   debugln("Sending to server...");
   if(WiFi.status() == WL_CONNECTED) {
     HTTPClient http;

     StaticJsonDocument<200> doc;
     doc["device_id"] = tag_id;
     doc["latitude"] = lat;
     doc["longitude"] = lng;
     doc["acc_mag"] = acc_mag_filtered;

     String payload;
     serializeJson(doc, payload);

     http.begin(server_url);
     http.addHeader("Content-Type", "application/json");

     int http_response_code = http.POST(payload);

     if(http_response_code > 0) {
       debugf("HTTP Response code: %d\n", http_response_code);
       String response = http.getString();
       debugln(response);
     } else {
       debugf("Error sending POST: %s\n", http.errorToString(http_response_code).c_str());
       debugln(http_response_code);
     }

     http.end();

   } else {
     debugln("Wifi Not Available");
   }
 }

 /**
* Initialize LORA module
*/
 void initLORA() {
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO);
    if(!LoRa.begin(LORA_FREQUENCY)) {
        debugln("Lora failed to start");
    } else {
        debugln("[+]Lora started OK!");
    }
}

void LORA_send_packet(char* msg) {

    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
    delay(LORA_DELAY);

}

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
                gps_packet.sec = gps.time.second();

            } else {
                //debug(F("INVALID"));
            }

            //debugln();
        }
    }
}

void transmit_to_base_station(char* data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  delay(200);
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    Serial.begin(SERIAL_BAUDRATE);
    //WIFI_provisioning();
    WIFI_basic_connection();
    imu.init();
    initLORA();
    GPS_init();
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(SIMULATE_BUTTON, INPUT);

    // for random number generation
    randomSeed(367);
} 

void loop() {
	
  // get current time
  now = millis();

  // read simulate button
  int reading = digitalRead(SIMULATE_BUTTON);
  if(reading != last_button_state) {
    last_debounce_time = now;
  }

  if((now - last_debounce_time) > debounce_delay) {
    if(reading != button_state) {
      button_state = reading;
      if(button_state == LOW) {
        press_count++;
        debugln("change state");
      }
    }
  }
  
  last_button_state = reading; 
  
  if(press_count == 2) {
    press_count = 0;
  }
  
  debug("Press count: "); debugln(press_count);

  /* change state */
  if(press_count == 0) {
    current_state = DEVICE_STATES::SIMULATION;
  } else if(press_count == 1) {
    current_state = DEVICE_STATES::OPERATIONAL;
  }

  // blink for status indication
  if((now - led1_previous) >= led1_interval) {
      led1_previous=now;
      led1_state = !led1_state;
      digitalWrite(LED1, led1_state);
  }

  /**
   * ACTIVITY MONITOR
   */
  // read raw acceleration values
  float ax = imu.readXAcceleration();
  float ay = imu.readYAcceleration();
  float az = imu.readZAcceleration();

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
  //Serial.print(acc_mag_raw); Serial.print(","); Serial.println(acc_mag_filtered);

  /**
   * END OF ACTIVITY MONITOR
   */
  // read GPS data
   /**
   * GPS DATA COLLECTION
   * Data is read at a frequency defined by the DATA_UPDATE frequency value
     */
    GPS_current_millis = millis();
    if((GPS_current_millis - GPS_last_millis) >= GPS_sample_interval) {
      GPS_get_coordinates();
      GPS_last_millis = GPS_current_millis;
    }
  
  
  if(current_state == DEVICE_STATES::OPERATIONAL) {
    led2_interval = 1000;
    if((now - led1_previous) >= 1000) {
      led1_previous=now;
      led1_state = !led1_state;
      digitalWrite(LED2, led1_state);
    }

  } else if(current_state == DEVICE_STATES::SIMULATION) {
    led2_interval = 250;
  }

  // LED 2 BLINK
  if((now - led2_previous) >= led2_interval) {
    led2_previous=now;
    led2_state = !led2_state;
    digitalWrite(LED2, led2_state);
  }

  // send data to server
  unsigned long current_http_time = millis();
  if((current_http_time - last_http_time) >= http_send_interval) {
    last_http_time = current_http_time;
    
    if(current_state == DEVICE_STATES::SIMULATION) {
		debugln("SIMULATION");
		// generate random location data
		float lat_offset = (random(0,2001) - 1000) / 10000.0;
		float lng_offset = (random(0,2001) - 1000) / 10000.0;
		send_packet_to_server(base_lat + lat_offset, base_long + lng_offset);
	} else if (current_state == DEVICE_STATES::OPERATIONAL) {
		debugln("OPERATIONAL");
		debug("Op coords:");debug(gps_packet.latitude); debug(","); debugln(gps_packet.longitude);
		send_packet_to_server(gps_packet.latitude, gps_packet.longitude);
		
	}

  }

  /**
  * Package LORA packet
  * tag_id
  * mode (operational or simulation)
  * day
  * month,
  * year,
  * hour,
  * minute,
  * second,
  * latitude
  * longitude,
  * geo_fence_proximity
  * acceleration x,
  * acceleration y
  * acceleration z,
  * acceleration_magnitude
  *
  */
  sprintf(data_packet,
          "%s,%d,%d,%d,%d,%d,%d,%.4f,%.4f,%d,%.2f,%.2f,%.2f,%.2f",
          tag_id,
          gps_packet.day,
          gps_packet.month,
          gps_packet.year,
          gps_packet.hr,
          gps_packet.minute,
          gps_packet.sec,
          gps_packet.latitude,
          gps_packet.longitude,
          geo_fence_proximity,
          ax,
          ay,
          az,
          acc_mag_filtered
          );

  transmit_to_base_station(data_packet);

}
