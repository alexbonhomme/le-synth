#include <synth_waveform.h>

#include "State.h"
#include "core/Synth.h"

using namespace AutosaveLib;
namespace Autosave {

void State::begin() {
  AudioNoInterrupts();

  synth_->audio->noteOffAll();
  synth_->audio->updateAllOscillatorsAmplitude(0.0f);
  synth_->audio->updateLFOAmplitude(0.0f);

  loadWaveform((WaveformType)synth_->hardware->read(hardware::CTRL_SWITCH_1));

  AudioInterrupts();
}

void State::process() {
  // Attack
  if (synth_->hardware->changed(hardware::CTRL_POT_ATTACK)) {
    AudioNoInterrupts();
    synth_->audio->updateAttack(
        synth_->hardware->read(hardware::CTRL_POT_ATTACK));
    AudioInterrupts();
  }

  // Decay/release
  if (synth_->hardware->changed(hardware::CTRL_POT_RELEASE)) {
    AudioNoInterrupts();
    synth_->audio->updateRelease(
        synth_->hardware->read(hardware::CTRL_POT_RELEASE));
    AudioInterrupts();
  }

  // Switch 1 changes the waveform type of the main oscillators
  if (synth_->hardware->changed(hardware::CTRL_SWITCH_1)) {
    loadWaveform((WaveformType)synth_->hardware->read(hardware::CTRL_SWITCH_1));
  }
}

void State::loadWaveform(WaveformType waveform_type) {
  AudioNoInterrupts();

  switch (waveform_type) {
  case WaveformType::SYNTH_WAVEFORM_SAWTOOTH:
    synth_->audio->updateAllOscillatorsWaveform(
        WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE);
    break;
  case WaveformType::SYNTH_WAVEFORM_SQUARE:
    synth_->audio->updateAllOscillatorsWaveform(WAVEFORM_BANDLIMIT_SQUARE);
    break;
  case WaveformType::SYNTH_WAVEFORM_CUSTOM:
    synth_->audio->updateAllOscillatorsWaveform(WAVEFORM_ARBITRARY);
    synth_->audio->applyCustomWaveform();
    break;
  default:
    Logger::error("Unknown waveform type: " + String(waveform_type));
  }

  AudioInterrupts();
}

} // namespace Autosave