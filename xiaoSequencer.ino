/*MIT License

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
       
 * Mighty Mite CV Sequence Generator
 * Analog Sketchbook 
 * 
 * This sketch allows you to use a Seeeduino Xiao as a CV sequence generator to drive notes on an analog synthesizer
 * or modular synthesizer. It can operate in two modes: sequencer mode or octave tuner mode. When in sequencer mode
 * it generates a repeating sequence of notes. When in octave tuner mode, it loops through 1V / 1 octave increments 
 * (useful when trying to tune oscillators).
 * 
 * This tool has the following features:
 * --10-bit analog voltage output (DAC), conformed to the one-volt-per-octave standard
 * --12-bit analog voltage input (ADC), for debugging
 * --Playback tempo control
 * --Sequence selection control
 * --Octave control
 * --LED indicators
 * 
 * Rotary encoder portion of this code is modified from code found on wwww.lastminuteengineers.com
 * 
 * DISCLAIMER: I'm not an electronics engineer...I'm just winging it. Use at your own risk! In other words,
 * before you hook this up to your fancy modular system or analog system, test it out on a multimeter, test equipment,
 * junky module or whatever you're not worried about in case you get the magic smoke. I've tested it on a couple of 
 * my DIY oscillator modules and nothing has ever blown up. But do your due diligence and test it carefully before 
 * you roll it out as part of a larger system. 
 * 
 * Overall sketch notes: 
 * 
 * --CV notes in the NOTES array are indicated by semitones starting at 0, rather than the more familiar MIDI values...
 * although I guess technically speaking they are MIDI values, just at the very bottom of the scale, with 0 representing
 * C0 and 1 representing C#0 and so on. If you want to program in your own sequences, you'll need to drop them down 
 * to the C0 octave in order to work with this script (since these values get multiplied by semitone voltages).
 * 
 * --The analog input CV signal is used mainly for debugging. The idea is that you could feed the output of the Xiao
 * back into the input and verify the exact voltages you're getting. 

 * --If you're playing back sequences at the highest octave, notes that go more than 6 semitones over one octave
 * might get capped at 3.3V (so that we don't go over the 3.3V analog output capabilities of the Xiao).
 * Probably a better way to do this would be to cap it so that the relationship of notes is preserved at the top
 * of the octave range, but I'm being a bit lazy :P
 * 
 * Overall hardware notes:
 * 
 * --Unit can be built as a standalone that can run off of USB power, or as a part of a modular unit powered by
 * the 5V regulated eurorack supply. 
 * 
 * --If you opt to not use the analog input for debugging, you can eliminate the audio jack connected to pin 1 and comment
 * out any analogRead stuff in the sketch.
 * 
 */

// Defining pinouts for Xiao
#define DAC_PIN A0                                  // voltage out pin
#define ADC_PIN  A1                                 // voltage in
#define CLK 3                                       // rotary encoder pin 1
#define DT 2                                        // rotary encoder pin 2
#define SW 5                                        // rotary encoder push button
#define POT A6                                      // tempo pot
#define MODE_SW 4                                   // mode switch to toggle between sequence and octave tuner
#define LEDA 7                                      // indicator LED
#define LEDB 8                                      // indicator LED
#define LEDC 9                                      // indicator LED
#define LEDD 10                                     // indicator LED

// Constant variables (won't change during execution)
const float V_MIN =    0.0000000;                   // minimum voltage
const float V_MAX =    3.3333333;                   // 3.3V max for xiao output
const float V_STEP =   0.0833333;                   // voltage of one semitone
const int BPM_MIN = 6;                              // slowest tempo possible...6 BPM = 10 secs but feel free to change
const int BPM_MAX = 480;                            // fastest tempo possible...feel free to change

// Constants used for storing note sequences
const int NUM_SEQUENCES = 23;                       // total number of sequences (must match number of sequences in NOTES array)
const int NUM_NOTES = 8;                            // total number of notes in each sequence (must all be the same and match length of each sequence in NOTES)

// Sequences of notes to play back
// You can add new sequences or modify existing ones to suit your tastes
// just make sure you update the NUM_SEQUENCES and NUM_NOTES variables above to match what you've done
const int NOTES[NUM_SEQUENCES][NUM_NOTES] = {
                        {0,1,2,3,4,5,6,7},                        // constant
                        {0,4,3,6,5,2,3,4},                        // melody
                        {0,12,0,12,0,12,0,12},                    // up on two
                        {0,0,0,0,0,0,0,0},                        // flat quarters (constant)
                        {0,0,0,12,0,0,0,12},                      // up on four
                        {0,0,12,6,0,0,0,12},                      // dum da dum
                        {12,12,0,12,12,0,12,0},                   // two up one down
                        {0,12,0,12,0,12,0,12},                    // up down
                        {0,0,12,0,0,12,0,12},                     // up on three
                        {0,1,2,3,4,5,6,7},                        // chromatic up
                        {7,6,5,4,3,2,1,0},                        // chromatic down
                        {0,2,3,5,7,8,10,12},                      // major up
                        {12,10,8,7,5,3,2,0},                      // major down
                        {0,2,4,7,9,12,14,16},                     // natural minor up
                        {16,14,12,9,7,4,2,0},                     // natural minor down
                        {0,3,5,6,7,10,12,15},                     // major pentatonic up
                        {15,12,10,7,6,5,3,0},                     // major pentatonic down
                        {0,3,5,6,7,10,12,15},                     // blues up
                        {15,12,10,7,6,5,3,0},                     // blues down
                        {0,4,3,9,12,3,2,10},                      // random one
                        {0,2,5,9,1,7,14,5},                       // random two
                        {0,10,8,1,12,3,6,9},                      // random three
                        {0,8,12,2,3,7,8,2}};                      // random four
                        
// Variables that will change during execution
float vOut =    0.0000000;                                    // to hold output voltage
int beat = 60000/125;                                         // one beat (of eight) based on msecs per minute / bpm
int sequence = 0;                                             // current sequence in NOTES array that is playing
int note = 0;                                                 // current note in current sequence playing                        
float voltageOffset = 0.0000000;                              // holds the octave we want the sequence to play at

// Timing variables
int currentStateCLK;                                          // for rotary encoder
int lastStateCLK;                                             // for rotary encoder
unsigned long currentMillis = 0;                              // stores the value of millis() in each iteration of loop()...all timing events are compared to this value
unsigned long lastButtonPress = 0;                            // for debouncing rotary encoder button                
unsigned long previousNoteMillis = 0;                         // for sequence playback timing                                                  
unsigned long lastAnalogRead = 0;                             // for reading analog inputs timing
unsigned long lastTunerMillis = 0;                            // for octave tuning timing

void setup() {
  analogWriteResolution(10);  // setting output resolution to max, 10-bits
  analogReadResolution(12);   // setting input resolution to max, 12-bits

  // LEDs
  pinMode(LEDA,OUTPUT);
  pinMode(LEDB,OUTPUT);
  pinMode(LEDC,OUTPUT);
  pinMode(LEDD,OUTPUT);
  
  // Rotary encoder
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(MODE_SW, INPUT_PULLUP);

  // Read the initial state of encoder CLK
  lastStateCLK = digitalRead(CLK);

  // Open up Serial Monitor Port for debugging
  SerialUSB.begin(9600);

  // initialize the LEDs to all off using our custom setLEDs function
  setLEDs(LOW,LOW,LOW,LOW);
}

int mapVoltageToAnalogOut(float value, float inMin, float inMax, float outMin, float outMax){
  // Even though the Xiao's DAC outputs voltages 0-3.3V, programmatically you must specify the output
  // voltage as a range from 0-1023. So we need to map our desired voltage to 0-1023. The normal map()
  // function only works on ints, so this function is basically just a float version of 
  // Arduino's built-in map function.
  return round((value-inMin)*(outMax-outMin)/(inMax-inMin)+outMin); 
}

void readEncoder(){
    currentStateCLK = digitalRead(CLK);

    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

      // Determine which way the encoder is rotating base on whether DT and CLK are the same
      // Then jump to the next or previous sequence in NOTES accordingly.
      // If we reach the end/beginning of the list, roll over to the beginning/end as needed
      if (digitalRead(DT) != currentStateCLK) { // if DT is not equal to CLK then we're turning clockwise
          if (sequence < NUM_SEQUENCES-1){
          sequence ++;
          }
          else {
            sequence = 0;     
          }
      } else {                                  // turning counterclockwise
          if (sequence > 0){ 
          sequence --;
        }
        else {                                    
          sequence = NUM_SEQUENCES-1;
        }
        
      }
      SerialUSB.print("Sequence: ");SerialUSB.println(sequence); // for debugging
    }

    // Record the last CLK state
    lastStateCLK = currentStateCLK;

    // Read the encoder's button state
    int btnState = digitalRead(SW);

    // LOW = button pressed
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {                    //debouncing by 50 msecs
        
        // If button is pressed, we'll bump up the value of the voltage to be sequence value + 1V (one octave)
        // Adding some code to make sure we keep our voltage ranges in the acceptable output range
        // so it cycles back to 0V if we hit a total offset of 3V max.
        
        if (voltageOffset <= 2.0000000){
            voltageOffset = voltageOffset + 1.000000;
        } else {
          voltageOffset = 0.0000000;
        }
      }

      // Record last button press event
      lastButtonPress = millis();
      }

      // Put in a slight delay to help debounce the reading further
      delay(1);  
}

void playNote(){
  // This function steps through the notes of the current sequence and writes them to the DAC
  // at intervals determined by the duration of the beat variable. It also flashes the first
  // LED on the first beat of the sequence as a visual cue
  if (note < NUM_NOTES){
        // The output voltage is determined by multiplying the (0-12) note value by 1/12th of a volt (volt per octave standard).
        // That value is then added to any voltageOffset (which are added by pressing the button on the encoder) in case the
        // user wants the sequence to play at a higher octave.
        vOut = (NOTES[sequence][note] * V_STEP) + voltageOffset;

        // Because the voltage offset + the note voltage could potentially cause values greater than maximum voltage
        // We'll add a ceiling to voltages here
        if (vOut > V_MAX){
            vOut = V_MAX;
        }

        // If we're on the first beat, we'll turn on the first LED, otherwise they're all off
        if (note == 0){
            if (voltageOffset == 0.00){
                setLEDs(HIGH,LOW,LOW,LOW);   
            }else if (voltageOffset == 1.00){
                setLEDs(LOW,HIGH,LOW,LOW);     
            }else if (voltageOffset == 2.00){
                setLEDs(LOW,LOW,HIGH,LOW);     
            }else if (voltageOffset == 3.00){
                setLEDs(LOW,LOW,LOW,HIGH);     
            }
        } else {
            setLEDs(LOW,LOW,LOW,LOW);
        }
        
        // mapping voltage to 0-1023 range for the DAC
        int dacVoltage = mapVoltageToAnalogOut(vOut, V_MIN, V_MAX, 0.0000000, 1023.0000000);
        analogWrite(DAC_PIN, dacVoltage);

        // Checking our timing against the master timing to see if a beat has passed yet
        if (currentMillis - previousNoteMillis >= beat) {  // one beat has elapsed so we'll bump up to the next note
            note += 1;
            previousNoteMillis += beat;    
        } 
  } else {
    note = 0;
  }
}

void playOctaves(){
      // this function plays the full voltage range in 1 volt increments 0V - 3V in a loop
      // The idea is that this can be used for oscillator tuning with the tempo pot turned 
      // way down to give you a chance to make tuning adjustments to your oscillator
      if (vOut <= V_MAX){
          // Light LEDs according to which voltage is playing
          if ((vOut < 1.0000000)){
              setLEDs(HIGH,LOW,LOW,LOW);  
          } else if ((vOut >= 1.0000000) && (vOut <2.0000000)){
              setLEDs(LOW,HIGH,LOW,LOW);
          } else if ((vOut >= 2.0000000) && (vOut <3.0000000)){
              setLEDs(LOW,LOW,HIGH,LOW);
          } else {
              setLEDs(LOW,LOW,LOW,HIGH);
          }  

          // mapping voltage to 0-1023 range for the DAC
          int dacVoltage = mapVoltageToAnalogOut(vOut, V_MIN, V_MAX, 0.0000000, 1023.0000000);
          analogWrite(DAC_PIN, dacVoltage);
          

          // Check our timing against the master timing to see if a beat has passed yet
          if (currentMillis - lastTunerMillis >= beat){
            vOut += 1.0000000;     
            lastTunerMillis += beat;
          }
      } else {
        vOut = 0.0000000;  
      }
}
void readAnalogInput(){
  // This function is to test our output voltages by reading them back in again
  // We read them at each iteration in loop(), but we only print them to the Serial Monitor on the first beat
  // (just to make it easier to read) 
  
  float inputVoltage = analogRead(ADC_PIN) * 3.3 / 4096.0;   // reading the analog input and converting from 12-bit analog to a 1-3.3V voltage range
  
  if (currentMillis - lastAnalogRead >= beat) {
        lastAnalogRead += beat;

        SerialUSB.print(int(inputVoltage/V_STEP));SerialUSB.print(" : ");
        SerialUSB.println(inputVoltage);  // for debugging    

  }  
}

void setLEDs(int A, int B, int C, int D){
  // This function allows us to write all four LED values at once...that's all.
        digitalWrite(LEDA,A);
        digitalWrite(LEDB,B);
        digitalWrite(LEDC,C);
        digitalWrite(LEDD,D); 
}

void loop() {

  // Record current time in msecs for master timing
  currentMillis = millis();

  // Read the tempo pot to determine our speed
  // Note that, because we've set the analogRead resolution to 12-bit in setup, the pot value will range from 0-4096
  // instead of the 0-1023 range we'd normally expect.
  int tempo = analogRead(POT);                                      
  int bpmRemap = map(tempo, 0, 4096, BPM_MIN, BPM_MAX);
  beat = 60000/bpmRemap;
  
  // check switch to see which mode we're in (sequencer or tuner) 
  int mode = digitalRead(MODE_SW);                
  
  // if we're in sequencer mode, we'll run subroutines to play through note sequences
  // otherwise we're in tuner mode and we'll just do 1V(octave) steps through the whole voltage range
  if (mode == 1){                       // sequencer mode  
    readEncoder();
    playNote();
  }
  else {                                // octave tuner mode 
    playOctaves();
  }
  // After writing out the value we can test it if needed
  readAnalogInput();                    // read analog inputs
}
