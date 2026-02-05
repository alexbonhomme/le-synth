#include "ArpSynthState.h"

#include "core/Hardware.h"
#include "core/Synth.h"
#include "lib/Logger.h"

namespace Autosave {

ArpSynthState *ArpSynthState::instance_ = nullptr;

ArpSynthState::~ArpSynthState() {
  if (synth_ != nullptr && synth_->midi != nullptr) {
    synth_->midi->setHandleClock(nullptr);
    synth_->midi->setHandleStart(nullptr);
    synth_->midi->setHandleContinue(nullptr);
    synth_->midi->setHandleStop(nullptr);
  }

  if (instance_ == this) {
    instance_ = nullptr;
  }
}

void ArpSynthState::begin() {
  AutosaveLib::Logger::debug("ArpSynthState::begin");

  MonoSynthState::begin();

  clock_tick_count_ = 0;
  instance_ = this;
  synth_->midi->setHandleClock(&ArpSynthState::onMidiClock);
  synth_->midi->setHandleStart(&ArpSynthState::onMidiStart);
  synth_->midi->setHandleContinue(&ArpSynthState::onMidiStart);
  synth_->midi->setHandleStop(&ArpSynthState::onMidiStop);
}

void ArpSynthState::onMidiClock() {
  if (instance_ != nullptr) {
    instance_->onClockTick();
  }
}

void ArpSynthState::onMidiStart() {
  if (instance_ != nullptr) {
    instance_->onStart();
  }
}

void ArpSynthState::onMidiStop() {
  if (instance_ != nullptr) {
    instance_->onStop();
  }
}

void ArpSynthState::onClockTick() {
  clock_tick_count_++;

  if (!is_running_) {
    return;
  }

  if (clock_tick_count_ == arp_synth_config::clock_ticks_per_sixteenth - 2) {
    if (current_note_ == 0) {
      return;
    }

    AutosaveLib::Logger::debug("ArpSynthState: note off " +
                               String(current_note_));

    MonoSynthState::noteOff(current_note_, 0);
    current_note_ = 0;
  }

  if (clock_tick_count_ >= arp_synth_config::clock_ticks_per_sixteenth) {
    clock_tick_count_ = 0;

    if (notes_.empty()) {
      return;
    }

    AutosaveLib::Logger::debug("ArpSynthState: note on " + String(note_index_) + ": " +
                               String(notes_[note_index_]));

    current_note_ = notes_[note_index_];
    note_index_ = (note_index_ + 1) % notes_.size();

    // @TODO: Handle velocity
    MonoSynthState::noteOn(current_note_, 127);
  }
}

void ArpSynthState::onStart() {
  AutosaveLib::Logger::debug("ArpSynthState: MIDI start");
  is_running_ = true;
}

void ArpSynthState::onStop() {
  AutosaveLib::Logger::debug("ArpSynthState: MIDI stop");
  is_running_ = false;
  current_note_ = 0;
  note_index_ = 0;
  notes_.clear();

  MonoSynthState::noteOff(0, 0);
}

void ArpSynthState::process() { MonoSynthState::process(); }

void ArpSynthState::noteOn(uint8_t note, uint8_t velocity) {
  if (notes_.size() >= arp_synth_config::max_arp_notes) {
    return;
  }

  notes_.push_back(note);
}

void ArpSynthState::noteOff(uint8_t note, uint8_t velocity) {
  if (notes_.empty()) {
    return;
  }

  for (uint8_t i = 0; i < notes_.size(); i++) {
    if (notes_[i] == note) {
      notes_.erase(notes_.begin() + i);

      AutosaveLib::Logger::debug("ArpSynthState: note off " + String(note) + " at index " + String(i));
      if (note_index_ > 0 && note_index_ >= i) {
        note_index_--;
      }

      break;
    }
  }
}
} // namespace Autosave