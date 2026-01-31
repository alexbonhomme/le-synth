#include <cmath>

#include "Audio.h"
#include "synth_waveform.h"

namespace Autosave {

Audio::Audio()
    : patchCord1(waveform1, 0, mixer1, 0), patchCord2(waveform2, 0, mixer1, 1),
      patchCord3(waveform3, 0, mixer1, 2), patchCord4(mixer1, envelope),
      patchCord5(envelope, 0, i2s1, 1),
      patchCord6(dc_signal, 0, envelope_filter, 0),
      patchCord7(envelope_filter, 0, i2s1, 0) {}

void Audio::begin() {
#ifdef DEBUG
  Serial.println("Initializing audio");
#endif

  // Audio connections require memory to work. For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(20);

  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);
  waveform3.begin(WAVEFORM_TRIANGLE);

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);
  waveform3.frequency(freq_sub);

  waveform1.amplitude(amplitude);
  waveform2.amplitude(amplitude_2);
  waveform3.amplitude(amplitude_sub);

  mixer1.gain(0, defaults::main_mix_gain);
  mixer1.gain(1, defaults::main_mix_gain);
  mixer1.gain(2, defaults::main_mix_gain);

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
  freq = computeFrequencyFromNote(current_note, 0);

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, detune);

  // update sub oscillator
  freq_sub = computeSubFrequency(freq, 2.0);

  AudioNoInterrupts();

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);
  waveform3.frequency(freq_sub);

  envelope.sustain(sustain);
  envelope_filter.sustain(sustain * 0.75);

  envelope.noteOn();
  envelope_filter.noteOn();

  AudioInterrupts();
}

void Audio::noteOff() {
  envelope.noteOff();
  envelope_filter.noteOff();
}

void Audio::updateEnvelopeMode(bool percussive_mode) {
  if (percussive_mode) {
    envelope.decay(release_time);
    envelope.sustain(0.0);
    envelope.release(0);
    envelope_filter.decay(release_time);
    envelope_filter.sustain(0.0);
    envelope_filter.release(0);
  } else {
    envelope.decay(0);
    envelope.sustain(1.0);
    envelope.release(release_time);
    envelope_filter.decay(0);
    envelope_filter.sustain(1.0);
    envelope_filter.release(release_time);
  }
}

void Audio::updateOsc2Frequency(float frequency) {
  freq_2 = computeFrequencyFromNote(current_note, frequency);
  waveform2.frequency(freq_2);
}

void Audio::updateOsc2Amplitude(float amplitude) {
  waveform2.amplitude(amplitude);
}

void Audio::updateSubAmplitude(float amplitude) {
  waveform3.amplitude(amplitude);
}

void Audio::updateWaveform(int waveform) {
#ifdef DEBUG
  Serial.println("Osc 1 & 2 waveform:");
  Serial.println(waveform);
#endif

  AudioNoInterrupts();

  waveform1.begin(waveform);
  waveform2.begin(waveform);
  // waveform3.begin(waveform);

  if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
    waveform1.pulseWidth(0.25);
    waveform2.pulseWidth(0.25);
    // waveform3.pulseWidth(0.25);

    // Gain correction
    mixer1.gain(0, defaults::main_mix_gain * 2.0f);
    mixer1.gain(1, defaults::main_mix_gain * 2.0f);
    // mixer1.gain(2, defaults::main_mix_gain * 2.0f);
  } else {
    mixer1.gain(0, defaults::main_mix_gain);
    mixer1.gain(1, defaults::main_mix_gain);
    // mixer1.gain(2, defaults::main_mix_gain);
  }

  AudioInterrupts();
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

  envelope.release(release_time);
  envelope_filter.release(release_time);

  AudioInterrupts();
}

float Audio::computeSubFrequency(float freq, float octave_divider) {
  return octave_divider > 0.0 ? freq / octave_divider : 0.0;
}

/**
 * Compute the frequency of the waveform based on the note and mod input.
 *
 * The note input is the MIDI note number, and the mod input is from 0 to 1.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float Audio::computeFrequencyFromNote(byte note, float mod) {
  float offset = mod * 16.0 - 8.0; // -8 to 8
  float note_offset = note + offset;

  return 440.0 *
         powf(2.0, (float)(note_offset - 73) * 0.08333333); // 440.0 = A4
}

/**
 * Compute the frequency of the waveform based on the mod and cv input.
 *
 * The mod input is should be from 0 to 1, and the cv input from 0 to 10.
 */
float Audio::computeFrequencyFromCV(float cv, float mod) {
  float offset = mod * 2.0 - 1.0; // -1 to 1
  float cv_offset = cv + offset;

  if (cv_offset < 0) {
    cv_offset = 0.0;
  } else if (cv_offset > 10) {
    cv_offset = 10.0;
  }

  return 32.7032 * pow(2, cv_offset); // f0 = C0 = 32.7032
}

} // namespace Autosave