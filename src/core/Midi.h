#ifndef AUTOSAVE_MIDI_H
#define AUTOSAVE_MIDI_H

#include <MIDI.h>

namespace Autosave {

struct MidiNote {
  uint8_t number;
  uint8_t velocity;
};

class Midi {
public:
  Midi();

  void setHandleNoteOn(void (*callback)(uint8_t channel, uint8_t note,
                                        uint8_t velocity));
  void setHandleNoteOff(void (*callback)(uint8_t channel, uint8_t note,
                                         uint8_t velocity));
  void setHandleClock(void (*callback)(void));
  void setHandleStart(void (*callback)(void));
  void setHandleContinue(void (*callback)(void));
  void setHandleStop(void (*callback)(void));

  void begin();
  void setChannel(uint8_t channel);
  uint8_t getChannel() const { return channel_; }
  void read();

  /** Callbacks for arp steps SysEx get/set; set from ArpSynthState. */
  using ArpStepsGetter = void (*)(uint8_t mode, uint8_t *len, uint8_t *data);
  using ArpStepsSetter = void (*)(uint8_t mode, uint8_t len,
                                  const uint8_t *data);
  void setArpStepsSysexHandlers(ArpStepsGetter getter, ArpStepsSetter setter);

  /** Callbacks for custom waveform SysEx get/set; set from Synth. */
  using CustomWaveformGetter = void (*)(uint8_t *bank, uint8_t *index);
  using CustomWaveformSetter = void (*)(uint8_t bank, uint8_t index);
  void setCustomWaveformSysexHandlers(CustomWaveformGetter getter,
                                      CustomWaveformSetter setter);

private:
  static Midi *instance_;
  uint8_t channel_ = 1;
  ArpStepsGetter arp_steps_getter_ = nullptr;
  ArpStepsSetter arp_steps_setter_ = nullptr;
  CustomWaveformGetter custom_waveform_getter_ = nullptr;
  CustomWaveformSetter custom_waveform_setter_ = nullptr;

  /** Static SysEx handler to register with the MIDI library. */
  static void handleSysEx(uint8_t *array, unsigned size);
  void sendSysEx(const uint8_t *data, unsigned size);
};

} // namespace Autosave

#endif