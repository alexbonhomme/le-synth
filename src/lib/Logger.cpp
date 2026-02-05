#include <Arduino.h>

#include "Logger.h"

namespace AutosaveLib {

uint8_t Logger::level_;

void Logger::begin(uint8_t level, long baudRate) {
#ifdef DEBUG
  Serial.begin(baudRate);

  while (!Serial) {
    delay(100);
  }

  Logger::setLevel(level);
  info("Logger initialized");
#endif
}

void Logger::print(const char *message, uint8_t level) {
#ifdef DEBUG
  if (Logger::getLevel() >= level) {
    Serial.print(message);
  }
#endif
}

void Logger::print(int value, uint8_t level) {
  print(String(value).c_str(), level);
}

void Logger::print(float value, uint8_t level) {
  print(String(value).c_str(), level);
}

void Logger::print(double value, uint8_t level) {
  print(String(value).c_str(), level);
}

void Logger::print(bool value, uint8_t level) {
  print(String(value).c_str(), level);
}

void Logger::print(const String &value, uint8_t level) {
  print(value.c_str(), level);
}

void Logger::println(const char *message, uint8_t level) {
#ifdef DEBUG
  if (Logger::getLevel() >= level) {
    Serial.println(message);
  }
#endif
}

void Logger::println(int value, uint8_t level) {
  println(String(value).c_str(), level);
}

void Logger::println(float value, uint8_t level) {
  println(String(value).c_str(), level);
}

void Logger::println(double value, uint8_t level) {
  println(String(value).c_str(), level);
}

void Logger::println(bool value, uint8_t level) {
  println(String(value).c_str(), level);
}

void Logger::println(const String &value, uint8_t level) {
  println(value.c_str(), level);
}

void Logger::error(const char *message) {
  println(String("[ERROR] " + String(message)).c_str(), LEVEL_ERROR);
}

void Logger::error(int value) {
  error(String(value).c_str());
}

void Logger::error(float value) {
  error(String(value).c_str());
}

void Logger::error(double value) {
  error(String(value).c_str());
}

void Logger::error(bool value) {
  error(String(value).c_str());
}

void Logger::error(const String &value) {
  error(value.c_str());
}

void Logger::warn(const char *message) {
  println(String("[WARN] " + String(message)).c_str(), LEVEL_WARN);
}

void Logger::warn(int value) {
  warn(String(value).c_str());
}

void Logger::warn(float value) {
  warn(String(value).c_str());
}

void Logger::warn(double value) {
  warn(String(value).c_str());
}

void Logger::warn(bool value) {
  warn(String(value).c_str());
}

void Logger::warn(const String &value) {
  warn(value.c_str());
}

void Logger::info(const char *message) {
  println(String("[INFO] " + String(message)).c_str(), LEVEL_INFO);
}

void Logger::info(int value) {
  info(String(value).c_str());
}

void Logger::info(float value) {
  info(String(value).c_str());
}

void Logger::info(double value) {
  info(String(value).c_str());
}

void Logger::info(bool value) {
  info(String(value).c_str());
}

void Logger::info(const String &value) {
  info(value.c_str());
}

void Logger::debug(const char *message) {
  println(String("[DEBUG] " + String(message)).c_str(), LEVEL_DEBUG);
}

void Logger::debug(int value) {
  debug(String(value).c_str());
}

void Logger::debug(float value) {
  debug(String(value).c_str());
}

void Logger::debug(double value) {
  debug(String(value).c_str());
}

void Logger::debug(bool value) {
  debug(String(value).c_str());
}

void Logger::debug(const String &value) {
  debug(value.c_str());
}

} // namespace AutosaveLib
