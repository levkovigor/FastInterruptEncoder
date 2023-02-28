#include "Arduino.h"
#include "FastInterruptEncoder.h"

Encoder::Encoder(int pinA, int pinB, encoder_mode_t mode, uint8_t filter){
	_pinA = pinA;
	_pinB = pinB;
	_mode = mode;	
	_filter = filter;
}

#if defined(ESP32)
	
	#include <soc/pcnt_struct.h>
	#include <driver/gpio.h>
	#include <driver/pcnt.h>
	
	static  pcnt_isr_handle_t user_isr_handle;
	pcnt_config_t r_enc_config;
	pcnt_unit_t unit;
	
	static void IRAM_ATTR pcnt_example_intr_handler(void *arg) {
	  uint32_t intr_status = PCNT.int_st.val;
	  int i = 0;
	  if (intr_status & (BIT(i))) {
		  if(PCNT.status_unit[i].h_lim_lat){ }
		  if(PCNT.status_unit[i].l_lim_lat){ }
		  PCNT.int_clr.val = BIT(i);
	   }
	}
	
	bool Encoder::init(){
	  pinMode(_pinA, INPUT_PULLUP);
	  pinMode(_pinB, INPUT_PULLUP);

	  unit = (pcnt_unit_t) 0;
	  
	  r_enc_config.pulse_gpio_num = (gpio_num_t) _pinA;
	  r_enc_config.ctrl_gpio_num = (gpio_num_t) _pinB;   

	  r_enc_config.unit = unit;
	  r_enc_config.channel = PCNT_CHANNEL_0;
	  
	  	  
	  if (_mode == SINGLE) {
		  r_enc_config.pos_mode = PCNT_COUNT_DIS;
	  } else {
		  r_enc_config.pos_mode = PCNT_COUNT_DEC;
	  }
	  r_enc_config.neg_mode = PCNT_COUNT_INC;

	  r_enc_config.lctrl_mode = PCNT_MODE_REVERSE; 
	  r_enc_config.hctrl_mode = PCNT_MODE_KEEP;    

	  r_enc_config.counter_h_lim = 32766;
	  r_enc_config.counter_l_lim = -32766;

	  pcnt_unit_config(&r_enc_config);

	  r_enc_config.pulse_gpio_num = (gpio_num_t) _pinB;
	  r_enc_config.ctrl_gpio_num = (gpio_num_t) _pinA;  

	  r_enc_config.unit = unit;
	  r_enc_config.channel = PCNT_CHANNEL_1;
  
	  if (_mode == FULLQUAD) {
		  r_enc_config.pos_mode = PCNT_COUNT_DEC;
		  r_enc_config.neg_mode = PCNT_COUNT_INC;
		  r_enc_config.lctrl_mode = PCNT_MODE_KEEP; 
		  r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; 
	  } else {
		  r_enc_config.pos_mode = PCNT_COUNT_DIS;
		  r_enc_config.neg_mode = PCNT_COUNT_DIS;	
		  r_enc_config.lctrl_mode = PCNT_MODE_DISABLE;  
		  r_enc_config.hctrl_mode = PCNT_MODE_DISABLE;
	  }

	  r_enc_config.counter_h_lim = 32766;
	  r_enc_config.counter_l_lim = -32766;

	  pcnt_unit_config(&r_enc_config);

	  // Filter out bounces and noise
	  pcnt_set_filter_value(unit, _filter);
	  pcnt_filter_enable(unit); 

	  /* Enable events on maximum and minimum limit values */
	  pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	  pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	  pcnt_counter_pause(unit); // Initial PCNT init
	  pcnt_counter_clear(unit);
	  
	  /* Register ISR handler and enable interrupts for PCNT unit */
	  esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler,(void *) NULL, (int)0, NULL);
	  if (er != ESP_OK){
		  return 0;
	  }
	  pcnt_intr_enable(unit);
	  pcnt_counter_resume(unit);
	  return 1;
	}
	
	void Encoder::loop(){
	  int16_t c;
	  pcnt_get_counter_value(unit, &c);
       
	  if ((_prevTicks+c) != _prevTicks){
		  pcnt_counter_clear(unit);
		 _prevTicks = _prevTicks+c;
		  if (_invert){
		  	_ticks -= _prevTicks;
		  } else {
			_ticks += _prevTicks;
		  }
		  _prevTicks = 0; 
	  }
	}
	
#elif defined(_STM32_DEF_)
	
	bool Encoder::init(){
	   pinMode(_pinA, INPUT_PULLUP);
	   pinMode(_pinB, INPUT_PULLUP);
	   
	   pin_function(digitalPinToPinName(_pinA), pinmap_function(digitalPinToPinName(_pinA), PinMap_TIM));
	   pin_function(digitalPinToPinName(_pinB), pinmap_function(digitalPinToPinName(_pinB), PinMap_TIM));
	   
	   TIM_HandleTypeDef       Encoder_Handle;
	   TIM_Encoder_InitTypeDef sEncoderConfig;

	   Encoder_Handle.Init.Period             = 65535;
	   if (_mode == SINGLE) {
		Encoder_Handle.Init.Prescaler          = 1;
	   } else {
		Encoder_Handle.Init.Prescaler          = 0; 
	   }
	   Encoder_Handle.Init.ClockDivision      = 0;
	   Encoder_Handle.Init.CounterMode        = TIM_COUNTERMODE_UP;
	   Encoder_Handle.Init.RepetitionCounter  = 0;
	   Encoder_Handle.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;

	   if (_mode == FULLQUAD) {
		sEncoderConfig.EncoderMode             = TIM_ENCODERMODE_TI12;
	   } else {
		sEncoderConfig.EncoderMode             = TIM_ENCODERMODE_TI1;
	   } 

	   sEncoderConfig.IC1Polarity             = TIM_ICPOLARITY_RISING;
	   sEncoderConfig.IC1Selection            = TIM_ICSELECTION_DIRECTTI;
	   sEncoderConfig.IC1Prescaler            = TIM_ICPSC_DIV1;
	   sEncoderConfig.IC1Filter               = _filter;
	
	   sEncoderConfig.IC2Polarity             = TIM_ICPOLARITY_RISING;
	   sEncoderConfig.IC2Selection            = TIM_ICSELECTION_DIRECTTI;
	   sEncoderConfig.IC2Prescaler            = TIM_ICPSC_DIV1;
	   sEncoderConfig.IC2Filter               = _filter;
		
	   Encoder_Handle.Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(_pinA), PinMap_TIM);
	   enableTimerClock(&Encoder_Handle);
	   if(HAL_TIM_Encoder_Init(&Encoder_Handle, &sEncoderConfig) != HAL_OK) return 0;
	   HAL_TIM_Encoder_Start(&Encoder_Handle, TIM_CHANNEL_ALL);
	   return 1;
	}

	void Encoder::loop(){
	  int c = LL_TIM_GetCounter((TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(_pinA), PinMap_TIM));
	  int change = c - _prevTicks;
          _prevTicks = c; 
          if (change > 40000) { change = 65535 - change;}
          else if (change < -40000) {change = -65535 - change;}
          if (_invert){
	  	_ticks -= change;
	  } else {
		_ticks += change;
	  }
	}

#endif

	
int32_t Encoder::getTicks(){
	return _ticks;
}


void Encoder::resetTicks(){
	_ticks = 0;
}

void Encoder::setInvert(bool invert){
	_invert = invert;
}
