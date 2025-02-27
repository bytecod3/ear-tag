#include "mpu.h"
#include <Arduino.h>

// constructor
MPU6050::MPU6050(uint8_t address, uint32_t accel_fs_range, uint32_t gyro_fs_range) {
    this->_address = address;
    this->_accel_fs_range = accel_fs_range;
    this->_gyro_fs_range = gyro_fs_range;

}

// initialize the MPU6050 
uint8_t  MPU6050::init() {
    // initialize the MPU6050 
    bool x = Wire.begin(static_cast<int>(SDA), static_cast<int>(SCL));
    Wire.beginTransmission(this->_address);
    Wire.write(PWR_MNGMT_1); // power on the device 
    Wire.write(RESET);
    Wire.endTransmission(true);
    delay(50);

    // configure the gyroscope
    Wire.beginTransmission(this->_address);
    Wire.write(GYRO_CONFIG);
    if(this->_gyro_fs_range == 250) {
        Wire.write(SET_GYRO_FS_250);
    } else if(this->_gyro_fs_range == 500) {
        Wire.write(SET_GYRO_FS_500);
    } else if (this->_gyro_fs_range == 1000) {
        Wire.write(SET_GYRO_FS_1000);
    } else if (this->_gyro_fs_range == 2000) {
        Wire.write(SET_GYRO_FS_2000);
    }
    Wire.endTransmission(true);
    delay(50);

    // configure the accelerometer
    Wire.beginTransmission(this->_address);
    Wire.write(ACCEL_CONFIG);

    if(this->_accel_fs_range == 2) {
        Wire.write(SET_ACCEL_FS_2G);
    } else if (this->_accel_fs_range == 4) {
         Wire.write(SET_ACCEL_FS_4G);
    }  else if (this->_accel_fs_range== 8) {
         Wire.write(SET_ACCEL_FS_8G);
    }  else if (this->_accel_fs_range == 16) {
         Wire.write(SET_ACCEL_FS_16G);
    }
    Wire.endTransmission(true);

    // TODO: ceck initialization properly
    if (x) {
        Serial.println(F("[+]MPU6050 init OK."));
        return 1;
    }
    else {
        Serial.println(F("[-]MPU6050 init failed."));
        return 0;
    }
    
}

/**
 * Read X axiS acceleration
*/
float MPU6050::readXAcceleration() {
    Wire.beginTransmission(this->_address);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, static_cast<int>(WIRE_SEND_STOP));
    this->acc_x = Wire.read()<<8 | Wire.read();

    // divide by the respective factors
    if(this->_accel_fs_range == 2) {
        this->acc_x_real = (float) acc_x / ACCEL_FACTOR_2G;
    } else if(this->_accel_fs_range == 4) {
        this->acc_x_real = (float) acc_x / ACCEL_FACTOR_4G; 
    } else if(this->_accel_fs_range == 8) {
        this->acc_x_real = (float) acc_x / ACCEL_FACTOR_8G;
    } else if(this->_accel_fs_range == 16) {
        this->acc_x_real = (float) acc_x / ACCEL_FACTOR_16G;
    }

    return this->acc_x_real;

}

/**
 * Read Y acceleration
*/
float MPU6050::readYAcceleration() {
    Wire.beginTransmission(this->_address);
    Wire.write(ACCEL_YOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, WIRE_SEND_STOP);
    this->acc_y = Wire.read()<<8 | Wire.read();

    // divide by the respective factors
    if(this->_accel_fs_range == 2) {
        this->acc_y_real = (float) acc_y / ACCEL_FACTOR_2G;
    } else if(this->_accel_fs_range == 4) {
        this->acc_y_real = (float) acc_y / ACCEL_FACTOR_4G; 
    } else if(this->_accel_fs_range == 8) {
        this->acc_y_real = (float) acc_y / ACCEL_FACTOR_8G;
    } else if(this->_accel_fs_range == 16) {
        this->acc_y_real = (float) acc_y / ACCEL_FACTOR_16G;
    }

    return this->acc_y_real;
    
}

/**
 * Read Z acceleration
*/
float MPU6050::readZAcceleration() {
    Wire.beginTransmission(this->_address);
    Wire.write(ACCEL_ZOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, WIRE_SEND_STOP);
    this->acc_z = Wire.read()<<8 | Wire.read();

    // divide by the respective factors
    if(this->_accel_fs_range == 2) {
        this->acc_z_real = (float) acc_z / ACCEL_FACTOR_2G;
    } else if(this->_accel_fs_range == 4) {
        this->acc_z_real = (float) acc_z / ACCEL_FACTOR_4G; 
    } else if(this->_accel_fs_range == 8) {
        this->acc_z_real = (float) acc_z / ACCEL_FACTOR_8G;
    } else if(this->_accel_fs_range == 16) {
        this->acc_z_real = (float) acc_z / ACCEL_FACTOR_16G;
    }

    return this->acc_z_real;
    
}

/**
 * compute the pitch angle
 * angle along the transverse axis 
 * return roll angle in degrees
*/
float MPU6050::getRoll() {
    // convert the imu readings to m/s^2
    this->acc_x_ms = this->readXAcceleration() * ONE_G;
    this->acc_y_ms = this->readYAcceleration() * ONE_G;
    this->acc_z_ms = this->readZAcceleration() * ONE_G;

    // this->roll_angle = atan2(this->acc_y_ms, this->acc_z_ms);

    this->roll_angle = atan2(this->acc_y_ms, sqrt(this->acc_x_ms*this->acc_x_ms + this->acc_z_ms*this->acc_z_ms));

    return this->roll_angle * TO_DEG_FACTOR;    
}

/**
 * compute the roll angle
 * angle along the longitudinal axis
 * return pitch angle in degrees
*/
float MPU6050::getPitch() {

    // convert the imu readings to m/s^2
    this->acc_x_ms = this->readXAcceleration() * ONE_G;
    this->acc_y_ms = this->readYAcceleration() * ONE_G;
    this->acc_z_ms = this->readZAcceleration() * ONE_G;    

    this->pitch_angle = atan2(this->acc_x_ms, sqrt(this->acc_y_ms*this->acc_y_ms + this->acc_z_ms*this->acc_z_ms));

    // double u = this->acc_x_ms / ONE_G;

    // // clip to [-1, +1] bound before passing to arcsine
    // if( ! ( (u > 1) || (u < -1) )) {
    //     this->pitch_angle = asin(this->acc_x_ms/ONE_G);
    // }

    return this->pitch_angle * TO_DEG_FACTOR;
}

float MPU6050::readXAngularVelocity() {
    Wire.beginTransmission(this->_address);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, static_cast<int>(WIRE_SEND_STOP));
    this->ang_vel_x = Wire.read() << 8 | Wire.read();

    // divide by the configured settings 
    if(this->_gyro_fs_range == 250) {
        this->ang_vel_x_real = (float) ang_vel_x / GYRO_FACTOR_250; 
    } else if (this->_gyro_fs_range == 500) {
        this->ang_vel_x_real = (float) ang_vel_x / GYRO_FACTOR_500; 
    } else if(this->_gyro_fs_range == 1000) {
        this->ang_vel_x_real = (float) ang_vel_x / GYRO_FACTOR_1000; 
    } else if(this->_gyro_fs_range == 2000) {
        this->ang_vel_x_real = (float) ang_vel_x / GYRO_FACTOR_2000; 
    }

    return this->ang_vel_x_real;
}

float MPU6050::readYAngularVelocity() {
    Wire.beginTransmission(this->_address);
    Wire.write(GYRO_YOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, static_cast<int>(WIRE_SEND_STOP));
    this->ang_vel_y = Wire.read() << 8 | Wire.read();

    // divide by the confiured settings 
    if(this->_gyro_fs_range == 250) {
        this->ang_vel_y_real = (float) ang_vel_y / GYRO_FACTOR_250; 
    } else if (this->_gyro_fs_range == 500) {
        this->ang_vel_y_real = (float) ang_vel_y / GYRO_FACTOR_500; 
    } else if(this->_gyro_fs_range == 1000) {
        this->ang_vel_y_real = (float) ang_vel_y / GYRO_FACTOR_1000; 
    } else if(this->_gyro_fs_range == 2000) {
        this->ang_vel_y_real = (float) ang_vel_y / GYRO_FACTOR_2000; 
    }

    return this->ang_vel_y_real;
}

float MPU6050::readZAngularVelocity() {
    Wire.beginTransmission(this->_address);
    Wire.write(GYRO_ZOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(static_cast<int>(this->_address), 2, static_cast<int>(WIRE_SEND_STOP));
    this->ang_vel_z= Wire.read() << 8 | Wire.read();

    // divide by the confiured settings 
    if(this->_gyro_fs_range == 250) {
        this->ang_vel_z_real = (float) ang_vel_z / GYRO_FACTOR_250; 
    } else if (this->_gyro_fs_range == 500) {
        this->ang_vel_z_real = (float) ang_vel_z / GYRO_FACTOR_500; 
    } else if(this->_gyro_fs_range == 1000) {
        this->ang_vel_z_real = (float) ang_vel_z / GYRO_FACTOR_1000; 
    } else if(this->_gyro_fs_range == 2000) {
        this->ang_vel_z_real = (float) ang_vel_z / GYRO_FACTOR_2000; 
    }

    return this->ang_vel_z_real;
}

/**
 * @brief compute the magnitude of acceleration of the module
 */
float MPU6050::computeAccelerationMagnitude() {
    float xa = this->readXAcceleration();
    float ya = this->readYAcceleration();
    float za = this->readZAcceleration();

    // filter the raw values using moving average filter
    float xa_filt = this->movingAverageFilter(xa);
    float ya_filt = this->movingAverageFilter(ya);
    float za_filt = this->movingAverageFilter(za);
    
    float mag = sqrt(pow(xa_filt,2) + pow(ya_filt,2) + pow(za_filt,2));
    return mag;
}

float MPU6050::computeRawAccelerationMagnitude() {
    float xa = this->readXAcceleration();
    float ya = this->readYAcceleration();
    float za = this->readZAcceleration();
    
    float mag = sqrt(pow(xa,2) + pow(ya,2) + pow(za,2));
    return mag;
}

/**
 * perform sensor fusion
 * perfom complementary filter to remove accelerometer high frequrecny noise 
 * remove low frequency noise from gyroscope and fuse the sensors 
*/
float MPU6050::filterPitch(unsigned long sample_time) {
    // sample time is in milliseconds, divide by 1000 to convert to seconds 
    this->filtered_pitch = ALPHA * (this->filtered_pitch + this->readXAngularVelocity() * sample_time/1000) + (1-ALPHA)*this->readXAcceleration();
    return this->filtered_pitch;
}

float MPU6050::filterRoll(unsigned long sample_time) {
    this->filtered_roll = ALPHA * (this->filtered_roll + this->readYAngularVelocity() * sample_time/1000) *  (1-ALPHA)*this->readYAcceleration();  
    return this->filtered_roll;
}

float MPU6050::movingAverageFilter(float value) {
    values[this->mov_avg_index] = value;
    this->mov_avg_index = (this->mov_avg_index + 1) % FILTER_BUFFER_SIZE;

    float sum = 0;
    for(int i=0; i< FILTER_BUFFER_SIZE; i++) {
        sum += values[i];
    }

    return sum/FILTER_BUFFER_SIZE; // averaged value
}

// float MPU6050::readTemperature() {
//     // write to temp register
//     Wire.beginTransmission(this->_address);
//     Wire.write(TEMP_OUT_H);
//     Wire.endTransmission(true);

//     Wire.requestFrom(static_cast<int>(this->_address), 2, WIRE_SEND_STOP);
//     this->temp = Wire.read()<<8 | Wire.read();

//     // temperature conversion formula 
//     // temp = (TEMP_OUT_VALUE as a signed quantity)/340 +36.53
// }


