#include "PolySynthState.h"

#include "core/Audio.h"
#include "core/Hardware.h"
#include "core/Synth.h"
#include "lib/Logger.h"

namespace Autosave {

void PolySynthState::begin() {
  AutosaveLib::Logger::debug("PolySynthState::begin");

  AudioNoInterrupts();

  synth_->audio->noteOffAll();
  synth_->audio->updateAllOscillatorsAmplitude(0.0f);
  synth_->audio->updateLFOAmplitude(0.0f);

  AudioInterrupts();
}

void PolySynthState::noteOn(MidiNote note) {
  if (note_count_ >= audio_config::voices_number) {
    return;
  }

  byte index = 0;

  // Looking for the first empty oscillator
  while (index < audio_config::voices_number) {
    if (current_notes_[index].number == 0) {
      break;
    }

    index++;
  }

  current_notes_[index] = note;
  note_count_++;

  float sustain = (float)note.velocity / 127.0f;
  float freq = Audio::computeFrequencyFromNote(note.number);

  AudioNoInterrupts();

  synth_->audio->normalizeMasterGain(note_count_);
  synth_->audio->updateOscillatorFrequency(index, freq);
  synth_->audio->updateOscillatorAmplitude(index, 1.0f);

  synth_->audio->noteOn(index, sustain, note_count_ == 1);

  AudioInterrupts();
}

void PolySynthState::noteOff(MidiNote note) {
  if (note_count_ <= 0 || note_count_ >= audio_config::voices_number) {
    return;
  }

  byte index = 0;

  // Looking for the first note that matches the note
  while (index < audio_config::voices_number) {
    if (current_notes_[index].number == note.number) {
      break;
    }

    index++;
  }

  // Note not found
  if (index >= audio_config::voices_number) {
    return;
  }

  current_notes_[index] = {0, 0};
  note_count_--;

  AudioNoInterrupts();

  synth_->audio->normalizeMasterGain(note_count_);
  synth_->audio->updateOscillatorAmplitude(index, 0.0f);

  synth_->audio->noteOff(index, note_count_ <= 0);

  AudioInterrupts();
}

void PolySynthState::process() {
  State::process();

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

    // @TODO: implement oscillators spread ?
  }

  // Update the frequency of the LFO
  if (synth_->hardware->changed(hardware::CTRL_POT_2)) {
    float pot_value =
        synth_->hardware->read(hardware::CTRL_POT_2) * 4000.0f + 200.0f;

    synth_->audio->updateLFOFrequency(pot_value);
  }

  // Update the amplitude of the LFO
  if (synth_->hardware->changed(hardware::CTRL_POT_3)) {
    synth_->audio->updateLFOAmplitude(
        synth_->hardware->read(hardware::CTRL_POT_3));
  }
}
} // namespace Autosave
