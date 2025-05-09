#include "InlineCurrentSenseSPI.h"
#include "../communication/SimpleFOCDebug.h"

InlineCurrentSenseSPI::InlineCurrentSenseSPI(float _shunt_resistor, float _gain, SPIClass* spi, int csA, int csB, int csC)
    : spi(_spi), csA(_csA), csB(_csB), csC(_csC) {
        
    shunt_resistor = _shunt_resistor;
    amp_gain = _gain;
    volts_to_amps_ratio = 1.0f/(_shunt_resistor * _gain);

    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;

    pinA = -1; pinB = -1; pinC = -1; // Disable internal ADC
}

int InlineCurrentSenseSPI::init(){
    const SPISettings settings(1000000, MSBFIRST, SPI_MODE0);
    pinMode(csA, OUTPUT);
    pinMode(csB, OUTPUT);
    pinMode(csC, OUTPUT);
    digitalWrite(csA, HIGH);
    digitalWrite(csB, HIGH);
    digitalWrite(csC, HIGH);

    calibrateOffsets();
    initialized = true;
    return 1;
}

void InlineCurrentSenseSPI::calibrateOffsets(){
    const int calibration_rounds = 1000;
    offset_ia = 0; offset_ib = 0; offset_ic = 0;

    for(int i=0; i<calibration_rounds; i++){
        offset_ia += readADC(spi, csA);
        offset_ib += readADC(spi, csB);
        offset_ic += readADC(spi, csC);
        _delay(1);
    }
}

PhaseCurrent_s InlineCurrentSenseSPI::getPhaseCurrents(){
    PhaseCurrent_s current;
    current.a = (readADC(spi, csA) - offset_ia) * gain_a;
    current.b = (readADC(spi, csB) - offset_ib) * gain_b;
    current.c = (readADC(spi, csC) - offset_ic) * gain_c;
    return current;
}

float InlineCurrentSenseSPI::readADC(SPISettings settings, int cs_pin){
    uint16_t raw;
    spi->beginTransaction(settings);
    digitalWrite(cs_pin, LOW);
    raw = spi->transfer16(0x0000); // ADS7044 returns 12 MSBs of data in top bits
    digitalWrite(cs_pin, HIGH);
    spi->endTransaction();

    raw = raw>>4;
    float voltage = -3.3f + (6.6f * raw)/4095.0f;
    return voltage;
}
