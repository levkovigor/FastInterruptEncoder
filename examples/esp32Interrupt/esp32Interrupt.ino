#include "FastInterruptEncoder.h"

//Encoder enc(PA0, PA1, SINGLE /* or HALFQUAD or FULLQUAD */, 250 /* Noise and Debounce Filter (default 0) */); // - Example for STM32, check datasheet for possible Timers for Encoder mode. TIM_CHANNEL_1 and TIM_CHANNEL_2 only
Encoder enc(25, 26, SINGLE, 250); // - Example for ESP32

unsigned long encodertimer = 0;

/* create a hardware timer */
hw_timer_t * timer = NULL;

void IRAM_ATTR Update_IT_callback()
{ 
  enc.loop(); 
}

void setup() {
  Serial.begin(115200);
  
  if (enc.init()) {
    Serial.println("Encoder Initialization OK");
  } else {
    Serial.println("Encoder Initialization Failed");
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
    Serial.println(enc.getTicks());
    encodertimer = millis();
  }
}
