#include <synth_waveform.h>

#include "Audio.h"
#include "lib/Logger.h"

namespace Autosave {

Audio::Audio()
    : patchCords{{lfo, 0, oscillators[0], 0},
                 {lfo, 0, oscillators[1], 0},
                 {lfo, 0, oscillators[2], 0},
                 {lfo, 0, oscillators[3], 0},
                 {lfo, 0, oscillators[4], 0},
                 {lfo, 0, oscillators[5], 0},
                 {lfo, 0, oscillators[6], 0},
                 {lfo, 0, oscillators[7], 0},
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
  lfo.frequency(audio_config::init_lfo_frequency);
  lfo.amplitude(audio_config::init_lfo_amplitude);

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(audio_config::init_waveform);
    oscillators[i].frequency(audio_config::init_frequency);
    oscillators[i].amplitude(audio_config::init_amplitude);
  }

  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].attack(attack_time);
    envelopes[i].hold(0);
    envelopes[i].decay(0);
    envelopes[i].sustain(1.0);
    envelopes[i].release(release_time);
  }

  for (uint8_t i = 0; i < 2; i++) {
    mixers[i].gain(0, audio_config::osc_mix_gain);
    mixers[i].gain(1, audio_config::osc_mix_gain);
    mixers[i].gain(2, audio_config::osc_mix_gain);
    mixers[i].gain(3, audio_config::osc_mix_gain);
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

void Audio::updateLFOFrequency(float frequency) { lfo.frequency(frequency); }

void Audio::updateLFOAmplitude(float amplitude) { lfo.amplitude(amplitude); }

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

void Audio::updateOscillatorWaveform(uint8_t index, uint8_t waveform) {
  oscillators[index].begin(waveform);

  float gain = audio_config::osc_mix_gain;
  // if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
  //   oscillators[index].pulseWidth(0.25);

  //   // Gain correction for pulse waveform
  //   gain = gain * 2.0f;
  // }

  mixers[index % audio_config::voices_number].gain(index % 4, gain);
}

void Audio::updateAllOscillatorsWaveform(uint8_t waveform) {
  // @TODO: 
  // float gain = audio_config::osc_mix_gain;
  // if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
  //   oscillators[index].pulseWidth(0.25);

  //   // Gain correction for pulse waveform
  //   gain = gain * 2.0f;
  // }


  for (uint8_t i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(waveform);

    // Gain correction
    // mixers[i % audio_config::voices_number].gain(i % 4, gain);
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
  } else {
    for (uint8_t i = 0; i < audio_config::voices_number; i++) {
      envelopes[i].release(release_time);
    }

    filter_envelope.release(release_time);
  }
}

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