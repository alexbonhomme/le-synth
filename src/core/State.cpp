#include "State.h"
#include "Synth.h"

namespace Autosave {

void MonoSynthState::noteOn(byte note, byte velocity) {
  synth_->audio->noteOn(
      note, velocity,
      synth_->hardware->read(hardware_type::POT, controls::pot_1));
}

void MonoSynthState::noteOff(byte note, byte velocity) {
  synth_->audio->noteOff();
}

void MonoSynthState::process() {

  if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_mode)) {
#ifdef DEBUG
    Serial.println("Updating mode: " +
                   String(synth_->hardware->read(hardware_type::SWITCH,
                                                 controls::sw_mode)));
#endif

    unsigned char mode = (unsigned char)synth_->hardware->read(
        hardware_type::SWITCH, controls::sw_mode);

    switch (mode) {
    case 0:
#ifdef DEBUG
      Serial.println("Changing to mono synth state");
#endif
      synth_->changeState(new MonoSynthState());
      break;
    case 1:
// @TODO: Implement polyphonic synth state
#ifdef DEBUG
      Serial.println("Polyphonic synth state not implemented");
#endif
      break;
    case 2:
// @TODO: Implement arpeggiator synth state
#ifdef DEBUG
      Serial.println("Arpeggiator synth state not implemented");
#endif
      break;
    default:
      break;
    }
  }

  // Button 1 changes the waveform type of the main oscillators
  if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_1)) {
#ifdef DEBUG
    Serial.println(
        "Updating waveform: " +
        String(synth_->hardware->read(hardware_type::SWITCH, controls::sw_1)));
#endif

    unsigned char waveform = (unsigned char)synth_->hardware->read(
        hardware_type::SWITCH, controls::sw_1);

    switch (waveform) {
    case 0:
      synth_->audio->updateWaveform(WAVEFORM_TRIANGLE);
      break;
    case 1:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SQUARE);
      break;
    default:
      synth_->audio->updateWaveform(WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE);
      break;
    }
  }

  // @TODO: Not connected yet (prototype)
  // if (synth_->hardware->changed(hardware_type::SWITCH, controls::sw_3)) {
  //   #ifdef DEBUG
  //   Serial.println("Updating envelope mode: " +
  //   String(synth_->hardware->read(hardware_type::SWITCH, controls::sw_3)));
  //   #endif

  //   synth_->audio->updateEnvelopeMode(
  //       synth_->hardware->read(hardware_type::SWITCH, controls::sw_3) ==
  //       LOW);
  // }

  // Update the frequency of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_1)) {
    // #ifdef DEBUG
    // Serial.println("Updating osc 2 frequency: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_1)));
    // #endif

    synth_->audio->updateOsc2Frequency(
        synth_->hardware->read(hardware_type::POT, controls::pot_1));
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_2)) {
    // #ifdef DEBUG
    // Serial.println("Updating osc 2 amplitude: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_2)));
    // #endif

    synth_->audio->updateOsc2Amplitude(
        synth_->hardware->read(hardware_type::POT, controls::pot_2));
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware_type::POT, controls::pot_3)) {
    // #ifdef DEBUG
    // Serial.println("Updating sub amplitude: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_3)));
    // #endif

    synth_->audio->updateSubAmplitude(
        synth_->hardware->read(hardware_type::POT, controls::pot_3));
  }

  if (synth_->hardware->changed(hardware_type::POT, controls::pot_attack)) {
    // #ifdef DEBUG
    // Serial.println("Updating attack: " +
    // String(synth_->hardware->read(hardware_type::POT,
    // controls::pot_attack))); #endif

    synth_->audio->updateAttack(
        synth_->hardware->read(hardware_type::POT, controls::pot_attack));
  }

  if (synth_->hardware->changed(hardware_type::POT, controls::pot_release)) {
    // #ifdef DEBUG
    // Serial.println("Updating release: " +
    // String(synth_->hardware->read(hardware_type::POT,
    // controls::pot_release))); #endif

    synth_->audio->updateRelease(
        synth_->hardware->read(hardware_type::POT, controls::pot_release));
  }
}

} // namespace Autosave