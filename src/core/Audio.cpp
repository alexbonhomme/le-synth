#include <synth_waveform.h>

#include "Audio.h"
#include "core/EepromStorage.h"
#include "lib/Logger.h"
#include "waveforms/Waveforms.h"

namespace {
constexpr float kInitLfoFmFrequency = 20.0f;
constexpr float kInitLfoFmAmplitude = 0.0f;

constexpr float kOscMixGain = 0.25f;

constexpr float kFilterEnvGain = 0.5f;

// Random-walk drift: max offset ±this many cents; small steps make it wander.
constexpr float kDriftCentsAmplitude = 0.2f;
// Step size per update (cents); smaller = smoother, larger = more unstable.
constexpr float kDriftStepCents = 0.08f;
constexpr uint8_t kDriftUpdateIntervalMs = 30;

// Per-voice detune (oscillator slop): small fixed cents offset per voice.
constexpr float kVoiceDetuneCents[Autosave::audio_config::voices_number] = {
    -1.5f, -0.8f, -0.4f, 0.1f, 0.5f, 0.9f, 1.4f, -1.2f};

// Initial oscillator state (implementation detail; only used in Audio::begin).
constexpr float kInitFrequency = 440.0f; // A4
constexpr float kInitAmplitude = 0.0f;
constexpr uint8_t kInitWaveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
constexpr int kInitCustomWaveformIndex = 42; // matches EepromStorage default

// MIDI note (0–127) to frequency (Hz). A4 = 440 Hz at index 69. 8 per line =
// exact indices.
constexpr float kMidiNoteToFrequency[128] = {
    8.175799f,     8.661957f,     9.177024f,    9.722718f,    10.300861f,
    10.913382f,    11.562326f,    12.249857f,   12.978272f,   13.750000f,
    14.567618f,    15.433853f,    16.351598f,   17.323914f,   18.354048f,
    19.445436f,    20.601722f,    21.826764f,   23.124651f,   24.499715f,
    25.956544f,    27.500000f,    29.135235f,   30.867706f,   32.703196f,
    34.647829f,    36.708096f,    38.890873f,   41.203445f,   43.653529f,
    46.249303f,    48.999429f,    51.913087f,   55.000000f,   58.270470f,
    61.735413f,    65.406391f,    69.295658f,   73.416192f,   77.781746f,
    82.406889f,    87.307058f,    92.498606f,   97.998859f,   103.826174f,
    110.000000f,   116.540940f,   123.470825f,  130.812783f,  138.591315f,
    146.832384f,   155.563492f,   164.813778f,  174.614116f,  184.997211f,
    195.997718f,   207.652349f,   220.000000f,  233.081881f,  246.941651f,
    261.625565f,   277.182631f,   293.664768f,  311.126984f,  329.627557f,
    349.228231f,   369.994423f,   391.995436f,  415.304698f,  440.000000f,
    466.163762f,   493.883301f,   523.251131f,  554.365262f,  587.329536f,
    622.253967f,   659.255114f,   698.456463f,  739.988845f,  783.990872f,
    830.609395f,   880.000000f,   932.327523f,  987.766603f,  1046.502261f,
    1108.730524f,  1174.659072f,  1244.507935f, 1318.510228f, 1396.912926f,
    1479.977691f,  1567.981744f,  1661.218790f, 1760.000000f, 1864.655046f,
    1975.533205f,  2093.004522f,  2217.461048f, 2349.318143f, 2489.015870f,
    2637.020455f,  2793.825851f,  2959.955382f, 3135.963488f, 3322.437581f,
    3520.000000f,  3729.310092f,  3951.066410f, 4186.009045f, 4434.922096f,
    4698.636287f,  4978.031740f,  5274.040911f, 5587.651703f, 5919.910763f,
    6271.926976f,  6644.875161f,  7040.000000f, 7458.620184f, 7902.132820f,
    8372.018090f,  8869.844191f,  9397.272573f, 9956.063479f, 10548.081821f,
    11175.303406f, 11839.821527f, 12543.853951f};
} // namespace

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
  lfo_fm.frequency(kInitLfoFmFrequency);
  lfo_fm.amplitude(kInitLfoFmAmplitude);

  EepromStorage::loadCustomWaveform(custom_waveform_bank_,
                                    custom_waveform_index_);
  const int16_t *custom_ptr =
      getCustomWaveformPointer(custom_waveform_bank_, custom_waveform_index_);
  if (custom_ptr == nullptr) {
    custom_waveform_bank_ = EepromStorage::kCustomWaveformBankDefault;
    custom_waveform_index_ = EepromStorage::kCustomWaveformIndexDefault;
    custom_ptr = AKWF_OVERTONE[kInitCustomWaveformIndex];
  }

  // Per-voice detune (oscillator slop): small fixed cents offset per voice
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    voice_detune_[i] = powf(2.0f, kVoiceDetuneCents[i] / 1200.0f);
  }

  // Slow pitch drift: per-voice random-walk (unstable, non-periodic)
  randomSeed(micros());
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    voice_base_frequency_[i] = kInitFrequency;
    voice_drift_cents_[i] = 0.0f;
    voice_drift_multiplier_[i] = 1.0f;
  }

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(kInitWaveform);
    applyVoiceFrequency(i);
    oscillators[i].amplitude(kInitAmplitude);
    oscillators[i].arbitraryWaveform(custom_ptr, 172.0f);

    envelopes[i].attack(attack_time);
    envelopes[i].hold(0);
    envelopes[i].decay(0);
    envelopes[i].sustain(1.0);
    envelopes[i].release(release_time);

    mixers[i / 4].gain(i % 4, kOscMixGain);
  }

  mixer_master.gain(0, 0.5f);
  mixer_master.gain(1, 0.5f);

  amplifier_master.gain(audio_config::master_gain);

  // Configure DC signal for filter envelope (constant voltage source)
  dc_signal.amplitude(kFilterEnvGain);

  // Configure filter envelope with the same ADSR values as envelopes
  filter_envelope.attack(attack_time);
  filter_envelope.hold(0);
  filter_envelope.decay(0);
  filter_envelope.sustain(1.0);
  filter_envelope.release(release_time);
  filter_envelope.releaseNoteOn(0);

  last_drift_update_ms_ = millis();
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

void Audio::applyVoiceFrequency(uint8_t index) {
  float f = voice_base_frequency_[index] * voice_detune_[index] *
            voice_drift_multiplier_[index];
  oscillators[index].frequency(f);
}

void Audio::updateOscillatorFrequency(uint8_t index, float frequency) {
  voice_base_frequency_[index] = frequency;
  applyVoiceFrequency(index);
}

void Audio::updateAllOscillatorsFrequency(float frequency) {
  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    voice_base_frequency_[i] = frequency;
    applyVoiceFrequency(i);
  }
}

void Audio::updateDrift() {
  uint32_t now = millis();
  if (now - last_drift_update_ms_ < kDriftUpdateIntervalMs) {
    return;
  }
  last_drift_update_ms_ = now;

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    // Random step: ±kDriftStepCents per update (non-periodic wander)
    float step = (static_cast<float>(random(0, 2001) - 1000) / 1000.0f) *
                 kDriftStepCents;
    voice_drift_cents_[i] += step;
    if (voice_drift_cents_[i] > kDriftCentsAmplitude) {
      voice_drift_cents_[i] = kDriftCentsAmplitude;
    } else if (voice_drift_cents_[i] < -kDriftCentsAmplitude) {
      voice_drift_cents_[i] = -kDriftCentsAmplitude;
    }
    voice_drift_multiplier_[i] = powf(2.0f, voice_drift_cents_[i] / 1200.0f);
    applyVoiceFrequency(i);
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

  return kMidiNoteToFrequency[note];
}

/**
 * Compute the gain of the waveform based on the waveform type.
 *
 * @param waveform The waveform type.
 * @return The gain of the waveform.
 */
float Audio::computeGainFromWaveform(uint8_t waveform) {
  float gain = kOscMixGain;

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