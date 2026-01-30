#include "State.h"
#include "Synth.h"
#include "synth_waveform.h"

namespace Autosave {

void State::process() {
  // Attack
  if (synth_->hardware->changed(hardware::CTRL_POT_ATTACK)) {
    synth_->audio->updateAttack(
        synth_->hardware->read(hardware::CTRL_POT_ATTACK));
  }

  // Decay/release
  if (synth_->hardware->changed(hardware::CTRL_POT_RELEASE)) {
    synth_->audio->updateRelease(
        synth_->hardware->read(hardware::CTRL_POT_RELEASE));
  }
}

/**
 * Mono synth state
 */
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
#ifdef DEBUG
    Serial.println("Updating waveform: " +
                   String(synth_->hardware->read(hardware::CTRL_SWITCH_1)));
#endif

    byte waveform = (byte)synth_->hardware->read(hardware::CTRL_SWITCH_1);
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
#ifdef DEBUG
    Serial.println("Not implemented yet! Value: " +
                   String(synth_->hardware->read(hardware::CTRL_SWITCH_2)));
#endif
  }

  // @TODO: Not connected yet (prototype)
  // if (synth_->hardware->changed(hardware::CTRL_SWITCH_3)) {
  //   #ifdef DEBUG
  //   Serial.println("Updating envelope mode: " +
  //   String(synth_->hardware->read(hardware::CTRL_SWITCH_3)));
  //   #endif

  //   synth_->audio->updateEnvelopeMode(
  //       synth_->hardware->read(hardware::CTRL_SWITCH_3) ==
  //       LOW);
  // }

  // Update the frequency of the second oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_1)) {
    // #ifdef DEBUG
    // Serial.println("Updating osc 2 frequency: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_1)));
    // #endif
    detune_ = synth_->hardware->read(hardware::CTRL_POT_1);

    synth_->audio->updateOsc2Frequency(detune_);
  }

  // Update the amplitude of the second oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_2)) {
    // #ifdef DEBUG
    // Serial.println("Updating osc 2 amplitude: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_2)));
    // #endif

    synth_->audio->updateOsc2Amplitude(
        synth_->hardware->read(hardware::CTRL_POT_2));
  }

  // Update the amplitude of the sub oscillator
  if (synth_->hardware->changed(hardware::CTRL_POT_3)) {
    // #ifdef DEBUG
    // Serial.println("Updating sub amplitude: " +
    // String(synth_->hardware->read(hardware_type::POT, controls::pot_3)));
    // #endif

    synth_->audio->updateSubAmplitude(
        synth_->hardware->read(hardware::CTRL_POT_3));
  }
}

} // namespace Autosave