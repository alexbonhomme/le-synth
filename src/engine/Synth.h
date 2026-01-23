#pragma once

#include <Arduino.h>
#include <Audio.h>

namespace Engine {

namespace defaults {
static constexpr float main_mix_gain = 0.1f;
static constexpr float sub_mix_gain = 0.45f;
static constexpr float filter_env_gain = 0.5f;
} // namespace defaults

class Synth {
public:
  Synth();

  void begin();
  void noteOn(byte note, byte velocity, float osc2_pot);
  void noteOff();
  void updateOctaveDivider(float octave_divider);
  void updateEnvelopeMode(bool percussive_mode);
  void updateOsc2Frequency(float frequency);
  void updateOsc2Amplitude(float amplitude);
  void updateSubAmplitude(float amplitude);
  void updateWaveform(int waveform);
  void updateAttack(float attack);
  void updateRelease(float release);

private:
  // GUItool: begin automatically generated code
  AudioSynthWaveform waveform1;     // xy=687,580
  AudioSynthWaveform waveform2;     // xy=688,627
  AudioSynthWaveform waveform_sub;  // xy=686,673
  AudioFilterStateVariable filter1; // xy=692,703
  AudioMixer4 mixer1;               // xy=897,623
  AudioEffectEnvelope envelope1;    // xy=1087,622
  AudioEffectEnvelope envelope2;    // xy=1087,680
  AudioSynthWaveformDc dc_signal;   // xy=687,680
  AudioOutputI2S i2s1;              // xy=1271,629
  AudioConnection patchCord1;
  AudioConnection patchCord2;
  AudioConnection patchCord3;
  AudioConnection patchCord4;
  AudioConnection patchCord5;
  AudioConnection patchCord6;
  AudioConnection patchCord7;
  AudioConnection patchCord8;
  // GUItool: end automatically generated code

  int current_waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
  int current_waveform_sub = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;

  float octave_divider = 0.0f;

  float freq = 440.0f;
  float freq_2 = 440.0f;
  float freq_sub = 220.0f;

  float amplitude = 1.0f;
  float amplitude_2 = 0.0f;
  float amplitude_sub = 0.0f;

  int current_note = 0;

  int attack_time = 1;
  int release_time = 10;

  static float computeSubFrequency(float freq, float octave_divider);
  static float computeFrequencyFromNote(byte note, float pot);
  static float computeFrequencyFromCV(float cv, float pot);
};

} // namespace Engine