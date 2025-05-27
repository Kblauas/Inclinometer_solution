
#include <Wire.h>
#include <Arduino.h>
#include <PL_ADXL355.h>

//==============================================================================

// Pin connections for using SPI or I2C interface
// ADXL355 pin    Arduino pin
//                SPI     I2C
// CS/SCL          2      SCL
// MOSI/SDA       MOSI    SDA
// MISO/ASEL      MISO    GND
// SCLK/Vssio     SCLK    GND

PL::ADXL355 adxl355;
//uint8_t spiCsPin = 2;
uint8_t i2cAddress = 0x53;

// ADXL355 range: +/- 2 g
auto range = PL::ADXL355_Range::range2g;

//==============================================================================

void setup() {
  // Initialize ADXL355 (uncomment one of the following 2 lines to use either SPI or I2C)
  //adxl355.beginSPI(spiCsPin);
  Wire.begin(21,37);
  adxl355.beginI2C(i2cAddress);

  // Set ADXL355 range
  adxl355.setRange(range);
  // Enable ADXL355 measurement
  adxl355.enableMeasurement();
  
  // Initialize Serial at 115200 kbps
  Serial.begin(115200);
  // Wait for Serial ready state
  while(!Serial);
}

//==============================================================================

void loop() {
  // Read and print the accelerations
  auto accelerations = adxl355.getAccelerations();
  float adxl_angl_x = atan2f(accelerations.x,(sqrt(accelerations.y*accelerations.y)+sqrt(accelerations.x*accelerations.x)))*(RAD_TO_DEG);
  float adxl_angl_y = atan2f(accelerations.y,(sqrt(accelerations.x*accelerations.x)+sqrt(accelerations.z*accelerations.z)))*(RAD_TO_DEG);
  float adxl_angl_z = atan2f(accelerations.z, (sqrt(accelerations.x*accelerations.x)+sqrt(accelerations.y*accelerations.y)))*(RAD_TO_DEG);
  //Serial.print("Accelerations: X: ");
  Serial.print(accelerations.x);
  Serial.print(",");
  Serial.print(accelerations.y);
  Serial.print(",");
  Serial.println(accelerations.z);
  Serial.println();
  //Serial.println(" ");
  //Serial.print("Angles: X: ");
  //Serial.print(adxl_angl_x);
  //Serial.print(" deg, Y: ");
  //Serial.print(adxl_angl_y);
  //Serial.print(" deg, Z: ");
  //Serial.print(adxl_angl_z);
  //Serial.print(" deg");

  delay (1000);
}
  //agora=0;
  //antes=0;

  //Serial.println("");
  //delay(100);


//void loop() {
  //agora=millis();
  //if(agora-antes>=periodo){

  /* Get new sensor events with the readings */
    //sensors_event_t a, g, temp;
    //mpu.getEvent(&a, &g, &temp);
    //ax=a.acceleration.x/9.81;
    //ay=a.acceleration.y/9.81;
    //az=a.acceleration.z/9.81;

  /* Print out the values */
    //Serial.print(ax, 6);
    //Serial.print(",");
    //Serial.print(ay, 6);
    //Serial.print(",");
    //Serial.print(az, 6);
    //Serial.println();

     //antes=agora;
  //}

//}