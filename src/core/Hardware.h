#ifndef AUTOSAVE_HARDWARE_H
#define AUTOSAVE_HARDWARE_H

#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

namespace Autosave {

namespace defaults {
static constexpr unsigned char bounce_interval = 100;

static constexpr unsigned char sw_1_1_pin = 2;
static constexpr unsigned char sw_1_2_pin = 3;
static constexpr unsigned char sw_2_1_pin = 4;
static constexpr unsigned char sw_2_2_pin = 5;
static constexpr unsigned char sw_3_1_pin = 6;
static constexpr unsigned char sw_3_2_pin = 8;
static constexpr unsigned char sw_4_pin = 9;

static constexpr unsigned char pot_1_pin = A0;
static constexpr unsigned char pot_2_pin = A1;
static constexpr unsigned char pot_3_pin = A2;
static constexpr unsigned char pot_attack_pin = A3;
static constexpr unsigned char pot_release_pin = A4;
} // namespace defaults

namespace controls {
static constexpr unsigned char sw_mode = 0;
static constexpr unsigned char sw_1 = 1;
static constexpr unsigned char sw_2 = 2;
static constexpr unsigned char sw_3 = 3;

static constexpr unsigned char pot_1 = 0;
static constexpr unsigned char pot_2 = 1;
static constexpr unsigned char pot_3 = 2;
static constexpr unsigned char pot_attack = 3;
static constexpr unsigned char pot_release = 4;
} // namespace controls

enum hardware_type { SWITCH, POT, CV };

class Switch {
private:
  Bounce bounce[2];
  bool has_two_pins = false;

  unsigned char computeSwitchState(bool state_1, bool state_2) {
    if (state_2 == LOW) {
      return 0;
    }

    if (state_1 == LOW) {
      return 2;
    }

    return 1;
  }

public:
  void begin(unsigned char pin) {
    bounce[0].attach(pin, INPUT_PULLUP);
    bounce[0].interval(defaults::bounce_interval);
  }

  void begin(unsigned char pin_1, unsigned char pin_2) {
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

  unsigned char read() {
    if (has_two_pins) {
      return computeSwitchState(bounce[0].read(), bounce[1].read());
    }

    return bounce[0].read() ? 1 : 0;
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
  bool changed(hardware_type type, unsigned char index);
  float read(hardware_type type, unsigned char index);
};

} // namespace Autosave

#endif