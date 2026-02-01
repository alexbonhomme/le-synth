#include <Arduino.h>

namespace AutosaveLib {

class Logger {

private:
  static byte level_;

public:
  static constexpr byte LEVEL_ERROR = 0;
  static constexpr byte LEVEL_WARN = 1;
  static constexpr byte LEVEL_INFO = 2;
  static constexpr byte LEVEL_DEBUG = 3;

  static void begin(byte level = LEVEL_INFO,
                    long baudRate = 115200);

  static void setLevel(byte level) { Logger::level_ = level; }
  static byte getLevel() { return Logger::level_; }

  static void print(const char *message, byte level);
  static void print(int value, byte level);
  static void print(float value, byte level);
  static void print(double value, byte level);
  static void print(bool value, byte level);
  static void print(const String &value, byte level);

  static void println(const char *message, byte level);
  static void println(int value, byte level);
  static void println(float value, byte level);
  static void println(double value, byte level);
  static void println(bool value, byte level);
  static void println(const String &value, byte level);

  static void error(const char *message);
  static void error(int value);
  static void error(float value);
  static void error(double value);
  static void error(bool value);
  static void error(const String &value);

  static void warn(const char *message);
  static void warn(int value);
  static void warn(float value);
  static void warn(double value);
  static void warn(bool value);
  static void warn(const String &value);

  static void info(const char *message);
  static void info(int value);
  static void info(float value);
  static void info(double value);
  static void info(bool value);
  static void info(const String &value);

  static void debug(const char *message);
  static void debug(int value);
  static void debug(float value);
  static void debug(double value);
  static void debug(bool value);
  static void debug(const String &value);
};

} // namespace AutosaveLib