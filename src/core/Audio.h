#ifndef AUTOSAVE_AUDIO_H
#define AUTOSAVE_AUDIO_H

#include <Arduino.h>
#include <Audio.h>

#define VOICES_NUMBER 8

namespace Autosave {

namespace defaults {
static constexpr float init_frequency = 440.0f; // A4
static constexpr short init_waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
static constexpr float init_amplitude = 0.0f;

static constexpr float main_mix_gain = 0.085f;
static constexpr float master_mix_gain = 0.5f;

static constexpr float filter_env_gain = 0.5f;
} // namespace defaults

class Audio {
public:
  Audio();

  void begin();

  void noteOn(float sustain);
  void noteOff();

  void updateEnvelopeMode(bool percussive_mode);

  void updateOscillatorFrequency(byte index, float frequency);
  void updateAllOscillatorsFrequency(float frequency);
  void updateOscillatorAmplitude(byte index, float amplitude);
  void updateAllOscillatorsAmplitude(float amplitude);
  void updateOscillatorWaveform(byte index, byte waveform);
  void updateAllOscillatorsWaveform(byte waveform);

  void updateAttack(float attack);
  void updateRelease(float release);

  static float computeFrequencyFromNote(byte note);
  static float computeFrequencyFromCV(float cv, float mod);

private:
  AudioSynthWaveform oscillators[VOICES_NUMBER];
  AudioEffectEnvelope envelopes[VOICES_NUMBER];
  AudioMixer4 mixers[VOICES_NUMBER / 4];
  AudioMixer4 mixer_master;
  AudioSynthWaveformDc dc_signal;
  AudioEffectEnvelope filter_envelope;
  AudioOutputI2S i2s1;
  AudioConnection patchCords[21];

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
};

} // namespace Autosave

#endif