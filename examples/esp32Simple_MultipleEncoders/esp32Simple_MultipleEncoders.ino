#include "FastInterruptEncoder.h"

#define ENCODER_READ_DELAY    300

Encoder enc1(25, 26, SINGLE, 250);
Encoder enc2(14, 27, SINGLE, 250);

unsigned long encodertimer = 0;

void setup() {
  Serial.begin(115200);
  
  if (enc1.init(0)) {
    Serial.println("Encoder 1 Initialization OK");
  } else {
    Serial.println("Encoder 1 Initialization Failed");
    while(1);
  }

  if (enc2.init(1)) {
    Serial.println("Encoder 2 Initialization OK");
  } else {
    Serial.println("Encoder 2 Initialization Failed");
    while(1);
  }

}

void loop() {
  enc1.loop();                  
  enc2.loop();                   
                  
  if ((unsigned long)ENCODER_READ_DELAY < (unsigned long)(millis() - encodertimer)) {
    Serial.print(enc1.getTicks());
    Serial.print('\t');
    Serial.println(enc2.getTicks());
    encodertimer = millis();
  }
}