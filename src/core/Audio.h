#ifndef AUTOSAVE_AUDIO_H
#define AUTOSAVE_AUDIO_H

#include <Audio.h>
#include <cmath>
#include <cstdint>

#include "lib/Logger.h"


namespace Autosave {

namespace audio_config {
static constexpr uint8_t voices_number = 8;
static constexpr float master_gain = 0.75f;
} // namespace audio_config

class Audio {
public:
  Audio();

  void begin();

  void noteOn(uint8_t index, float sustain, bool triggerFilterEnvelope = false);
  void noteOff(uint8_t index, bool triggerFilterEnvelope = false);
  void noteOffAll();

  void updateEnvelopeMode(bool percussive_mode);

  void updateLFOFrequency(float frequency);
  void updateLFOAmplitude(float amplitude);

  void updateOscillatorFrequency(uint8_t index, float frequency);
  void updateAllOscillatorsFrequency(float frequency);

  /** Advance slow pitch drift (call from main loop, rate-limited internally).
   */
  void updateDrift();

  void updateOscillatorAmplitude(uint8_t index, float amplitude);
  void updateAllOscillatorsAmplitude(float amplitude);

  void updateAllOscillatorsWaveform(uint8_t waveform);

  /** Custom (arbitrary) waveform: bank 0=FM, 1=Granular, 2=Overtone; index
   * within bank. */
  void setCustomWaveform(uint8_t bank, uint8_t index);
  void getCustomWaveform(uint8_t *out_bank, uint8_t *out_index) const;
  /** Apply current custom waveform table to all oscillators (when in arbitrary
   * mode). */
  void applyCustomWaveform();

  void updateAttack(float attack);
  void updateRelease(float release);

  void normalizeMasterGain(uint8_t oscillators_count) {
    // @TODO: use table instead of log2f to improve performance
    float gain_correction = 3.0f / std::log2f(oscillators_count + 10.0f);
    float normalized_gain = audio_config::master_gain * gain_correction;

    AutosaveLib::Logger::debug("Normalized gain: " + String(normalized_gain));

    amplifier_master.gain(normalized_gain);
  }

  static float computeFrequencyFromNote(uint8_t note);
  static float computeFrequencyFromCV(float cv);

private:
  AudioSynthWaveformSine lfo_fm;
  AudioSynthWaveformModulated oscillators[audio_config::voices_number];
  AudioEffectEnvelope envelopes[audio_config::voices_number];
  AudioMixer4 mixers[audio_config::voices_number / 4];
  AudioMixer4 mixer_master;
  AudioAmplifier amplifier_master;
  AudioSynthWaveformDc dc_signal;
  AudioEffectEnvelope filter_envelope;
  AudioOutputI2S i2s1;
  AudioConnection patchCords[30];

  bool percussive_mode_ = false;
  float attack_time = 1.0f;
  float release_time = 15.0f;

  uint8_t custom_waveform_bank_ = 2;
  uint8_t custom_waveform_index_ = 42;

  /** Per-voice detune multipliers (oscillator slop); applied to frequency. */
  float voice_detune_[audio_config::voices_number];

  /** Per-voice base frequency (nominal pitch before detune and drift). */
  float voice_base_frequency_[audio_config::voices_number];
  /** Per-voice drift offset in cents (random walk); applied with detune. */
  float voice_drift_cents_[audio_config::voices_number];
  /** Per-voice drift multiplier (1.0 ± small); applied with detune. */
  float voice_drift_multiplier_[audio_config::voices_number];
  /** Last time updateDrift() ran (ms). */
  uint32_t last_drift_update_ms_ = 0;

  void applyVoiceFrequency(uint8_t index);

  const int16_t *getCustomWaveformPointer(uint8_t bank, uint8_t index) const;
  float computeGainFromWaveform(uint8_t waveform);
};

} // namespace Autosave

#endif