#include "MonoSynthState.h"

#include "core/Audio.h"
#include "core/Hardware.h"
#include "core/Synth.h"
#include "lib/Logger.h"

namespace Autosave {

void MonoSynthState::begin() {
  State::begin();

  AutosaveLib::Logger::debug("MonoSynthState::begin");

  AudioNoInterrupts();

  // Setup oscillators
  synth_->audio->updateOscillatorAmplitude(0, 1.0f);
  synth_->audio->updateOscillatorAmplitude(
      1, synth_->hardware->read(hardware::CTRL_POT_2));
  synth_->audio->updateOscillatorAmplitude(
      2, synth_->hardware->read(hardware::CTRL_POT_3));

  synth_->audio->normalizeMasterGain(3);

  AudioInterrupts();
}

void MonoSynthState::noteOn(MidiNote note) {
  current_note_ = note;
  float sustain = (float)note.velocity / 127.0f;

  float freq = Audio::computeFrequencyFromNote(note.number);
  float freq_2 = freq * detune_;
  float freq_sub = freq / 2.0f;

  AudioNoInterrupts();

  synth_->audio->updateOscillatorFrequency(0, freq);
  synth_->audio->updateOscillatorFrequency(1, freq_2);
  synth_->audio->updateOscillatorFrequency(2, freq_sub);

  synth_->audio->noteOn(0, sustain, true);
  synth_->audio->noteOn(1, sustain);
  synth_->audio->noteOn(2, sustain);

  AudioInterrupts();
}

void MonoSynthState::noteOff(MidiNote note) {
  AudioNoInterrupts();

  synth_->audio->noteOff(0, true);
  synth_->audio->noteOff(1);
  synth_->audio->noteOff(2);

  AudioInterrupts();
}

void MonoSynthState::process() {
  State::process();

  // if (synth_->hardware->changed(hardware::CTRL_SWITCH_2)) {
  //   AutosaveLib::Logger::warn(
  //       "Not implemented yet! Value: " +
  //       String(synth_->hardware->read(hardware::CTRL_SWITCH_2)));
  // }

  // @TODO: Not connected yet (prototype)
  // if (synth_->hardware->changed(hardware::CTRL_SWITCH_3)) {
  //   AutosaveLib::Logger::debug("Updating envelope mode: " +
  //   String(synth_->hardware->read(hardware::CTRL_SWITCH_3)));

  //   synth_->audio->updateEnvelopeMode(
  //       synth_->hardware->read(hardware::CTRL_SWITCH_3) ==
  //       LOW);
  // }

  // Update the frequency of the second oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_1)) {
    float pot_value = synth_->hardware->read(hardware::CTRL_POT_1);

    /*
     * Convert [0 - 1] value to [0.5 - 1][1 - 2] for detune
     *
     * 0.5 - 1: 1 octave down
     * 1 - 2: 1 octave up
     */
    detune_ = pot_value + 0.5f;
    if (detune_ > 1.0f) {
      detune_ = detune_ * 1.3333333333333333f;
    }

    float frequency = Audio::computeFrequencyFromNote(current_note_.number) * detune_;

    synth_->audio->updateOscillatorFrequency(1, frequency);
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_2)) {
    synth_->audio->updateOscillatorAmplitude(
        1, synth_->hardware->read(hardware::CTRL_POT_2));
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_3)) {
    synth_->audio->updateOscillatorAmplitude(
        2, synth_->hardware->read(hardware::CTRL_POT_3));
  }
}

} // namespace Autosave