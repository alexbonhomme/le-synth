#include "State.h"
#include "synth_waveform.h"
#include "core/Synth.h"
#include "lib/Logger.h"

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

  // Switch 1 changes the waveform type of the main oscillators
  if (synth_->hardware->changed(hardware::CTRL_SWITCH_1)) {
    byte waveform = (byte)synth_->hardware->read(hardware::CTRL_SWITCH_1);

    AutosaveLib::Logger::debug("Updating waveform: " + String(waveform));

    switch (waveform) {
    case 0:
      synth_->audio->updateAllOscillatorsWaveform(WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE);
      break;
    case 1:
      synth_->audio->updateAllOscillatorsWaveform(WAVEFORM_BANDLIMIT_SQUARE);
      break;
    default:
      synth_->audio->updateAllOscillatorsWaveform(WAVEFORM_BANDLIMIT_PULSE);
      break;
    }
  }
}

} // namespace Autosave