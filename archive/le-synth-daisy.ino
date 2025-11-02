#include "DaisyDuino.h"

#include <MIDI.h>
#include <ResponsiveAnalogRead.h>

#define MIDI_CHANNEL 8

#define BT_1 0
#define BT_2 1
#define OCTAVE_SWITCH_1 2
#define OCTAVE_SWITCH_2 3
#define ENVELOPE_SWITCH 4

#define MAIN_MIX_GAIN 0.1
#define SUB_MIX_GAIN 0.2
#define FILTER_ENV_GAIN 0.5

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

DaisyHardware hw;

Oscillator osc1;
Oscillator osc2;
Oscillator osc_sub;

Svf filter_sub;

Adsr env1;
Adsr env2;

Switch button_1;
Switch button_2;
Switch octave_switch_1;
Switch octave_switch_2;
Switch envelope_switch;

ResponsiveAnalogRead pot_freq_2(A0, true);
ResponsiveAnalogRead pot_amplitude_2(A1, true);
ResponsiveAnalogRead pot_amplitude_sub(A2, true);
ResponsiveAnalogRead pot_attack(A3, true);
ResponsiveAnalogRead pot_release(A4, true);

int current_waveform = Oscillator::WAVE_POLYBLEP_SAW;
int current_waveform_sub = Oscillator::WAVE_POLYBLEP_SQUARE;

float octave_divider = 2.0;

float freq = 440.0;
float freq_2 = 440.0;
float freq_sub = 220.0;

float amplitude = 1.0;
float amplitude_2 = 0.0;
float amplitude_sub = 0.0;

int current_note = 0;

int attack_time = 1;
int release_time = 10;

bool note_on = false;
float env_out = 0.0;

void setup()
{
  Serial.begin(115200);

  // Initialize the buttons and switches
  button_1.Init(1000.f, true, BT_1, INPUT_PULLUP);
  button_2.Init(1000.f, true, BT_2, INPUT_PULLUP);
  octave_switch_1.Init(1000.f, true, OCTAVE_SWITCH_1, INPUT_PULLUP);
  octave_switch_2.Init(1000.f, true, OCTAVE_SWITCH_2, INPUT_PULLUP);
  envelope_switch.Init(1000.f, true, ENVELOPE_SWITCH, INPUT_PULLUP);

  // Initialize for Daisyseed at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  DAISY.SetAudioBlockSize(1);

  float sample_rate = DAISY.get_samplerate();

  // Initialize the oscillators
  osc1.Init(sample_rate);
  osc1.SetFreq(freq);
  osc1.SetAmp(amplitude);
  osc1.SetWaveform(current_waveform);

  osc2.Init(sample_rate);
  osc2.SetFreq(freq_2);
  osc2.SetAmp(amplitude_2);
  osc2.SetWaveform(current_waveform);

  osc_sub.Init(sample_rate);
  osc_sub.SetFreq(freq_sub);
  osc_sub.SetAmp(amplitude_sub);
  osc_sub.SetWaveform(current_waveform_sub);

  filter_sub.Init(sample_rate);
  filter_sub.SetFreq(freq_sub * 2);
  filter_sub.SetRes(0.7);

  // Initialize the envelopes
  env1.SetTime(ADSR_SEG_ATTACK, attack_time);
  env1.SetTime(ADSR_SEG_DECAY, 0);
  env1.SetTime(ADSR_SEG_RELEASE, release_time);
  env1.SetSustainLevel(1.0);

  // Configure the secondary envelope with the same ADSR values as the first one
  env2.SetTime(ADSR_SEG_ATTACK, attack_time);
  env2.SetTime(ADSR_SEG_DECAY, 0);
  env2.SetTime(ADSR_SEG_RELEASE, release_time);
  env2.SetSustainLevel(1.0);

  // USB MIDI
  // usbMIDI.setHandleNoteOn(OnNoteOn);
  // usbMIDI.setHandleNoteOff(OnNoteOff);

  // Serial MIDI
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.begin(MIDI_CHANNEL);

  DAISY.begin(AudioCallback);
}

void loop()
{
  // Handle MIDI messages
  // usbMIDI.read();
  MIDI.read();

  // Updates envelope DAC value
  analogWrite(A8, env_out * 255);

  /*
   * Read the buttons and knobs, scale knobs to 0-1.0
   */
  button_1.Debounce();
  button_2.Debounce();
  octave_switch_1.Debounce();
  octave_switch_2.Debounce();
  envelope_switch.Debounce();

  pot_freq_2.update();
  pot_amplitude_2.update();
  pot_amplitude_sub.update();
  pot_attack.update();
  pot_release.update();

  // Set sub oscillator to octave -1, -2 or 0 (disabled)
  if (octave_switch_1.RisingEdge() || octave_switch_2.RisingEdge())
  {
    octave_divider = computeOctaveDivider(octave_switch_1.RisingEdge(), octave_switch_2.RisingEdge());

    if (octave_divider > 0.0)
    {
      freq_sub = computeSubFrequency(freq, octave_divider);

      osc_sub.SetFreq(freq_sub);
      filter_sub.SetFreq(freq_sub * 2);
    }
  }

  if (envelope_switch.FallingEdge())
  {
    env1.SetTime(ADSR_SEG_DECAY, release_time);
    env1.SetTime(ADSR_SEG_RELEASE, 0);
    env1.SetSustainLevel(0.0);

    env2.SetTime(ADSR_SEG_DECAY, release_time);
    env2.SetTime(ADSR_SEG_RELEASE, 0);
    env2.SetSustainLevel(0.0);
  }
  else if (envelope_switch.RisingEdge())
  {
    env1.SetTime(ADSR_SEG_DECAY, 0);
    env1.SetTime(ADSR_SEG_RELEASE, release_time);
    env1.SetSustainLevel(1.0);

    env2.SetTime(ADSR_SEG_DECAY, 0);
    env2.SetTime(ADSR_SEG_RELEASE, release_time);
    env2.SetSustainLevel(1.0);
  }

  // 1v/Oct (0 - 10v)
  // float cv = (float)analogRead(A9) / 102.3;

  // Update the frequency of the second oscillator
  if (pot_freq_2.hasChanged())
  {
    freq_2 = computeFrequencyFromNote(current_note, (float)pot_freq_2.getValue() / 1023.0);

    osc2.SetFreq(freq_2);
  }

  // Update the amplitude of the second oscillator
  if (pot_amplitude_2.hasChanged())
  {
    amplitude_2 = (float)pot_amplitude_2.getValue() / 1023.0;

    osc2.SetAmp(amplitude_2);
  }

  // Update the amplitude of the sub oscillator
  if (pot_amplitude_sub.hasChanged())
  {
    amplitude_sub = ((float)pot_amplitude_sub.getValue() / 1023.0) * 0.5; // attenuation to avoid clipping in filter

    osc_sub.SetAmp(amplitude_sub);
  }

  // Button 1 changes the waveform type of the main oscillators
  if (button_1.FallingEdge())
  {
    switch (current_waveform)
    {
    case Oscillator::WAVE_POLYBLEP_SAW:
      current_waveform = Oscillator::WAVE_POLYBLEP_SQUARE;
      break;
    case Oscillator::WAVE_POLYBLEP_SQUARE:
      current_waveform = Oscillator::WAVE_POLYBLEP_SAW;
      break;
    }

    osc1.SetWaveform(current_waveform);
    osc2.SetWaveform(current_waveform);
  }

  // Button 2 changes the waveform type of the sub oscillator
  if (button_2.FallingEdge())
  {
    switch (current_waveform_sub)
    {
    case Oscillator::WAVE_POLYBLEP_SAW:
      current_waveform_sub = Oscillator::WAVE_POLYBLEP_SQUARE;
      break;
    case Oscillator::WAVE_POLYBLEP_SQUARE:
      current_waveform_sub = Oscillator::WAVE_POLYBLEP_TRI;
      break;
    case Oscillator::WAVE_POLYBLEP_TRI:
      current_waveform_sub = Oscillator::WAVE_POLYBLEP_SAW;
      break;
    }

    osc_sub.SetWaveform(current_waveform_sub);
  }

  if (pot_attack.hasChanged())
  {
    float pot_normalized = (float)pot_attack.getValue() / 1023.0;
    attack_time = pot_normalized * 149 + 1; // 1 to 150ms

    env1.SetTime(ADSR_SEG_ATTACK, attack_time);
    env2.SetTime(ADSR_SEG_ATTACK, attack_time);
  }

  if (pot_release.hasChanged())
  {
    float pot_normalized = (float)pot_release.getValue() / 1023.0;
    release_time = pot_normalized * 598 + 2; // 2 to 600ms

    env1.SetTime(ADSR_SEG_RELEASE, release_time);
    env2.SetTime(ADSR_SEG_RELEASE, release_time);
  }
}

void AudioCallback(float **in, float **out, size_t size)
{
  float osc1_out, osc2_out, osc_sub_out;

  for (size_t i = 0; i < size; i++)
  {
    // Update envelope value
    env_out = env1.Process(note_on);

    osc1_out = osc1.Process() * env_out;
    osc2_out = osc2.Process() * env_out;

    //  Sub oscillator processing
    if (octave_divider > 0.0)
    {
      filter_sub.Process(osc_sub.Process());

      osc_sub_out = filter_sub.Low() * env_out;
    }
    else
    {
      osc_sub_out = 0.0;
    }

    // Mix the oscillators
    OUT_L[i] = osc1_out * MAIN_MIX_GAIN + osc2_out * MAIN_MIX_GAIN + osc_sub_out * SUB_MIX_GAIN;
  }
}

// MIDI event handlers
void OnNoteOn(byte channel, byte note, byte velocity)
{
  Serial.println("OnNoteOn");
  Serial.println(note);
  Serial.println(velocity);

  current_note = note;
  float sustain = (float)velocity / 127.0;

  // update main oscillator
  freq = computeFrequencyFromNote(current_note, 0);

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, (float)pot_freq_2.getValue() / 1023.0);

  osc1.SetFreq(freq);
  osc2.SetFreq(freq_2);

  // update sub oscillator
  if (octave_divider > 0.0)
  {
    freq_sub = computeSubFrequency(freq, octave_divider);

    osc_sub.SetFreq(freq_sub);
    filter_sub.SetFreq(freq_sub * 2);
  }

  env1.SetSustainLevel(sustain);
  env2.SetSustainLevel(sustain * 0.8);

  // Trigger the envelopes
  note_on = true;
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  note_on = false;
}

float computeSubFrequency(float freq, float octave_divider)
{
  return octave_divider > 0.0 ? freq / octave_divider : 0.0;
}

/*
 * Compute the octave divider based on the octave switch inputs.
 *
 * The computation is a little bit weird because of the switch schematic.
 */
float computeOctaveDivider(bool octave_switch_1, bool octave_switch_2)
{
  if (octave_switch_2)
  {
    return 0.0;
  }
  else if (octave_switch_1)
  {
    return 4.0;
  }

  return 2.0;
}

/**
 * Compute the frequency of the waveform based on the note and pot input.
 *
 * The note input is the MIDI note number, and the pot input is from 0 to 1.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float computeFrequencyFromNote(byte note, float pot)
{
  float offset = pot * 16.0 - 8.0; // -8 to 8
  float note_offset = note + offset;

  return 440.0 * powf(2.0, (float)(note_offset - 73) * 0.08333333); // 440.0 = A4
}

/**
 * Compute the frequency of the waveform based on the pot and cv input.
 *
 * The pot input is should be from 0 to 1, and the cv input from 0 to 10.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float computeFrequencyFromCV(float cv, float pot)
{
  float offset = pot * 2.0 - 1.0; // -1 to 1
  float cv_offset = cv + offset;

  if (cv_offset < 0)
  {
    cv_offset = 0.0;
  }
  else if (cv_offset > 10)
  {
    cv_offset = 10.0;
  }

  return 32.7032 * pow(2, cv_offset); // f0 = C0 = 32.7032
}
