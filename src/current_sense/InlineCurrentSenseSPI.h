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
         */
        InlineCurrentSenseSPI(float shunt_resistor, float gain, SPIClass *spi, int csA, int csB);
        
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

        float gain_a = 0; // Phase A current gain
        float gain_b = 0; // Phase B current gain

        SPIClass* spi; // SPIClass pointer 
        SPISettings settings = SPISettings(1000000, MSBFIRST, SPI_MODE0); // SPI settings for the ADC
        const int csA; // chip select pin for phase A
        const int csB; // chip select pin for phase B

        /*
        Function for finding zero offsets of ADC and reading ADC off SPI
        */
        void calibrateOffsets();
        float readADC(const int cs_pin) const;
};

#endif