#include "State.h"
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
}

} // namespace Autosave