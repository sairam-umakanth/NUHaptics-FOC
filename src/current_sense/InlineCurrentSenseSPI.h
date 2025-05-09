#ifndef SPI_INLINE_CS_LIB_H
#define SPI_INLINE_CS_LIB_H

#include "Arduino.h"
#include "SPI.h"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "../common/base_classes/CurrentSense.h"
#include "../common/base_classes/StepperDriver.h"
#include "../common/base_classes/BLDCDriver.h"
#include "../common/lowpass_filter.h"
#include "hardware_api.h"

class InlineCurrentSenseSPI: public CurrentSense{
    public:
        /**
         * InlineCurrentSenseSPI class constructor
         * @param shunt_resistor shunt resistor value
         * @param gain current-sense op-amp gain
         * @param spi SPIClass pointer
         * @param csA chip select pin for phase A
         * @param csB chip select pin for phase B
         * @param csC chip select pin for phase C
         */
        InlineCurrentSenseSPI(float shunt_resistor, float gain, SPIClass *spi, int csA, int csB, int csC);
        
        // CurrentSense interface implementing functions
        int init() override;
        PhaseCurrent_s getPhaseCurrents() override;

    private:
        // gain variables
        float shunt_resistor; // Shunt resistor value
        float amp_gain; // amp gain value
        float volts_to_amps_ratio; // Volts to amps ratio

        float offset_ia = 0; // Phase A current offset
        float offset_ib = 0; // Phase B current offset
        float offset_ic = 0; // Phase C current offset

        float gain_a; // Phase A current gain
        float gain_b; // Phase B current gain
        float gain_c; // Phase C current gain

        SPIClass* spi; // SPIClass pointer 
        SPISettings settings; // SPI settings for the ADC
        int csA; // chip select pin for phase A
        int csB; // chip select pin for phase B
        int csC; // chip select pin for phase C

        /*
        Function for finding zero offsets of ADC and reading ADC off SPI
        */
        void calibrateOffsets();
        float readADC(SPISettings settings, int cs_pin);
};

#endif