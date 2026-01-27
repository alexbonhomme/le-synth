#include "State.h"
#include "Synth.h"

namespace Autosave {

void MonoSynthState::noteOn(byte note, byte velocity) {
  synth_->audio->noteOn(note, velocity, (float)synth_->hardware->read(hardware_type::POT, 1) / 1023.0);
}

void MonoSynthState::noteOff(byte note, byte velocity) {
  synth_->audio->noteOff();
}

void MonoSynthState::process() {
  synth_->hardware->update();

  if (synth_->hardware->changed(hardware_type::SWITCH, 0)) {
    switch (synth_->hardware->read(hardware_type::SWITCH, 0)) {
    case 0:
      synth_->changeState(new MonoSynthState());
      break;
    case 1:
      // TODO: Implement polyphonic synth state
      break;
    case 2:
      // TODO: Implement arpeggiator synth state
      break;
    default:
      break;
    }
  }

  // Button 1 changes the waveform type of the main oscillators
  if (synth_->hardware->changed(hardware_type::SWITCH, 1)) {
    switch (synth_->hardware->read(hardware_type::SWITCH, 1)) {
    case 0:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE);
      break;
    case 1:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SQUARE);
      break;
    default:
      synth_->audio->updateWaveform(WAVEFORM_TRIANGLE);
      break;
    }
  }

  if (synth_->hardware->changed(hardware_type::SWITCH, 3)) {
    synth_->audio->updateEnvelopeMode(
        synth_->hardware->read(hardware_type::SWITCH, 3) == LOW);
  }

  // Update the frequency of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, 1)) {
    synth_->audio->updateOsc2Frequency(
        (float)synth_->hardware->read(hardware_type::POT, 1) / 1023.0);
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, 2)) {
    synth_->audio->updateOsc2Amplitude(
        (float)synth_->hardware->read(hardware_type::POT, 2) / 1023.0);
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware_type::POT, 3)) {
    synth_->audio->updateSubAmplitude(
        (float)synth_->hardware->read(hardware_type::POT, 3) / 1023.0);
  }

  if (synth_->hardware->changed(hardware_type::POT, 3)) {
    synth_->audio->updateAttack(
        (float)synth_->hardware->read(hardware_type::POT, 3) / 1023.0);
  }

  if (synth_->hardware->changed(hardware_type::POT, 4)) {
    synth_->audio->updateRelease(
        (float)synth_->hardware->read(hardware_type::POT, 4) / 1023.0);
  }
}

} // namespace Autosave