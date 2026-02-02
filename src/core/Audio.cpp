#include "synth_waveform.h"

#include "Audio.h"
#include "lib/Logger.h"

namespace Autosave {

Audio::Audio()
    : patchCord{
          {osc[0], 0, mixer[0], 0},       {osc[1], 0, mixer[0], 1},
          {osc[2], 0, mixer[0], 2},       {osc[3], 0, mixer[1], 0},
          {osc[4], 0, mixer[1], 1},       {osc[5], 0, mixer[1], 2},
          {osc[6], 0, mixer[1], 3},       {mixer[0], 0, mixer_master, 0},
          {mixer[1], 0, mixer_master, 1}, {mixer_master, 0, envelope, 0},
          {envelope, 0, i2s1, 1},         {dc_signal, 0, envelope_filter, 0},
          {envelope_filter, 0, i2s1, 0}} {}

void Audio::begin() {
  AutosaveLib::Logger::info("Initializing Audio module");

  // Audio connections require memory to work. For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(20);

  // @TODO: init oscillators on states
  osc[0].begin(current_waveform);
  osc[1].begin(current_waveform);
  osc[2].begin(WAVEFORM_TRIANGLE);

  osc[0].frequency(freq);
  osc[1].frequency(freq_2);
  osc[2].frequency(freq_sub);

  osc[0].amplitude(amplitude);
  osc[1].amplitude(amplitude_2);
  osc[2].amplitude(amplitude_sub);

  mixer[0].gain(0, defaults::main_mix_gain);
  mixer[0].gain(1, defaults::main_mix_gain);
  mixer[0].gain(2, defaults::main_mix_gain);

  mixer[1].gain(0, defaults::main_mix_gain);
  mixer[1].gain(1, defaults::main_mix_gain);
  mixer[1].gain(2, defaults::main_mix_gain);
  mixer[1].gain(3, defaults::main_mix_gain);

  // Configure DC signal for envelope2 (constant voltage source)
  dc_signal.amplitude(defaults::filter_env_gain);

  // Configure envelope1
  envelope.attack(attack_time);
  envelope.hold(0);
  envelope.decay(0);
  envelope.sustain(1.0);
  envelope.release(release_time);
  envelope.releaseNoteOn(0);

  // Configure envelope2 with the same ADSR values as envelope1
  envelope_filter.attack(attack_time);
  envelope_filter.hold(0);
  envelope_filter.decay(0);
  envelope_filter.sustain(1.0);
  envelope_filter.release(release_time);
  envelope_filter.releaseNoteOn(0);
}

void Audio::noteOn(byte note, byte velocity, float detune) {
  current_note = note;
  float sustain = (float)velocity / 127.0f;

  // update main oscillator
  freq = computeFrequencyFromNote(current_note, 0.0f);

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, detune);

  // update sub oscillator
  freq_sub = computeSubFrequency(freq, 2.0f);

  AudioNoInterrupts();

  osc[0].frequency(freq);
  osc[1].frequency(freq_2);
  osc[2].frequency(freq_sub);

  envelope.sustain(sustain);
  envelope_filter.sustain(sustain * 0.75f);

  envelope.noteOn();
  envelope_filter.noteOn();

  AudioInterrupts();
}

void Audio::noteOff() {
  envelope.noteOff();
  envelope_filter.noteOff();
}

void Audio::updateOsc2Frequency(float frequency) {
  freq_2 = computeFrequencyFromNote(current_note, frequency);
  osc[1].frequency(freq_2);
}

void Audio::updateOsc2Amplitude(float amplitude) {
  osc[1].amplitude(amplitude);
}

void Audio::updateSubAmplitude(float amplitude) { osc[2].amplitude(amplitude); }

void Audio::updateWaveform(int waveform) {
  AutosaveLib::Logger::debug("Osc 1 & 2 waveform: " + String(waveform));

  AudioNoInterrupts();

  osc[0].begin(waveform);
  osc[1].begin(waveform);
  // waveform3.begin(waveform);

  if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
    osc[0].pulseWidth(0.25);
    osc[1].pulseWidth(0.25);
    // waveform3.pulseWidth(0.25);

    // Gain correction
    mixer[0].gain(0, defaults::main_mix_gain * 2.0f);
    mixer[0].gain(1, defaults::main_mix_gain * 2.0f);
    // mixer1.gain(2, defaults::main_mix_gain * 2.0f);
  } else {
    mixer[0].gain(0, defaults::main_mix_gain);
    mixer[0].gain(1, defaults::main_mix_gain);
    // mixer1.gain(2, defaults::main_mix_gain);
  }

  AudioInterrupts();
}

void Audio::updateEnvelopeMode(bool percussive_mode) {
  percussive_mode_ = percussive_mode;

  if (percussive_mode_) {
    envelope.decay(release_time);
    envelope.sustain(0.0f);
    envelope.release(0);
    envelope_filter.decay(release_time);
    envelope_filter.sustain(0.0f);
    envelope_filter.release(0);
  } else {
    envelope.decay(0);
    envelope.sustain(1.0f);
    envelope.release(release_time);
    envelope_filter.decay(0);
    envelope_filter.sustain(1.0f);
    envelope_filter.release(release_time);
  }
}

void Audio::updateAttack(float attack) {
  attack_time = attack * 149 + 1; // 1 to 150ms

  AudioNoInterrupts();

  envelope.attack(attack_time);
  envelope_filter.attack(attack_time);

  AudioInterrupts();
}

void Audio::updateRelease(float release) {
  release_time = release * 598 + 2; // 2 to 600ms

  AudioNoInterrupts();

  if (percussive_mode_) {
    envelope.decay(release_time);
    envelope_filter.decay(release_time);
  } else {
    envelope.release(release_time);
    envelope_filter.release(release_time);
  }

  AudioInterrupts();
}

float Audio::computeSubFrequency(float freq, float octave_divider) {
  return octave_divider > 0 ? freq / octave_divider : 0.0f;
}

/**
 * @TODO: remove mod and apply detune of frequency
 *
 * Compute the frequency of the waveform based on the note and mod input.
 *
 * The note input is the MIDI note number, and the mod input is from 0 to 1.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float Audio::computeFrequencyFromNote(byte note, float mod) {
  float offset = mod * 16.0f - 8.0f; // -8 to 8
  float note_offset = note + offset;

  return 440.0f * powf(2.0f, (note_offset - 73.0f) * 0.08333333f); // 440.0 = A4
}

/**
 * @TODO: NOT USED YET
 * Compute the frequency of the waveform based on the mod and cv input.
 *
 * The mod input is should be from 0 to 1, and the cv input from 0 to 10.
 */
float Audio::computeFrequencyFromCV(float cv, float mod) {
  float offset = mod * 2.0f - 1.0f; // -1 to 1
  float cv_offset = cv + offset;

  if (cv_offset < 0) {
    cv_offset = 0.0f;
  } else if (cv_offset > 10) {
    cv_offset = 10.0f;
  }

  return 32.7032f * powf(2.0f, cv_offset); // f0 = C0 = 32.7032
}

} // namespace Autosave