#ifndef AUTOSAVE_HARDWARE_H
#define AUTOSAVE_HARDWARE_H

#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

namespace Autosave {

namespace defaults {
static constexpr byte bounce_interval = 100;
static constexpr float snap_multiplier = 0.001f;
} // namespace defaults

namespace hardware {
enum pins {
  PIN_SW_1_1 = 2,
  PIN_SW_1_3 = 3,
  PIN_SW_2_1 = 4,
  PIN_SW_2_3 = 5,
  PIN_SW_3_1 = 6,
  PIN_SW_3_3 = 8,
  PIN_SW_4_1 = 9,
  PIN_POT_1 = A0,
  PIN_POT_2 = A1,
  PIN_POT_3 = A2,
  PIN_POT_ATTACK = A3,
  PIN_POT_RELEASE = A4,
  PIN_CV = A5,
};

enum controls {
  CTRL_SWITCH_MODE = 0,
  CTRL_SWITCH_1 = 1,
  CTRL_SWITCH_2 = 2,
  CTRL_SWITCH_3 = 3,
  CTRL_POT_1 = 4,
  CTRL_POT_2 = 5,
  CTRL_POT_3 = 6,
  CTRL_POT_ATTACK = 7,
  CTRL_POT_RELEASE = 8,
  CTRL_CV = 9
};
} // namespace hardware

class HardwareControl {
public:
  virtual ~HardwareControl() {}

  virtual void begin(byte pin) = 0;
  virtual void update() = 0;
  virtual bool changed() = 0;
  virtual float read() = 0;
};

class SwitchControl : public HardwareControl {
private:
  Bounce bounce[2];
  bool has_two_pins = false;

public:
  void begin(byte pin) {
    bounce[0].attach(pin, INPUT_PULLUP);
    bounce[0].interval(defaults::bounce_interval);
  }

  void begin(byte pin_1, byte pin_2) {
    has_two_pins = true;

    bounce[0].attach(pin_1, INPUT_PULLUP);
    bounce[0].interval(defaults::bounce_interval);
    bounce[1].attach(pin_2, INPUT_PULLUP);
    bounce[1].interval(defaults::bounce_interval);
  }

  void update() {
    bounce[0].update();

    if (has_two_pins) {
      bounce[1].update();
    }
  }

  bool changed() {
    return bounce[0].changed() || (has_two_pins && bounce[1].changed());
  }

  float read() {
    if (has_two_pins) {
      if (bounce[0].read() == LOW) {
        return 2.0f;
      }

      if (bounce[1].read() == LOW) {
        return 0.0f;
      }

      return 1.0f;
    }

    return bounce[0].read() == LOW ? 0.0f : 1.0f;
  }
};

class AnalogControl : public HardwareControl {
private:
  ResponsiveAnalogRead pot;

public:
  void begin(byte pin) { pot.begin(pin, true, defaults::snap_multiplier); }

  void update() { pot.update(); }

  bool changed() { return pot.hasChanged(); }

  float read() { return pot.getValue() / 1023.0f; }
};

class Hardware {
private:
  HardwareControl *controls[10];

public:
  Hardware();

  void begin();
  void update();
  bool changed(hardware::controls control);
  float read(hardware::controls control);
};

} // namespace Autosave

#endif