#include "FastInterruptEncoder.h"

Encoder enc1(25, 26, SINGLE, 250);
Encoder enc2(14, 27, SINGLE, 250);

unsigned long encodertimer = 0;

/* create a hardware timer */
hw_timer_t * timer = NULL;

void IRAM_ATTR Update_IT_callback()
{ 
  enc1.loop(); 
  enc2.loop(); 
}

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
  
  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 80, true);
  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &Update_IT_callback, true);
  /* Set alarm to call onTimer function every 100 ms -> 100 Hz */
  timerAlarmWrite(timer, 10000, true);
  /* Start an alarm */
  timerAlarmEnable(timer);
}

void loop() {                  
  if ((millis() > (encodertimer + 1000)) || (millis() < encodertimer)) {
    Serial.print(enc1.getTicks());
    Serial.print('\t');
    Serial.println(enc2.getTicks());
    encodertimer = millis();
  }
}