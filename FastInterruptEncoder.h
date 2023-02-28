#ifndef __FASTINTERRUPTENCODER_H__
#define __FASTINTERRUPTENCODER_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "Arduino.h"

typedef enum {
  SINGLE = 0,
  HALFQUAD = 1,
  FULLQUAD = 2,
} encoder_mode_t;

class Encoder
{
  public:
    Encoder(int pinA, int pinB, encoder_mode_t mode = SINGLE, uint8_t filter = 0);
	bool init();
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
};

#endif // __FASTINTERRUPTENCODER_H__
