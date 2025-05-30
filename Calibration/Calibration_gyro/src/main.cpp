#include <Adafruit_SensorLab.h>
#include <Arduino.h>
#include <Wire.h>

Adafruit_SensorLab lab;

#define NUMBER_SAMPLES 500

Adafruit_Sensor *gyro;
sensors_event_t event;

float min_x, max_x, mid_x;
float min_y, max_y, mid_y;
float min_z, max_z, mid_z;

void setup(void) {
  Wire.begin(21, 37);
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  
  Serial.println(F("Sensor Lab - Gyroscope Calibration!"));
  lab.begin();
  
  Serial.println("Looking for a gyro");
  gyro = lab.getGyroscope();
  if (! gyro) {
    Serial.println(F("Could not find a gyro, check wiring!"));
    while(1) delay(10);
  }
  gyro->printSensorDetails();
  delay(100);

  gyro->getEvent(&event);
  min_x = max_x = event.gyro.x;
  min_y = max_y = event.gyro.y;
  min_z = max_z = event.gyro.z;
  delay(10);

  Serial.println(F("Place gyro on flat, stable surface!"));

  Serial.print(F("Fetching samples in 3..."));
  delay(1000);
  Serial.print("2...");
  delay(1000);
  Serial.print("1...");
  delay(1000);
  Serial.println("NOW!");
  
  float x, y, z;
  for (uint16_t sample = 0; sample < NUMBER_SAMPLES; sample++) {
    gyro->getEvent(&event);
    x = event.gyro.x;
    y = event.gyro.y;
    z = event.gyro.z;
    Serial.print(F("Gyro: ("));
    Serial.print(x); Serial.print(", ");
    Serial.print(y); Serial.print(", ");
    Serial.print(z); Serial.print(")");

    min_x = min(min_x, x);
    min_y = min(min_y, y);
    min_z = min(min_z, z);
  
    max_x = max(max_x, x);
    max_y = max(max_y, y);
    max_z = max(max_z, z);
  
    mid_x = (max_x + min_x) / 2;
    mid_y = (max_y + min_y) / 2;
    mid_z = (max_z + min_z) / 2;

    Serial.print(F(" Zero rate offset: ("));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.print(mid_z, 4); Serial.print(")");  
  
    Serial.print(F(" rad/s noise: ("));
    Serial.print(max_x - min_x, 3); Serial.print(", ");
    Serial.print(max_y - min_y, 3); Serial.print(", ");
    Serial.print(max_z - min_z, 3); Serial.println(")");   
    delay(10);
  }
  Serial.println(F("\n\nFinal zero rate offset in radians/s: "));
  Serial.print(mid_x, 4); Serial.print(", ");
  Serial.print(mid_y, 4); Serial.print(", ");
  Serial.println(mid_z, 4);
}



void loop() {
  delay(10); 
}
//primeira vez: 0.0370 0.0766 0.0255
//segunda vez: -0.0044 0.0813 0.0318
//terceira vez: -0.0045 0.0759 0.0322
//quarta vez: -0.0066 0.0744 0.0322
//quinta vez: -0.0051 0.0761 0.0330
//sexta vez: -0.0050 0.0775 0.0324
//setima vez: -0.0061 0.0755 0.0330
//oitava vez: -0.0083 0.0724 0.0322
//nona vez: -0.0017 0.0847 0.0331
//décima vez: -0.0071 0.0743 0.0330
//décima primeira vez: -0.0066 0.0777 0.0328
//décima segunda vez: -0.0050, 0.0774, 0.0340
//décima terceira vez: -0.0007, 0.0855, 0.0332
//décima quarta vez: -0.0051 0.0735 0.0324
//décima quinta vez:  -0.0049, 0.0773, 0.0329
//décima sexta vez: -0.0066, 0.0785, 0.0334
//décima sétima vez: -0.0033, 0.0777, 0.0331
//décima oitava vez: -0.0123, 0.0464, 0.0187
//décima nona vez:    -0.0027, 0.0793, 0.0333
//vigésima vez: 0.0015, 0.0822, 0.0332
//offset x: -0.005125
//offset y: 0.076175
//offset z: 0.03181