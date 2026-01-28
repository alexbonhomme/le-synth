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

  if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_mode)) {
    unsigned char mode = (unsigned char)synth_->hardware->read(hardware_type::SWITCH, controls::sw_mode);

    switch (mode) {
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
  if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_1)) {
    unsigned char waveform = (unsigned char)synth_->hardware->read(hardware_type::SWITCH, controls::sw_1);

    switch (waveform) {
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

  if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_3)) {
    synth_->audio->updateEnvelopeMode(
        synth_->hardware->read(hardware_type::SWITCH, controls::sw_3) == LOW);
  }

  // Update the frequency of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_1)) {
    synth_->audio->updateOsc2Frequency(
        synth_->hardware->read(hardware_type::POT, controls::pot_1));
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_2)) {
    synth_->audio->updateOsc2Amplitude(
        synth_->hardware->read(hardware_type::POT, controls::pot_2));
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_3)) {
    synth_->audio->updateSubAmplitude(
        synth_->hardware->read(hardware_type::POT, controls::pot_3));
  }

  if (synth_->hardware->changed(hardware_type::POT, controls::pot_attack)) {
    synth_->audio->updateAttack(
        synth_->hardware->read(hardware_type::POT, controls::pot_attack));
  }

  if (synth_->hardware->changed(hardware_type::POT, controls::pot_release)) {
    synth_->audio->updateRelease(
        synth_->hardware->read(hardware_type::POT, controls::pot_release));
  }
}

} // namespace Autosave