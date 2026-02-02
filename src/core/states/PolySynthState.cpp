#include "PolySynthState.h"

#include "core/Audio.h"
#include "core/Hardware.h"
#include "core/Synth.h"
#include "lib/Logger.h"

namespace Autosave {

void PolySynthState::begin() {
  note_count_ = 0;

  for (int i = 0; i < audio_config::voices_number; i++) {
    current_notes_[i] = 0;
  }

  AudioNoInterrupts();

  synth_->audio->updateAllOscillatorsAmplitude(1.0f);

  AudioInterrupts();
}

void PolySynthState::noteOn(byte note, byte velocity) {
  if (note_count_ >= audio_config::voices_number) {
    return;
  }

  byte index = 0;

  // Looking for the first empty oscillator
  while (index < audio_config::voices_number) {
    if (current_notes_[index] == 0) {
      break;
    }

    index++;
  }

  current_notes_[index] = note;
  note_count_++;

  float sustain = (float)velocity / 127.0f;
  float freq = Audio::computeFrequencyFromNote(note);

  AudioNoInterrupts();

  synth_->audio->updateOscillatorFrequency(index, freq);
  synth_->audio->noteOn(index, sustain, note_count_ == 1);

  AutosaveLib::Logger::debug("Note on: " + String(note) +
                             " index: " + String(index));
  AutosaveLib::Logger::debug("Note count: " + String(note_count_));

  AudioInterrupts();
}

void PolySynthState::noteOff(byte note, byte velocity) {
  if (note_count_ <= 0) {
    return;
  }

  byte index = 0;

  // Looking for the first note that matches the note
  while (index < audio_config::voices_number) {
    if (current_notes_[index] == note) {
      break;
    }

    index++;
  }

  if (index == audio_config::voices_number) {
    return;
  }

  current_notes_[index] = 0;
  note_count_--;

  AudioNoInterrupts();

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

    // @TODO: implement oscillators spread
  }
}
} // namespace Autosave
