#include "FastInlineCurrentSense.h"
#include "communication/SimpleFOCDebug.h"
#include <numeric>

// InlineCurrentSensor constructor
//  - shunt_resistor  - shunt resistor value
//  - gain  - current-sense op-amp gain
//  - phA   - A phase adc pin
//  - phB   - B phase adc pin
//  - phC   - C phase adc pin (optional)
FastInlineCurrentSense::FastInlineCurrentSense(float _shunt_resistor, float _gain, int _pinA, int _pinB, int _pinC){
    pinA = _pinA;
    pinB = _pinB;
    pinC = _pinC;

    shunt_resistor = _shunt_resistor;
    amp_gain  = _gain;
    volts_to_amps_ratio = 1.0f /_shunt_resistor / _gain; // volts to amps
    // gains for each phase
    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;
};


FastInlineCurrentSense::FastInlineCurrentSense(float _mVpA, int _pinA, int _pinB, int _pinC){
    pinA = _pinA;
    pinB = _pinB;
    pinC = _pinC;

    volts_to_amps_ratio = 1000.0f / _mVpA; // mV to amps
    // gains for each phase
    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;
};



// Inline sensor init function
int FastInlineCurrentSense::init(){
    // if no linked driver its fine in this case 
    // at least for init()
    void* drv_params = driver ? driver->params : nullptr;
    // configure ADC variables
    params = _configureADCInline(drv_params,pinA,pinB,pinC);
    // if init failed return fail
    if (params == SIMPLEFOC_CURRENT_SENSE_INIT_FAILED) return 0; 
    // set the center pwm (0 voltage vector)
    if(driver_type==DriverType::BLDC)
        static_cast<BLDCDriver*>(driver)->setPwm(driver->voltage_limit/2, driver->voltage_limit/2, driver->voltage_limit/2);
    // calibrate zero offsets
    calibrateOffsets();
    // set zero voltage to all phases
    if(driver_type==DriverType::BLDC)
        static_cast<BLDCDriver*>(driver)->setPwm(0,0,0);
    // set the initialized flag
    initialized = (params!=SIMPLEFOC_CURRENT_SENSE_INIT_FAILED);
    // return success
    return 1;
}
// Function finding zero offsets of the ADC
void FastInlineCurrentSense::calibrateOffsets(){
    const int calibration_rounds = 1000;
    
    // find adc offset = zero current voltage
    offset_ia = 0;
    offset_ib = 0;
    offset_ic = 0;
    // read the adc voltage 1000 times ( arbitrary number )
    for (int i = 0; i < calibration_rounds; i++) {
        if(_isset(pinA)) offset_ia += _readADCVoltageInline(pinA, params);
        if(_isset(pinB)) offset_ib += _readADCVoltageInline(pinB, params);
        if(_isset(pinC)) offset_ic += _readADCVoltageInline(pinC, params);
        _delay(1);
    }
    // calculate the mean offsets
    if(_isset(pinA)) offset_ia = offset_ia / calibration_rounds;
    if(_isset(pinB)) offset_ib = offset_ib / calibration_rounds;
    if(_isset(pinC)) offset_ic = offset_ic / calibration_rounds;
}

float FastInlineCurrentSense::getFilterCurrent(int pin, float offset, float gain)
{
    const auto read = (_readADCVoltageInline(pin, params) - offset)*gain;
    current_readings.push_back(read);
    if(current_readings.size() > filter_size){
        current_readings.pop_front();
    }
    const auto filt_read = std::accumulate(current_readings.begin(), current_readings.end(), 0) / current_readings.size();
    return filt_read;
}

// read all three phase currents (if possible 2 or 3)
PhaseCurrent_s FastInlineCurrentSense::getPhaseCurrents(){
    PhaseCurrent_s current;
    current.a = (!_isset(pinA)) ? 0 : getFilterCurrent(pinA, offset_ia, gain_a); // amps
    current.b = (!_isset(pinB)) ? 0 : getFilterCurrent(pinB, offset_ib, gain_b); // amps
    current.c = (!_isset(pinC)) ? 0 : getFilterCurrent(pinC, offset_ic, gain_c); // amps
    return current;
}
