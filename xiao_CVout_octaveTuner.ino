/*
 * MIT License

Copyright (c) 2021 Tim Dobbert (analog.sketchbook.101@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * Script for tuning analog oscillators. This script cycles up through 1V per octave range
 * one octave at a time. To tune oscillators use this procedure:
 * 
 * 1. Set V_BOUNDS to 1.0000000. This will cause the script to bounce between 0V and 1V every ten seconds.
 * 
 * 2. On your oscillator, use course tune knob to bring 0V tone to a C2 using a tuner 
 * 
 * 3. Then use fine tuning trim pot on oscillator to bring upper tone (1V) into tune on C3. As you adjust the scale using
 * the trim pot, both notes will shift. Seems best to try and adjust the two notes so that they're the same note, one
 * octave apart (even if it ends up not being C). Then once you have two notes playing an octave apart using the trim pot,
 * use the fine tune to bring it down to C again. 
 * 
 * 3. Repeat previous two steps as necessary until both tones are in tune on C.
 * 
 * 4. Change the V_BOUNDS variable to max whole-volt range (on the Xiao this would be 3.0000000) and check that 
 * the tuning holds across all octaves. If not, continue using trim pot and coarse/fine tune knobs.
 */
 
#define DAC_PIN A0                            // voltage out pin
#define ADC_PIN  A2                           // voltage in
#define LED 7

float V_STEP =   1.0000000;                   // stepping by one octave (one volt)
float V_BOUNDS = 3.0000000;                   // upper limits of loop
float V_OUT =    0.0000000;                   // to hold output voltage
float V_MIN =    0.0000000;                   // minimum voltage
float V_MAX =    3.3333333;                   // 3.3V max for xiao output
int RATE = 10000;

void setup() {
  analogWriteResolution(10);  // setting output resolution to max, 10-bits
  //analogReadResolution(12);   // setting input resolution to max, 12-bits
  //pinMode(LED,OUTPUT);
  SerialUSB.begin(9600);
}

int mapVoltages(float value, float inMin, float inMax, float outMin, float outMax){
  // convert voltage-per-octave semitone voltages to analog out values (0-1023)
  return round((value-inMin)*(outMax-outMin)/(inMax-inMin)+outMin); 
}

void loop() {
  V_OUT = 0.0000000;
  while (V_OUT <= V_BOUNDS){
      int dacVoltage = mapVoltages(V_OUT, V_MIN, V_MAX, 0.0000000, 1023.0000000);  // mapping analog voltage range to 0-1023 for DAC
      //digitalWrite(LED,HIGH);
      analogWrite(DAC_PIN, dacVoltage);
      SerialUSB.print("OUT:  "); SerialUSB.print(V_OUT); SerialUSB.print(" / ");SerialUSB.println(dacVoltage);
      //float involtage = analogRead(ADC_PIN) * 3.3 / 4096.0;
      //SerialUSB.print("  IN:   "); SerialUSB.println(involtage);
      delay(RATE);
      V_OUT += V_STEP;  
  }
}
