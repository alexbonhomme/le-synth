#ifndef AUTOSAVE_AUDIO_H
#define AUTOSAVE_AUDIO_H

#include <Audio.h>
#include <cmath>

#include "lib/Logger.h"

namespace Autosave {

namespace audio_config {
static constexpr uint8_t voices_number = 8;

static constexpr float init_lfo_frequency = 20.0f;
static constexpr float init_lfo_amplitude = 0.0f;

static constexpr float init_frequency = 440.0f; // A4
static constexpr short init_waveform = WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
static constexpr float init_amplitude = 0.0f;

static constexpr float osc_mix_gain = 0.25f;
static constexpr float master_gain = 0.48f;

static constexpr float filter_env_gain = 0.5f;

// MIDI note (0–127) to frequency (Hz). A4 = 440 Hz at index 69. 8 per line =
// exact indices.
static constexpr float midi_note_to_frequency[128] = {
    8.175799f,     8.661957f,     9.177024f,    9.722718f,    10.300861f,
    10.913382f,    11.562326f,    12.249857f,   12.978272f,   13.750000f,
    14.567618f,    15.433853f,    16.351598f,   17.323914f,   18.354048f,
    19.445436f,    20.601722f,    21.826764f,   23.124651f,   24.499715f,
    25.956544f,    27.500000f,    29.135235f,   30.867706f,   32.703196f,
    34.647829f,    36.708096f,    38.890873f,   41.203445f,   43.653529f,
    46.249303f,    48.999429f,    51.913087f,   55.000000f,   58.270470f,
    61.735413f,    65.406391f,    69.295658f,   73.416192f,   77.781746f,
    82.406889f,    87.307058f,    92.498606f,   97.998859f,   103.826174f,
    110.000000f,   116.540940f,   123.470825f,  130.812783f,  138.591315f,
    146.832384f,   155.563492f,   164.813778f,  174.614116f,  184.997211f,
    195.997718f,   207.652349f,   220.000000f,  233.081881f,  246.941651f,
    261.625565f,   277.182631f,   293.664768f,  311.126984f,  329.627557f,
    349.228231f,   369.994423f,   391.995436f,  415.304698f,  440.000000f,
    466.163762f,   493.883301f,   523.251131f,  554.365262f,  587.329536f,
    622.253967f,   659.255114f,   698.456463f,  739.988845f,  783.990872f,
    830.609395f,   880.000000f,   932.327523f,  987.766603f,  1046.502261f,
    1108.730524f,  1174.659072f,  1244.507935f, 1318.510228f, 1396.912926f,
    1479.977691f,  1567.981744f,  1661.218790f, 1760.000000f, 1864.655046f,
    1975.533205f,  2093.004522f,  2217.461048f, 2349.318143f, 2489.015870f,
    2637.020455f,  2793.825851f,  2959.955382f, 3135.963488f, 3322.437581f,
    3520.000000f,  3729.310092f,  3951.066410f, 4186.009045f, 4434.922096f,
    4698.636287f,  4978.031740f,  5274.040911f, 5587.651703f, 5919.910763f,
    6271.926976f,  6644.875161f,  7040.000000f, 7458.620184f, 7902.132820f,
    8372.018090f,  8869.844191f,  9397.272573f, 9956.063479f, 10548.081821f,
    11175.303406f, 11839.821527f, 12543.853951f};
} // namespace audio_config

class Audio {
public:
  Audio();

  void begin();

  void noteOn(uint8_t index, float sustain, bool triggerFilterEnvelope = false);
  void noteOff(uint8_t index, bool triggerFilterEnvelope = false);
  void noteOffAll();

  void updateEnvelopeMode(bool percussive_mode);

  void updateLFOFrequency(float frequency);
  void updateLFOAmplitude(float amplitude);

  void updateOscillatorFrequency(uint8_t index, float frequency);
  void updateAllOscillatorsFrequency(float frequency);
  void updateOscillatorAmplitude(uint8_t index, float amplitude);
  void updateAllOscillatorsAmplitude(float amplitude);
  void updateOscillatorWaveform(uint8_t index, uint8_t waveform);
  void updateAllOscillatorsWaveform(uint8_t waveform);

  void updateAttack(float attack);
  void updateRelease(float release);

  void normalizeMasterGain(uint8_t oscillators_count) {
    // @TODO: use table instead of log2f to improve performance
    float gain_correction = 3.0f / std::log2f(oscillators_count + 10.0f);
    float normalized_gain = audio_config::master_gain * gain_correction;

    AutosaveLib::Logger::debug("Normalized gain: " + String(normalized_gain));

    amplifier_master.gain(normalized_gain);
  }

  static float computeFrequencyFromNote(uint8_t note);
  static float computeFrequencyFromCV(float cv);

private:
  AudioSynthWaveformSine lfo;
  AudioSynthWaveformModulated oscillators[audio_config::voices_number];
  AudioEffectEnvelope envelopes[audio_config::voices_number];
  AudioMixer4 mixers[audio_config::voices_number / 4];
  AudioMixer4 mixer_master;
  AudioAmplifier amplifier_master;
  AudioSynthWaveformDc dc_signal;
  AudioEffectEnvelope filter_envelope;
  AudioOutputI2S i2s1;
  AudioConnection patchCords[30];

  bool percussive_mode_ = false;
  float attack_time = 1.0f;
  float release_time = 15.0f;

  float computeGainFromWaveform(uint8_t waveform);
};

} // namespace Autosave

#endif