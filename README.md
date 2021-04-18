# Xiao CV Sequencer
Synthesizer module that generates CV sequences using a Seeeduino Xiao microcontroller

This project uses a Seeeduino Xiao as a CV sequence generator to drive notes on an analog synthesizer
or modular synthesizer. It can operate in two modes: sequencer mode or octave tuner mode. 
When in sequencer mode it generates a repeating sequence of notes. When in octave tuner mode, 
it loops through 1V / 1 octave increments (useful when trying to tune oscillators).

The project can be run from USB power or made into a module that runs off of a regulated 5V power supply.
The schematic shows the wiring needed to run as a module.

This tool has the following features:
 * 10-bit analog voltage output (DAC), conformed to the one-volt-per-octave standard
 * 12-bit analog voltage input (ADC), for debugging
 * Playback tempo control
 * Sequence selection control
 * Octave control
 * LED indicators
