#ifndef F_INLINE_CS_LIB_H
#define F_INLINE_CS_LIB_H

#include "Arduino.h"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "../common/base_classes/CurrentSense.h"
#include "../common/base_classes/StepperDriver.h"
#include "../common/base_classes/BLDCDriver.h"
#include "../common/lowpass_filter.h"
#include "hardware_api.h"
#include <deque>



class FastInlineCurrentSense: public CurrentSense{
  public:
    /**
      FastInlineCurrentSense class constructor
      @param shunt_resistor shunt resistor value
      @param gain current-sense op-amp gain
      @param phA A phase adc pin
      @param phB B phase adc pin
      @param phC C phase adc pin (optional)
    */
    FastInlineCurrentSense(float shunt_resistor, float gain, int pinA, int pinB, int pinC = NOT_SET);
    /**
      FastInlineCurrentSense class constructor
      @param mVpA mV per Amp ratio
      @param phA A phase adc pin
      @param phB B phase adc pin
      @param phC C phase adc pin (optional)
    */
    FastInlineCurrentSense(float mVpA, int pinA, int pinB, int pinC = NOT_SET);

    // CurrentSense interface implementing functions 
    int init() override;
    PhaseCurrent_s getPhaseCurrents() override;

  private:
  
    // gain variables
    float shunt_resistor; //!< Shunt resistor value
    float amp_gain; //!< amp gain value
    float volts_to_amps_ratio; //!< Volts to amps ratio


    std::deque<float> current_readings{};
    constexpr size_t filter_size = 4;
    
    /**
     *  Function finding zero offsets of the ADC
     */
    void calibrateOffsets();

    float getFilterCurrent();

};

#endif