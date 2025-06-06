/**
 * Testing example code for the Inline current sensing class
*/
#include <SimpleFOC.h>

// current sensor
// shunt resistor value
// gain value
// pins phase A,B, (C optional)
InlineCurrentSense current_sense = InlineCurrentSense(0.01f, 50.0f, A0, A2);


void setup() {

  // use monitoring with serial 
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  // initialise the current sensing
  if(!current_sense.init()){
    Serial.println("Current sense init failed.");
    return;
  }

  // for SimpleFOCShield v2.01/v2.0.2
  current_sense.gain_b *= -1;
  
  Serial.println("Current sense ready.");
}

void loop() {

    PhaseCurrent_s currents = current_sense.getPhaseCurrents();
    float current_magnitude = current_sense.getDCCurrent();

    Serial.print(currents.a*1000); // milli Amps
    Serial.print("\t");
    Serial.print(currents.b*1000); // milli Amps
    Serial.print("\t");
    Serial.print(currents.c*1000); // milli Amps
    Serial.print("\t");
    Serial.println(current_magnitude*1000); // milli Amps
}
