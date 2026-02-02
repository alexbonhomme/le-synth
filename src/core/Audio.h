#ifndef AUTOSAVE_AUDIO_H
#define AUTOSAVE_AUDIO_H

#include <Arduino.h>
#include <Audio.h>

namespace Autosave {

namespace audio_config {
static constexpr byte voices_number = 8;

static constexpr float init_frequency = 440.0f; // A4
static constexpr short init_waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
static constexpr float init_amplitude = 0.0f;

static constexpr float main_mix_gain = 0.085f;
static constexpr float master_mix_gain = 0.5f;

static constexpr float filter_env_gain = 0.5f;
} // namespace audio_config

class Audio {
public:
  Audio();

  void begin();

  void noteOn(byte index, float sustain, bool triggerFilterEnvelope = false);
  void noteOff(byte index, bool triggerFilterEnvelope = false);

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
  AudioSynthWaveform oscillators[audio_config::voices_number];
  AudioEffectEnvelope envelopes[audio_config::voices_number];
  AudioMixer4 mixers[audio_config::voices_number / 4];
  AudioMixer4 mixer_master;
  AudioSynthWaveformDc dc_signal;
  AudioEffectEnvelope filter_envelope;
  AudioOutputI2S i2s1;
  AudioConnection patchCords[21];

  bool percussive_mode_ = false;
  float attack_time = 1.0f;
  float release_time = 10.0f;
};

} // namespace Autosave

#endif