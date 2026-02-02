#include "synth_waveform.h"

#include "MonoSynthState.h"
#include "core/Synth.h"
#include "core/Hardware.h"
#include "lib/Logger.h"

namespace Autosave {

void MonoSynthState::noteOn(byte note, byte velocity) {
  synth_->audio->noteOn(note, velocity, detune_);
}

void MonoSynthState::noteOff(byte note, byte velocity) {
  synth_->audio->noteOff();
}

void MonoSynthState::process() {
  State::process();

  // Button 1 changes the waveform type of the main oscillators
  if (synth_->hardware->changed(hardware::CTRL_SWITCH_1)) {
    byte waveform = (byte)synth_->hardware->read(hardware::CTRL_SWITCH_1);

    AutosaveLib::Logger::debug("Updating waveform: " + String(waveform));

    switch (waveform) {
    case 0:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE);
      break;
    case 1:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SQUARE);
      break;
    default:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_PULSE);
      break;
    }
  }

  if (synth_->hardware->changed(hardware::CTRL_SWITCH_2)) {
    AutosaveLib::Logger::warn(
        "Not implemented yet! Value: " +
        String(synth_->hardware->read(hardware::CTRL_SWITCH_2)));
  }

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
    detune_ = synth_->hardware->read(hardware::CTRL_POT_1);

    synth_->audio->updateOsc2Frequency(detune_);
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_2)) {
    synth_->audio->updateOsc2Amplitude(
        synth_->hardware->read(hardware::CTRL_POT_2));
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_3)) {
    synth_->audio->updateSubAmplitude(
        synth_->hardware->read(hardware::CTRL_POT_3));
  }
}

} // namespace Autosave