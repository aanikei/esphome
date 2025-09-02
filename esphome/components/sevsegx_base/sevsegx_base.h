#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/core/gpio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sevsegx_base {

class SEVSEGX;

using sevsegx_writer_t = std::function<void(SEVSEGX &)>;

class SEVSEGX : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void set_writer(sevsegx_writer_t &&writer);
  virtual void enable_display() {}
  virtual void disable_display() {}

  void set_segment_pins(std::vector<std::vector<GPIOPin *>> segment_pins);
  void set_digit_pins(std::vector<GPIOPin *> digit_pins);
  void set_digit_brightness(std::vector<uint8_t> digit_pins);
  void set_display_brightness(uint8_t display_brightness);
  void set_digit_dots(std::vector<uint8_t> dots);
  bool get_display_state();
  std::vector<uint8_t> get_digit_dots();
  std::vector<uint8_t> get_digit_brightness();

  virtual void print(const char *str) = 0;
  void print(std::string str);
  void strftime(const char *format, ESPTime time) __attribute__((format(strftime, 2, 0)));

 protected:
  bool setup_complete_{false};

  optional<sevsegx_writer_t> writer_{};

  static const size_t MAX_SEGMENTS = 8;
  static const size_t MAX_SECTIONS = 2;
  static std::array<std::array<GPIOPin *, MAX_SEGMENTS>, MAX_SECTIONS> segment_pins_;
  static std::array<std::array<uint8_t, MAX_SEGMENTS>, MAX_SECTIONS> segment_pin_nums_;

  static std::vector<GPIOPin *> digit_pins_;
  static std::vector<uint8_t> digit_pin_nums_;
  static std::vector<uint8_t> digit_brightness_;
  static std::vector<uint8_t> digit_dots_;
  static uint8_t display_brightness_;
  static uint8_t *buffer_;
  static uint8_t buffer_size_;
  static bool is_enabled;

  static const uint8_t SEVSEG_ASCII_TO_RAW[128];
};

}  // namespace sevsegx_base
}  // namespace esphome
