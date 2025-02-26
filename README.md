# ear-tag
Ear tag with LORA

## Introduction 
This proejct implements a catle ear tag using LORA technology. The purpose of the tag is to monitor location and activity of the 
cattle. It is tailored to open ranch setups where the cattle have a huge chunk of land to graze.

This ear tag also monitors the activity level of the cattle and uses a custom algrithm to detect shcok and abnormal vibrations 
to tell whether the animal fell, is still lying down etc.

## How it works
The device reads accelerations from an accelerometer sensor and does data processing to translate them into useful data like shock and activity levels.
Using a GPS, we can monitor the real-time location of the animal. 

This data is transmitted to a base station via a LORA gateway and visualized on the farmer's device, either 
phone and/or desktop setup.

## Measuring acceleration and activity monitoring
MPU6050 measures acceleration using its on-chip accelerometer with four programmable full scale ranges of ±2g, ±4g, ±8g and ±16g that can be set by the user.The X, Y and Z are relative to how the chip sits on the module as shown to the right and will ultimately be dependent on the orientation of the module 
on the device.

### Measuring rotation 
MPU6050 measures rotation using its on-chip gyroscope with four programmable full scale ranges of ±250°/s, ±500°/s, ±1000°/s and ±2000°/s that can be set by the user. The axis of rotation are relative the the X,Y and Z shown to the right.

### Using the I2C Interface
The module uses I2C interface for communications with MCU.It supports two different I2C addresses;0x68 and 0x69.That allows two devices to be used on the same bus or in case there is an address conflict with another device on the bus. The AD0 pin determines the I2C address to use.This pin has built in 4.7k pull-down resistor on module.if the pin is left unconnected,the line will be pulled low and the default I2C address will be 0X68.To select 0x69, connect the ADO pin to 3.3V. The SCL and SDA pins connect to A5 and A4 pins in analog input pins on the arduino board. The Auxiliary I2C address with pins labeled EDA and ECL are an I2C bus controlled by the MPU-9250 so that it can communicate directly with other sensors so that it can get additional information for its internal computations.

### Pitch and roll calculation 
Acceleration is read from all axes(x, y, z). The read values are then used to calculate the pitch and roll angles as shown below.

#### Pitch calculation
```c
pitch = 180 * atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ))/PI 
```

#### Roll calculation 
```c
roll = 180 * atan2(accelY,sqrt(accelX*accelX + accelZ*accelZ))/PI
```

### Filtering and correcting MPU6050 data for accuracy 
After obtaining MPU6050 accelerometer and gyroscope readings, addressing sensor drift, noise, and measurement errors is essential for accurate motion tracking and orientation estimation. [1]

Animal exhibit distinct diurnal patterns depending on the time of day[2]. This means that in the morning and evening they show high activity but during nightime they are ruminating and therefore low activiy.  To detect the shocks, a High pass filter(HPF) is used to discard all data/noise below a given cutoff frequency. 

This ear tag uses a simple complemetary filter to do the data cleanup. 
The formula is shown below: 

```c


```

## Shock detection 
In case the animal falls, we can use the MPU6050 to detect the shock and apply corrective measures. The accelerometer used is configured to measure up to (+/-)16g range.

### Resultant acceleration calculation 
To be able to register sudden animal activity, we have to keep track of the resultant acceleration. 

## Schematics 

## PCB designs
The PCBs are designed using EasyEDA Pro. Please download EasyEDA Pro here -> https://easyeda.com/page/download

## 3D casing designs 

## Viewing the system flowcharts and diagrams
To open the flowcharts and diagrams, download draw.io app here -> https://github.com/jgraph/drawio-desktop/releases/tag/v25.0.2
Choose the WindowsInstaller version


### References
[1] 
[2] [text](https://www.mdpi.com/2076-2615/14/2/301)