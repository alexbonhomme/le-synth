#ifndef AUTOSAVE_EEPROM_STORAGE_H
#define AUTOSAVE_EEPROM_STORAGE_H

#include <array>
#include <vector>
#include <cstdint>

namespace Autosave {

/** EEPROM-backed storage for configuration (MIDI channel, arp steps, etc.). */
class EepromStorage {
public:
  /** Type for the 3 arp mode step sequences (max 8 steps each). */
  using ArpModeSteps = std::array<std::vector<uint8_t>, 3>;

  /** Max steps per arp mode. */
  static constexpr uint8_t kMaxArpSteps = 8;

  /** Default custom waveform (Overtone bank, index 42). Used when EEPROM invalid. */
  static constexpr uint8_t kCustomWaveformBankDefault = 2;  // Overtone
  static constexpr uint8_t kCustomWaveformIndexDefault = 42;

  /**
   * Load MIDI channel from EEPROM (1–16).
   * Returns 1 if magic invalid or value out of range.
   */
  static uint8_t loadMidiChannel();

  /**
   * Save MIDI channel to EEPROM (1–16). No-op if out of range.
   */
  static void saveMidiChannel(uint8_t channel);

  /**
   * Load arp mode steps from EEPROM into out.
   * If magic is invalid, out is left unchanged.
   */
  static void loadArpModeSteps(ArpModeSteps &out);

  /**
   * Save arp mode steps to EEPROM.
   */
  static void saveArpModeSteps(const ArpModeSteps &data);

  /**
   * Load custom waveform bank (0=FM, 1=Granular, 2=Overtone) and index from EEPROM.
   * If magic is invalid, out_bank and out_index are set to defaults.
   */
  static void loadCustomWaveform(uint8_t &out_bank, uint8_t &out_index);

  /**
   * Save custom waveform bank and index to EEPROM.
   */
  static void saveCustomWaveform(uint8_t bank, uint8_t index);
};

} // namespace Autosave

#endif
