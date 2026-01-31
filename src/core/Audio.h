#ifndef AUTOSAVE_AUDIO_H
#define AUTOSAVE_AUDIO_H

#include <Arduino.h>
#include <Audio.h>

namespace Autosave {

namespace defaults {
static constexpr float main_mix_gain = 0.085f;
static constexpr float filter_env_gain = 0.5f;
} // namespace defaults

class Audio {
public:
  Audio();

  void begin();
  void noteOn(byte note, byte velocity, float detune);
  void noteOff();
  void updateEnvelopeMode(bool percussive_mode);
  void updateOsc2Frequency(float frequency);
  void updateOsc2Amplitude(float amplitude);
  void updateSubAmplitude(float amplitude);
  void updateWaveform(int waveform);
  void updateAttack(float attack);
  void updateRelease(float release);

private:
  // GUItool: begin automatically generated code
  AudioSynthWaveform waveform1;
  AudioSynthWaveform waveform2;
  AudioSynthWaveform waveform3;
  AudioMixer4 mixer1;
  AudioEffectEnvelope envelope;
  AudioEffectEnvelope envelope_filter;
  AudioSynthWaveformDc dc_signal;
  AudioOutputI2S i2s1;
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

} // namespace Autosave

#endif