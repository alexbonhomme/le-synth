#include <Arduino.h>
#include <MIDI.h>
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>
#include "engine/Synth.h"

// #define DEBUG

#define MIDI_CHANNEL 8

#define WAVEFORM_SWITCH_1_1 2
#define WAVEFORM_SWITCH_1_2 3
#define WAVEFORM_SWITCH_2_1 4
#define WAVEFORM_SWITCH_2_2 5
#define OCTAVE_SWITCH_1 6
#define OCTAVE_SWITCH_2 8
#define ENVELOPE_SWITCH 9

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

Engine::Synth synth;

Bounce waveform_switch_1_1 = Bounce();
Bounce waveform_switch_1_2 = Bounce();
Bounce waveform_switch_2_1 = Bounce();
Bounce waveform_switch_2_2 = Bounce();
Bounce octave_switch_1 = Bounce();
Bounce octave_switch_2 = Bounce();
Bounce envelope_switch = Bounce();

ResponsiveAnalogRead pot_freq_2(A0, true);
ResponsiveAnalogRead pot_amplitude_2(A1, true);
ResponsiveAnalogRead pot_amplitude_sub(A2, true);
ResponsiveAnalogRead pot_attack(A3, true);
ResponsiveAnalogRead pot_release(A4, true);
ResponsiveAnalogRead cv_2(A5, true);

// Forward declarations
void OnNoteOn(byte channel, byte note, byte velocity);
void OnNoteOff(byte channel, byte note, byte velocity);
float computeOctaveDivider(int octave_switch_1, int octave_switch_2);

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Initialize the buttons and switches
  pinMode(WAVEFORM_SWITCH_1_1, INPUT_PULLUP);
  pinMode(WAVEFORM_SWITCH_1_2, INPUT_PULLUP);
  pinMode(WAVEFORM_SWITCH_2_1, INPUT_PULLUP);
  pinMode(WAVEFORM_SWITCH_2_2, INPUT_PULLUP);
  pinMode(OCTAVE_SWITCH_1, INPUT_PULLUP);
  pinMode(OCTAVE_SWITCH_2, INPUT_PULLUP);
  pinMode(ENVELOPE_SWITCH, INPUT_PULLUP);

  // Initialize Bounce objects
  waveform_switch_1_1.attach(WAVEFORM_SWITCH_1_1);
  waveform_switch_1_1.interval(15);
  waveform_switch_1_2.attach(WAVEFORM_SWITCH_1_2);
  waveform_switch_1_2.interval(15);
  waveform_switch_2_1.attach(WAVEFORM_SWITCH_2_1);
  waveform_switch_2_1.interval(15);
  waveform_switch_2_2.attach(WAVEFORM_SWITCH_2_2);
  waveform_switch_2_2.interval(15);
  octave_switch_1.attach(OCTAVE_SWITCH_1);
  octave_switch_1.interval(15);
  octave_switch_2.attach(OCTAVE_SWITCH_2);
  octave_switch_2.interval(15);
  envelope_switch.attach(ENVELOPE_SWITCH);
  envelope_switch.interval(15);

  // Synth engine init
  synth.begin();

  // USB MIDI
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);

  // Serial MIDI
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.begin(MIDI_CHANNEL);
}

void loop()
{
  // Handle MIDI messages
  usbMIDI.read();
  MIDI.read();

  /*
   * Read the buttons and knobs, scale knobs to 0-1.0
   */
  waveform_switch_1_1.update();
  waveform_switch_1_2.update();
  waveform_switch_2_1.update();
  waveform_switch_2_2.update();
  pot_freq_2.update();
  pot_amplitude_2.update();
  pot_amplitude_sub.update();
  pot_attack.update();
  pot_release.update();
  cv_2.update();

#ifdef DEBUG
  // @todo impl osc 2 frequency modulation
  if (cv_2.hasChanged())
  {
    Serial.println("CV 2:");
    Serial.println(cv_2.getValue());
  }
#endif

  // Set sub oscillator to octave -1, -2 or 0 (disabled)
  if (octave_switch_1.update() || octave_switch_2.update())
  {
    float octave_divider = computeOctaveDivider(octave_switch_1.read(), octave_switch_2.read());

    synth.updateOctaveDivider(octave_divider);
  }

  if (envelope_switch.update())
  {
    synth.updateEnvelopeMode(envelope_switch.read() == LOW);
  }

  // 1v/Oct (0 - 10v)
  // float cv = (float)analogRead(A9) / 102.3;

  // Update the frequency of the second oscillator
  if (pot_freq_2.hasChanged())
  {
    synth.updateOsc2Frequency((float)pot_freq_2.getValue() / 1023.0);
  }

  // Update the amplitude of the second oscillator
  if (pot_amplitude_2.hasChanged())
  {
    synth.updateOsc2Amplitude((float)pot_amplitude_2.getValue() / 1023.0);
  }

  // Update the amplitude of the sub oscillator
  if (pot_amplitude_sub.hasChanged())
  {
    synth.updateSubAmplitude((float)pot_amplitude_sub.getValue() / 1023.0);
  }

  // Button 1 changes the waveform type of the main oscillators
  if (waveform_switch_1_1.update() || waveform_switch_1_2.update())
  {
    int waveform = WAVEFORM_TRIANGLE;

    if (waveform_switch_1_1.read() == LOW)
    {
      waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
    }
    else if (waveform_switch_1_2.read() == LOW)
    {
      waveform = WAVEFORM_BANDLIMIT_SQUARE;
    }

    synth.updateWaveform(waveform);
  }

  if (pot_attack.hasChanged())
  {
    synth.updateAttack((float)pot_attack.getValue() / 1023.0);
  }

  if (pot_release.hasChanged())
  {
    synth.updateRelease((float)pot_release.getValue() / 1023.0);
  }
}

// MIDI event handlers
void OnNoteOn(byte channel, byte note, byte velocity)
{
  synth.noteOn(note, velocity, (float)pot_freq_2.getValue() / 1023.0);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  synth.noteOff();
}

/*
 * Compute the octave divider based on the octave switch inputs.
 *
 * The computation is a little bit weird because of the switch schematic.
 */
 float computeOctaveDivider(int octave_switch_1, int octave_switch_2)
 {
   if (octave_switch_2 == LOW)
   {
     return 0.0;
   }
   else if (octave_switch_1 == LOW)
   {
     return 4.0;
   }

   return 2.0;
 }