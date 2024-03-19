#ifndef __FASTINTERRUPTENCODER_H__
#define __FASTINTERRUPTENCODER_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "Arduino.h"
#if defined(ESP32)
	#include <soc/pcnt_struct.h>
	#include <driver/gpio.h>
	#include <driver/pcnt.h>
#endif

typedef enum {
  SINGLE = 0,
  HALFQUAD = 1,
  FULLQUAD = 2,
} encoder_mode_t;

class Encoder
{
  public:
	Encoder();
    	Encoder(int pinA, int pinB, encoder_mode_t mode = SINGLE, uint8_t filter = 0);
	void setEncoder(int pinA, int pinB, encoder_mode_t mode = SINGLE, uint8_t filter = 0);
	bool init(uint8_t unitNum = 0);
	void loop();
	int32_t getTicks();
	void resetTicks();
	void setInvert(bool invert = true);
	
  private:
	int _pinA;
	int _pinB;
	encoder_mode_t _mode = SINGLE;
	int32_t _ticks = 0;
	int32_t _prevTicks = 0;
	uint8_t _filter = 0;
	bool _invert = false;
	#if defined(ESP32)
		pcnt_unit_t unit;
	#endif
};

#endif // __FASTINTERRUPTENCODER_H__
