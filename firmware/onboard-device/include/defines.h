#ifndef DEFINES_H
#define DEFINES_H

#define SERIAL_BAUDRATE 115200

#define SERIAL_DEBUG 1
#if SERIAL_DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x,y) Serial.printf(x,y)
#else
#define debug(x)
    #define debugln(x)
    #define debugf(x,y)
#endif

/* MPU config parameters */
#define MPU_ADDRESS 0x68
#define MPU_ACCEL_RANGE 16
#define GYRO_RANGE 1000 /* 1000 deg/s */
#define WIRE_SEND_STOP 0

#define GPS_BAUDRATE 9600
#define GPS_TX 17
#define GPS_RX 16
#define GMT_OFFSET 3

#define LORA_FREQUENCY 433E6
#define LORA_CS 25
#define LORA_RST    26
#define LORA_DIO    27
#define LORA_DELAY 3000

#endif // DEFINES_H