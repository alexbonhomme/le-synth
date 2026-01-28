#ifndef AUTOSAVE_HARDWARE_H
#define AUTOSAVE_HARDWARE_H

#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

namespace Autosave {

namespace defaults {
static constexpr byte bounce_interval = 100;

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
} // namespace defaults

namespace controls {
static constexpr byte sw_mode = 0;
static constexpr byte sw_1 = 1;
static constexpr byte sw_2 = 2;
static constexpr byte sw_3 = 3;

static constexpr byte pot_1 = 0;
static constexpr byte pot_2 = 1;
static constexpr byte pot_3 = 2;
static constexpr byte pot_attack = 3;
static constexpr byte pot_release = 4;
} // namespace controls

enum hardware_type { SWITCH, POT, CV };

class Switch {
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

  byte read() {
    if (has_two_pins) {
      if (bounce[0].read() == LOW) {
        return 2;
      }

      if (bounce[1].read() == LOW) {
        return 0;
      }

      return 1;
    }

    return bounce[0].read() == LOW ? 0 : 1;
  }
};

class Hardware {
private:
  Switch switches[4];
  ResponsiveAnalogRead pots[5];
  ResponsiveAnalogRead cv; // CV input for modulation

public:
  Hardware();

  void begin();
  void update();
  bool changed(hardware_type type, byte index);
  float read(hardware_type type, byte index);
};

} // namespace Autosave

#endif