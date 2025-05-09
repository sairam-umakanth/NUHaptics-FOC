#include "InlineCurrentSenseSPI.h"
#include "../communication/SimpleFOCDebug.h"

InlineCurrentSenseSPI::InlineCurrentSenseSPI(float _shunt_resistor, float _gain, SPIClass* _spi, int _csA, int _csB, int _csC)
    : spi(_spi), csA(_csA), csB(_csB), csC(_csC) {
        
    shunt_resistor = _shunt_resistor;
    amp_gain = _gain;
    volts_to_amps_ratio = 1.0f/(_shunt_resistor * _gain);

    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;

}

int InlineCurrentSenseSPI::init(){
    pinMode(csA, OUTPUT);
    pinMode(csB, OUTPUT);
    pinMode(csC, OUTPUT);
    digitalWriteFast(csA, HIGH);
    digitalWriteFast(csB, HIGH);
    digitalWriteFast(csC, HIGH);

    // set the center pwm (0 voltage vector)
    if(driver_type==DriverType::BLDC)
        static_cast<BLDCDriver*>(driver)->setPwm(driver->voltage_limit/2, driver->voltage_limit/2, driver->voltage_limit/2);
    calibrateOffsets();
    // set zero voltage to all phases
    if(driver_type==DriverType::BLDC)
        static_cast<BLDCDriver*>(driver)->setPwm(0,0,0);
    initialized = true;
    return 1;
}

void InlineCurrentSenseSPI::calibrateOffsets(){
    const int calibration_rounds = 1000;
    offset_ia = 0; offset_ib = 0; offset_ic = 0;

    for(int i=0; i<calibration_rounds; i++){
        offset_ia += readADC(settings, csA);
        offset_ib += readADC(settings, csB);
        offset_ic += readADC(settings, csC);
        _delay(1);
    }
}

PhaseCurrent_s InlineCurrentSenseSPI::getPhaseCurrents(){
    PhaseCurrent_s current;
    current.a = (readADC(settings, csA) - offset_ia) * gain_a;
    current.b = (readADC(settings, csB) - offset_ib) * gain_b;
    current.c = (readADC(settings, csC) - offset_ic) * gain_c;
    return current;
}

float InlineCurrentSenseSPI::readADC(const int cs_pin) const{
    uint16_t raw;
    spi->beginTransaction(settings);
    digitalWriteFast(cs_pin, LOW);
    raw = spi->transfer16(0x0000); // ADS7044 returns 12 MSBs of data in top bits
    digitalWriteFast(cs_pin, HIGH);
    spi->endTransaction();

    raw = raw>>4;
    float voltage = -3.3f + (6.6f * raw)/4095.0f;
    return voltage;
}
