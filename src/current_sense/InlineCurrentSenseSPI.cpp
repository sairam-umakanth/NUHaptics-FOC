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
        offset_ia += readADC(csA);
        offset_ib += readADC(csB);
        offset_ic += readADC(csC);
        _delay(1);
    }
    offset_ia /= calibration_rounds;
    offset_ib /= calibration_rounds;
    offset_ic /= calibration_rounds;
}

PhaseCurrent_s InlineCurrentSenseSPI::getPhaseCurrents(){
    PhaseCurrent_s current;
    current.a = (readADC(csA) - offset_ia) * gain_a;
    current.b = (readADC(csB) - offset_ib) * gain_b;
    current.c = (readADC(csC) - offset_ic) * gain_c;
    return current;
}

float InlineCurrentSenseSPI::readADC(const int cs_pin) const{
    uint16_t raw;
    spi->beginTransaction(settings);
    digitalWriteFast(cs_pin, LOW);
    raw = spi->transfer16(0x0000); // ADS7044 returns 12 MSBs of data in top bits
    digitalWriteFast(cs_pin, HIGH);
    spi->endTransaction();

    raw = raw>>2; // shift to get 12 bits
    int16_t signed_val = (raw >= 0x800) ? (static_cast<int16_t>(raw) - 0x1000) : static_cast<int16_t>(raw);
    // convert to voltage
    float voltage = static_cast<float>(signed_val) * 1.65f / 2047.0f; // 1.65V reference voltage, 12 bit ADC
    return voltage;
}
