#ifndef AUTOSAVE_WAVEFORMS_H
#define AUTOSAVE_WAVEFORMS_H

#include <cstddef>
#include <cstdint>

/**
 * AKWF waveform registry — single header, no per-waveform includes.
 *
 * Waveforms are grouped by type (matching folder names): fm, granular, overtone.
 * Each group is an array of pointers to 256-sample int16_t data. Definitions
 * live in Waveforms.cpp (one TU) so callers only include this header and use
 * AKWF_FM[i], AKWF_GRANULAR[i], or AKWF_OVERTONE[i].
 */

namespace Autosave {

constexpr size_t AKWF_WAVEFORM_LENGTH = 256;

/// FM (fmsynth) — 122 waveforms
extern const int16_t* const AKWF_FM[];
extern const size_t AKWF_FM_COUNT;

/// Granular — 44 waveforms
extern const int16_t* const AKWF_GRANULAR[];
extern const size_t AKWF_GRANULAR_COUNT;

/// Overtone — 44 waveforms
extern const int16_t* const AKWF_OVERTONE[];
extern const size_t AKWF_OVERTONE_COUNT;

} // namespace Autosave

#endif
