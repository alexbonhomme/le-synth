#include "Audio.h"
#include "synth_waveform.h"

#include <cmath>

namespace Autosave {

Audio::Audio()
    : patchCord1(waveform1, 0, mixer1, 0), patchCord2(waveform2, 0, mixer1, 1),
      patchCord3(waveform_sub, 0, filter1, 0),
      patchCord4(filter1, 0, mixer1, 2), patchCord5(mixer1, envelope1),
      patchCord6(envelope1, 0, i2s1, 1), patchCord7(dc_signal, 0, envelope2, 0),
      patchCord8(envelope2, 0, i2s1, 0) {}

void Audio::begin() {
#ifdef DEBUG
  Serial.println("Initializing audio");
#endif

  // Audio connections require memory to work. For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(20);

  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);
  waveform_sub.begin(current_waveform_sub);

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);
  waveform_sub.frequency(freq_sub);

  filter1.frequency(freq_sub * 2);
  filter1.resonance(0.6);

  waveform1.amplitude(amplitude);
  waveform2.amplitude(amplitude_2);
  waveform_sub.amplitude(amplitude_sub);

  // Configure DC signal for envelope2 (constant voltage source)
  dc_signal.amplitude(defaults::filter_env_gain);

  // Configure envelope1
  envelope1.attack(attack_time);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1.0);
  envelope1.release(release_time);
  envelope1.releaseNoteOn(0);

  // Configure envelope2 with the same ADSR values as envelope1
  envelope2.attack(attack_time);
  envelope2.hold(0);
  envelope2.decay(0);
  envelope2.sustain(1.0);
  envelope2.release(release_time);
  envelope2.releaseNoteOn(0);

  mixer1.gain(0, defaults::main_mix_gain);
  mixer1.gain(1, defaults::main_mix_gain);
  mixer1.gain(2, defaults::sub_mix_gain);
}

void Audio::noteOn(byte note, byte velocity, float detune) {
  current_note = note;
  float sustain = (float)velocity / 127.0;

  // update main oscillator
  freq = computeFrequencyFromNote(current_note, 0);

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, detune);

  AudioNoInterrupts();

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);

  // update sub oscillator
  freq_sub = computeSubFrequency(freq, 2.0);

  waveform_sub.frequency(freq_sub);
  filter1.frequency(freq_sub * 2);

  envelope1.sustain(sustain);
  envelope2.sustain(sustain * 0.8);

  envelope1.noteOn();
  envelope2.noteOn();

  AudioInterrupts();
}

void Audio::noteOff() {
  envelope1.noteOff();
  envelope2.noteOff();
}

void Audio::updateEnvelopeMode(bool percussive_mode) {
  if (percussive_mode) {
    envelope1.decay(release_time);
    envelope1.sustain(0.0);
    envelope1.release(0);
    envelope2.decay(release_time);
    envelope2.sustain(0.0);
    envelope2.release(0);
  } else {
    envelope1.decay(0);
    envelope1.sustain(1.0);
    envelope1.release(release_time);
    envelope2.decay(0);
    envelope2.sustain(1.0);
    envelope2.release(release_time);
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
  amplitude_sub = amplitude * 0.5; // attenuation to avoid clipping in filter
  waveform_sub.amplitude(amplitude_sub);
}

void Audio::updateWaveform(int waveform) {
#ifdef DEBUG
  Serial.println("Osc 1 & 2 waveform:");
  Serial.println(waveform);
#endif

  AudioNoInterrupts();

  waveform1.begin(waveform);
  waveform2.begin(waveform);
  waveform_sub.begin(waveform);

  if (waveform == WAVEFORM_BANDLIMIT_PULSE) {
    waveform1.pulseWidth(0.25);
    waveform2.pulseWidth(0.25);
    waveform_sub.pulseWidth(0.25);

    // Gain correction
    mixer1.gain(0, defaults::main_mix_gain * 2.0f);
    mixer1.gain(1, defaults::main_mix_gain * 2.0f);
    mixer1.gain(2, defaults::sub_mix_gain * 2.0f);
  } else {
    mixer1.gain(0, defaults::main_mix_gain);
    mixer1.gain(1, defaults::main_mix_gain);
    mixer1.gain(2, defaults::sub_mix_gain);
  }

  AudioInterrupts();
}

void Audio::updateAttack(float attack) {
  attack_time = attack * 149 + 1; // 1 to 150ms

  AudioNoInterrupts();

  envelope1.attack(attack_time);
  envelope2.attack(attack_time);

  AudioInterrupts();
}

void Audio::updateRelease(float release) {
  release_time = release * 598 + 2; // 2 to 600ms

  AudioNoInterrupts();

  envelope1.release(release_time);
  envelope2.release(release_time);

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