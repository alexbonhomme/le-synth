#include "synth_waveform.h"

#include "Audio.h"
#include "lib/Logger.h"

namespace Autosave {

Audio::Audio()
    : patchCords{{oscillators[0], 0, envelopes[0], 0},
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

  for (int i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(audio_config::init_waveform);
    oscillators[i].frequency(audio_config::init_frequency);
    oscillators[i].amplitude(audio_config::init_amplitude);
  }

  for (int i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].attack(attack_time);
    envelopes[i].hold(0);
    envelopes[i].decay(0);
    envelopes[i].sustain(1.0);
    envelopes[i].release(release_time);
  }

  for (unsigned int i = 0; i < sizeof(mixers) / sizeof(mixers[0]); i++) {
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

void Audio::noteOn(byte index, float sustain, bool triggerFilterEnvelope) {
  envelopes[index].sustain(sustain);
  envelopes[index].noteOn();

  if (triggerFilterEnvelope) {
    filter_envelope.sustain(sustain * 0.85f);
    filter_envelope.noteOn();
  }
}

void Audio::noteOff(byte index, bool triggerFilterEnvelope) {
  envelopes[index].noteOff();

  if (triggerFilterEnvelope) {
    filter_envelope.noteOff();
  }
}

void Audio::noteOffAll() {
  AutosaveLib::Logger::debug("Audio::noteOffAll");

  for (int i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].noteOff();
  }

  filter_envelope.noteOff();
}

void Audio::updateOscillatorFrequency(byte index, float frequency) {
  oscillators[index].frequency(frequency);
}

void Audio::updateAllOscillatorsFrequency(float frequency) {
  for (int i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].frequency(frequency);
  }
}

void Audio::updateOscillatorAmplitude(byte index, float amplitude) {
  oscillators[index].amplitude(amplitude);
}

void Audio::updateAllOscillatorsAmplitude(float amplitude) {
  for (int i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].amplitude(amplitude);
  }
}

void Audio::updateOscillatorWaveform(byte index, byte waveform) {
  oscillators[index].begin(waveform);

  float gain = audio_config::osc_mix_gain;
  // if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
  //   oscillators[index].pulseWidth(0.25);

  //   // Gain correction for pulse waveform
  //   gain = gain * 2.0f;
  // }

  mixers[index % audio_config::voices_number].gain(index % 4, gain);
}

void Audio::updateAllOscillatorsWaveform(byte waveform) {
  for (int i = 0; i < audio_config::voices_number; i++) {
    oscillators[i].begin(waveform);
  }
}

void Audio::updateEnvelopeMode(bool percussive_mode) {
  percussive_mode_ = percussive_mode;

  float decay = percussive_mode_ ? release_time : 0.0f;
  float sustain = percussive_mode_ ? 0.0f : 1.0f;
  float release = percussive_mode_ ? 0.0f : release_time;

  for (int i = 0; i < audio_config::voices_number; i++) {
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

  for (int i = 0; i < audio_config::voices_number; i++) {
    envelopes[i].attack(attack_time);
  }

  filter_envelope.attack(attack_time);
}

void Audio::updateRelease(float release) {
  release_time = release * 598.0f + 2.0f; // 2 to 600ms

  if (percussive_mode_) {
    for (int i = 0; i < audio_config::voices_number; i++) {
      envelopes[i].decay(release_time);
    }

    filter_envelope.decay(release_time);
  } else {
    for (int i = 0; i < audio_config::voices_number; i++) {
      envelopes[i].release(release_time);
    }

    filter_envelope.release(release_time);
  }
}

float Audio::computeFrequencyFromNote(byte note) {
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