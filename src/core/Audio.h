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
  AudioSynthWaveform osc[7];
  AudioMixer4 mixer[2];
  AudioMixer4 mixer_master;
  AudioEffectEnvelope envelope;
  AudioEffectEnvelope envelope_filter;
  AudioSynthWaveformDc dc_signal;
  AudioOutputI2S i2s1;
  AudioConnection patchCord[13];

  int current_waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;

  float freq = 440.0f;
  float freq_2 = 440.0f;
  float freq_sub = 220.0f;

  float amplitude = 1.0f;
  float amplitude_2 = 0.0f;
  float amplitude_sub = 0.0f;

  int current_note = 0;

  bool percussive_mode_ = false;
  int attack_time = 1;
  int release_time = 10;

  static float computeSubFrequency(float freq, float octave_divider);
  static float computeFrequencyFromNote(byte note, float pot);
  static float computeFrequencyFromCV(float cv, float pot);
};

} // namespace Autosave

#endif