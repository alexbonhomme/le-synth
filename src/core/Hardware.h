#ifndef AUTOSAVE_HARDWARE_H
#define AUTOSAVE_HARDWARE_H

#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

namespace Autosave {

namespace defaults {
static constexpr byte bounce_interval = 100;
static constexpr float snap_multiplier = 0.001f;

static constexpr byte sw_1_1_pin = 2;
static constexpr byte sw_1_2_pin = 3;
static constexpr byte sw_2_1_pin = 4;
static constexpr byte sw_2_2_pin = 5;
static constexpr byte sw_3_1_pin = 6;
static constexpr byte sw_3_2_pin = 8;
static constexpr byte sw_4_pin = 9;

static constexpr byte pot_1_pin = A0;
static constexpr byte pot_2_pin = A1;
static constexpr byte pot_3_pin = A2;
static constexpr byte pot_attack_pin = A3;
static constexpr byte pot_release_pin = A4;
static constexpr byte cv_pin = A5;
} // namespace defaults

enum hardware_controls {
  SWITCH_MODE = 0,
  SWITCH_1 = 1,
  SWITCH_2 = 2,
  SWITCH_3 = 3,
  POT_1 = 4,
  POT_2 = 5,
  POT_3 = 6,
  POT_ATTACK = 7,
  POT_RELEASE = 8,
  CV = 9
};

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
  bool changed(hardware_controls control);
  float read(hardware_controls control);
};

} // namespace Autosave

#endif