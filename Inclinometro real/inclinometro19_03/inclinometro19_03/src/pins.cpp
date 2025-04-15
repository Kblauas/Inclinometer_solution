#include "pins.h"

void pins_init(void){
    Wire.begin(SDA, SCL); 
    
}