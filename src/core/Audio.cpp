#include <synth_waveform.h>

#include "Audio.h"
#include "core/EepromStorage.h"
#include "lib/Logger.h"

namespace Autosave {

enum CustomWaveformBank {
  CUSTOM_WAVEFORM_BANK_FM = 0,
  CUSTOM_WAVEFORM_BANK_GRANULAR = 1,
  CUSTOM_WAVEFORM_BANK_OVERTONE = 2,
};

Audio::Audio()
    : patchCords{{lfo_fm, 0, oscillators[0], 0},
                 {lfo_fm, 0, oscillators[1], 0},
                 {lfo_fm, 0, oscillators[2], 0},
                 {lfo_fm, 0, oscillators[3], 0},
                 {lfo_fm, 0, oscillators[4], 0},
                 {lfo_fm, 0, oscillators[5], 0},
                 {lfo_fm, 0, oscillators[6], 0},
                 {lfo_fm, 0, oscillators[7], 0},
                 {oscillators[0], 0, envelopes[0], 0},
                 {oscillators[1], 0, envelopes[1], 0},
                 {oscillators[2], 0, envelopes[2], 0},
                 {oscillators[3], 0, envelopes[3], 0},
                 {oscillators[4], 0, envelopes[4], 0},
                 {oscillators[5], 0, envelopes[5], 0},
                 {oscillators[6], 0, envelopes[6], 0},
                 {oscillators[7], 0, envelopes[7], 0},
                 {envelopes[0], 0, mixers[0], 0},
                 {envelopes[1], 0, mixers[0], 1},
                 {envelopes[2], 0, mixers[0], 2},
                 {envelopes[3], 0, mixers[0], 3},
                 {envelopes[4], 0, mixers[1], 0},
                 {envelopes[5], 0, mixers[1], 1},
                 {envelopes[6], 0, mixers[1], 2},
                 {envelopes[7], 0, mixers[1], 3},
                 {mixers[0], 0, mixer_master, 0},
                 {mixers[1], 0, mixer_master, 1},
                 {mixer_master, 0, amplifier_master, 0},
                 {amplifier_master, 0, i2s1, 1},
                 {dc_signal, 0, filter_envelope, 0},
                 {filter_envelope, 0, i2s1, 0}} {}

void Audio::begin() {
  AutosaveLib::Logger::info("Initializing Audio module");

  // Audio connections require memory to work. For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(20);

  // Configure LFO
  lfo_fm.frequency(audio_config::init_lfo_fm_frequency);
  lfo_fm.amplitude(audio_config::init_lfo_fm_amplitude);

  EepromStorage::loadCustomWaveform(custom_waveform_bank_,
                                    custom_waveform_index_);
  const int16_t *custom_ptr =
      getCustomWaveformPointer(custom_waveform_bank_, custom_waveform_index_);
  if (custom_ptr == nullptr) {
    custom_waveform_bank_ = 2;
    custom_waveform_index_ = 42;
    custom_ptr = AKWF_OVERTONE[42];
  }

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(audio_config::init_waveform);
    oscillators[i].frequency(audio_config::init_frequency);
    oscillators[i].amplitude(audio_config::init_amplitude);
    oscillators[i].arbitraryWaveform(custom_ptr, 172.0f);

    envelopes[i].attack(attack_time);
    envelopes[i].hold(0);
    envelopes[i].decay(0);
    envelopes[i].sustain(1.0);
    envelopes[i].release(release_time);

    mixers[i / 4].gain(i % 4, audio_config::osc_mix_gain);
  }

  mixer_master.gain(0, 0.5f);
  mixer_master.gain(1, 0.5f);

  amplifier_master.gain(audio_config::master_gain);

  // Configure DC signal for filter envelope (constant voltage source)
  dc_signal.amplitude(audio_config::filter_env_gain);

  // Configure filter envelope with the same ADSR values as envelopes
  filter_envelope.attack(attack_time);
  filter_envelope.hold(0);
  filter_envelope.decay(0);
  filter_envelope.sustain(1.0);
  filter_envelope.release(release_time);
  filter_envelope.releaseNoteOn(0);
}

void Audio::noteOn(uint8_t index, float sustain, bool triggerFilterEnvelope) {
  envelopes[index].sustain(sustain);
  envelopes[index].noteOn();

  if (triggerFilterEnvelope) {
    filter_envelope.sustain(sustain * 0.85f);
    filter_envelope.noteOn();
  }
}

void Audio::noteOff(uint8_t index, bool triggerFilterEnvelope) {
  envelopes[index].noteOff();

  if (triggerFilterEnvelope) {
    filter_envelope.noteOff();
  }
}

void Audio::noteOffAll() {
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].noteOff();
  }

  filter_envelope.noteOff();
}

void Audio::updateLFOFrequency(float frequency) { lfo_fm.frequency(frequency); }

void Audio::updateLFOAmplitude(float amplitude) { lfo_fm.amplitude(amplitude); }

void Audio::updateOscillatorFrequency(uint8_t index, float frequency) {
  oscillators[index].frequency(frequency);
}

void Audio::updateAllOscillatorsFrequency(float frequency) {
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].frequency(frequency);
  }
}

void Audio::updateOscillatorAmplitude(uint8_t index, float amplitude) {
  oscillators[index].amplitude(amplitude);
}

void Audio::updateAllOscillatorsAmplitude(float amplitude) {
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].amplitude(amplitude);
  }
}

void Audio::updateAllOscillatorsWaveform(uint8_t waveform) {
  float gain = computeGainFromWaveform(waveform);

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(waveform);
    mixers[i / 4].gain(i % 4, gain);
  }
}

const int16_t *Audio::getCustomWaveformPointer(uint8_t bank,
                                               uint8_t index) const {
  switch (bank) {
  case CUSTOM_WAVEFORM_BANK_FM:
    return (index < AKWF_FM_COUNT) ? AKWF_FM[index] : nullptr;
  case CUSTOM_WAVEFORM_BANK_GRANULAR:
    return (index < AKWF_GRANULAR_COUNT) ? AKWF_GRANULAR[index] : nullptr;
  case CUSTOM_WAVEFORM_BANK_OVERTONE:
    return (index < AKWF_OVERTONE_COUNT) ? AKWF_OVERTONE[index] : nullptr;
  default:
    return nullptr;
  }
}

void Audio::setCustomWaveform(uint8_t bank, uint8_t index) {
  if (bank > CUSTOM_WAVEFORM_BANK_OVERTONE) {
    return;
  }
  size_t max_index = 0;
  switch (bank) {
  case CUSTOM_WAVEFORM_BANK_FM:
    max_index = AKWF_FM_COUNT;
    break;
  case CUSTOM_WAVEFORM_BANK_GRANULAR:
    max_index = AKWF_GRANULAR_COUNT;
    break;
  case CUSTOM_WAVEFORM_BANK_OVERTONE:
    max_index = AKWF_OVERTONE_COUNT;
    break;
  }
  if (index >= max_index) {
    return;
  }
  custom_waveform_bank_ = bank;
  custom_waveform_index_ = index;
}

void Audio::getCustomWaveform(uint8_t *out_bank, uint8_t *out_index) const {
  if (out_bank != nullptr) {
    *out_bank = custom_waveform_bank_;
  }
  if (out_index != nullptr) {
    *out_index = custom_waveform_index_;
  }
}

void Audio::applyCustomWaveform() {
  const int16_t *ptr =
      getCustomWaveformPointer(custom_waveform_bank_, custom_waveform_index_);
  if (ptr == nullptr) {
    return;
  }
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].arbitraryWaveform(ptr, 172.0f);
  }
}

void Audio::updateEnvelopeMode(bool percussive_mode) {
  percussive_mode_ = percussive_mode;

  float decay = percussive_mode_ ? release_time : 0.0f;
  float sustain = percussive_mode_ ? 0.0f : 1.0f;
  float release = percussive_mode_ ? 0.0f : release_time;

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].decay(decay);
    envelopes[i].sustain(sustain);
    envelopes[i].release(release);
  }

  filter_envelope.decay(decay);
  filter_envelope.sustain(sustain * 0.75f);
  filter_envelope.release(release);
}

void Audio::updateAttack(float attack) {
  attack_time = attack * 149.0f + 1.0f; // 1 to 150ms

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].attack(attack_time);
  }

  filter_envelope.attack(attack_time);
}

void Audio::updateRelease(float release) {
  release_time = release * 598.0f + 2.0f; // 2 to 600ms

  if (percussive_mode_) {
    for (uint8_t i = 0; i < audio_config::voices_number; i++) {
      envelopes[i].decay(release_time);
    }

    filter_envelope.decay(release_time);

    return;
  }

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].release(release_time);
  }

  filter_envelope.release(release_time);
}

/**
 * Get the frequency of the note from the MIDI note number.
 *
 * @param note The MIDI note number.
 * @return The frequency of the note.
 */
float Audio::computeFrequencyFromNote(uint8_t note) {
  if (note < 0) {
    return 0.0f;
  }

  if (note > 127) {
    note = 127;
  }

  return audio_config::midi_note_to_frequency[note];
}

/**
 * Compute the gain of the waveform based on the waveform type.
 *
 * @param waveform The waveform type.
 * @return The gain of the waveform.
 */
float Audio::computeGainFromWaveform(uint8_t waveform) {
  float gain = audio_config::osc_mix_gain;

  if (waveform == WAVEFORM_BANDLIMIT_SQUARE) {
    return gain * 0.75f;
  }
  if (waveform == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE) {
    return gain * 0.85f;
  }

  return gain; // No gain correction
}

/**
 * @TODO: NOT USED YET
 * Compute the frequency of the waveform based on the mod and cv input.
 *
 * The mod input is should be from 0 to 1, and the cv input from 0 to 10.
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float Audio::computeFrequencyFromCV(float cv) {
  if (cv < 0) {
    cv = 0.0f;
  } else if (cv > 10) {
    cv = 10.0f;
  }

  return 32.7032f * powf(2.0f, cv); // f0 = C0 = 32.7032
}

} // namespace Autosave