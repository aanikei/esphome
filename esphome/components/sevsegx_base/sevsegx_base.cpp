#include "sevsegx_base.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace sevsegx_base {

static const char *const TAG = "sevsegx";

static const uint8_t SEVENSEG_UNKNOWN_CHAR = 0b00000000;

const uint8_t SEVSEGX::SEVSEG_ASCII_TO_RAW[128] = {
    SEVENSEG_UNKNOWN_CHAR,  // 0x00
    SEVENSEG_UNKNOWN_CHAR,  // 0x01
    SEVENSEG_UNKNOWN_CHAR,  // 0x02
    SEVENSEG_UNKNOWN_CHAR,  // 0x03
    SEVENSEG_UNKNOWN_CHAR,  // 0x04
    SEVENSEG_UNKNOWN_CHAR,  // 0x05
    SEVENSEG_UNKNOWN_CHAR,  // 0x06
    SEVENSEG_UNKNOWN_CHAR,  // 0x07
    SEVENSEG_UNKNOWN_CHAR,  // 0x08
    SEVENSEG_UNKNOWN_CHAR,  // 0x09
    SEVENSEG_UNKNOWN_CHAR,  // 0x0A
    SEVENSEG_UNKNOWN_CHAR,  // 0x0B
    SEVENSEG_UNKNOWN_CHAR,  // 0x0C
    SEVENSEG_UNKNOWN_CHAR,  // 0x0D
    SEVENSEG_UNKNOWN_CHAR,  // 0x0E
    SEVENSEG_UNKNOWN_CHAR,  // 0x0F
    SEVENSEG_UNKNOWN_CHAR,  // 0x10
    SEVENSEG_UNKNOWN_CHAR,  // 0x11
    SEVENSEG_UNKNOWN_CHAR,  // 0x12
    SEVENSEG_UNKNOWN_CHAR,  // 0x13
    SEVENSEG_UNKNOWN_CHAR,  // 0x14
    SEVENSEG_UNKNOWN_CHAR,  // 0x15
    SEVENSEG_UNKNOWN_CHAR,  // 0x16
    SEVENSEG_UNKNOWN_CHAR,  // 0x17
    SEVENSEG_UNKNOWN_CHAR,  // 0x18
    SEVENSEG_UNKNOWN_CHAR,  // 0x19
    SEVENSEG_UNKNOWN_CHAR,  // 0x1A
    SEVENSEG_UNKNOWN_CHAR,  // 0x1B
    SEVENSEG_UNKNOWN_CHAR,  // 0x1C
    SEVENSEG_UNKNOWN_CHAR,  // 0x1D
    SEVENSEG_UNKNOWN_CHAR,  // 0x1E
    SEVENSEG_UNKNOWN_CHAR,  // 0x1F
    0b00000000,             // ' ', ord 0x20
    0b10110000,             // '!', ord 0x21
    0b00100010,             // '"', ord 0x22
    SEVENSEG_UNKNOWN_CHAR,  // '#', ord 0x23
    SEVENSEG_UNKNOWN_CHAR,  // '$', ord 0x24
    0b01001001,             // '%', ord 0x25
    SEVENSEG_UNKNOWN_CHAR,  // '&', ord 0x26
    0b00000010,             // ''', ord 0x27
    0b01001110,             // '(', ord 0x28
    0b01111000,             // ')', ord 0x29
    0b01000000,             // '*', ord 0x2A
    SEVENSEG_UNKNOWN_CHAR,  // '+', ord 0x2B
    0b00010000,             // ',', ord 0x2C
    0b00000001,             // '-', ord 0x2D
    0b10000000,             // '.', ord 0x2E
    SEVENSEG_UNKNOWN_CHAR,  // '/', ord 0x2F
    0b01111110,             // '0', ord 0x30
    0b00110000,             // '1', ord 0x31
    0b01101101,             // '2', ord 0x32
    0b01111001,             // '3', ord 0x33
    0b00110011,             // '4', ord 0x34
    0b01011011,             // '5', ord 0x35
    0b01011111,             // '6', ord 0x36
    0b01110000,             // '7', ord 0x37
    0b01111111,             // '8', ord 0x38
    0b01111011,             // '9', ord 0x39
    0b01001000,             // ':', ord 0x3A
    0b01011000,             // ';', ord 0x3B
    SEVENSEG_UNKNOWN_CHAR,  // '<', ord 0x3C
    0b00001001,             // '=', ord 0x3D
    SEVENSEG_UNKNOWN_CHAR,  // '>', ord 0x3E
    0b01100101,             // '?', ord 0x3F
    0b01101111,             // '@', ord 0x40
    0b01110111,             // 'A', ord 0x41
    0b00011111,             // 'B', ord 0x42
    0b01001110,             // 'C', ord 0x43
    0b00111101,             // 'D', ord 0x44
    0b01001111,             // 'E', ord 0x45
    0b01000111,             // 'F', ord 0x46
    0b01011110,             // 'G', ord 0x47
    0b00110111,             // 'H', ord 0x48
    0b00110000,             // 'I', ord 0x49
    0b00111100,             // 'J', ord 0x4A
    SEVENSEG_UNKNOWN_CHAR,  // 'K', ord 0x4B
    0b00001110,             // 'L', ord 0x4C
    0b01010101,             // 'M', ord 0x4D
    0b00010101,             // 'N', ord 0x4E
    0b01111110,             // 'O', ord 0x4F
    0b01100111,             // 'P', ord 0x50
    0b11111110,             // 'Q', ord 0x51
    0b00000101,             // 'R', ord 0x52
    0b01011011,             // 'S', ord 0x53
    0b00000111,             // 'T', ord 0x54
    0b00111110,             // 'U', ord 0x55
    0b00111110,             // 'V', ord 0x56
    0b00111111,             // 'W', ord 0x57
    SEVENSEG_UNKNOWN_CHAR,  // 'X', ord 0x58
    0b00100111,             // 'Y', ord 0x59
    0b01101101,             // 'Z', ord 0x5A
    0b01001110,             // '[', ord 0x5B
    SEVENSEG_UNKNOWN_CHAR,  // '\', ord 0x5C
    0b01111000,             // ']', ord 0x5D
    SEVENSEG_UNKNOWN_CHAR,  // '^', ord 0x5E
    0b00001000,             // '_', ord 0x5F
    0b00100000,             // '`', ord 0x60
    0b01110111,             // 'a', ord 0x61
    0b00011111,             // 'b', ord 0x62
    0b00001101,             // 'c', ord 0x63
    0b00111101,             // 'd', ord 0x64
    0b01001111,             // 'e', ord 0x65
    0b01000111,             // 'f', ord 0x66
    0b01011110,             // 'g', ord 0x67
    0b00010111,             // 'h', ord 0x68
    0b00010000,             // 'i', ord 0x69
    0b00111100,             // 'j', ord 0x6A
    SEVENSEG_UNKNOWN_CHAR,  // 'k', ord 0x6B
    0b00001110,             // 'l', ord 0x6C
    0b01010101,             // 'm', ord 0x6D
    0b00010101,             // 'n', ord 0x6E
    0b00011101,             // 'o', ord 0x6F
    0b01100111,             // 'p', ord 0x70
    SEVENSEG_UNKNOWN_CHAR,  // 'q', ord 0x71
    0b00000101,             // 'r', ord 0x72
    0b01011011,             // 's', ord 0x73
    0b00000111,             // 't', ord 0x74
    0b00011100,             // 'u', ord 0x75
    0b00011100,             // 'v', ord 0x76
    0b00101011,             // 'w', ord 0x77
    SEVENSEG_UNKNOWN_CHAR,  // 'x', ord 0x78
    0b00100111,             // 'y', ord 0x79
    SEVENSEG_UNKNOWN_CHAR,  // 'z', ord 0x7A
    0b00110001,             // '{', ord 0x7B
    0b00000110,             // '|', ord 0x7C
    0b00000111,             // '}', ord 0x7D
    0b01100011,             // '~', ord 0x7E (degree symbol)
    SEVENSEG_UNKNOWN_CHAR,  // 0x7F
};

std::array<std::array<GPIOPin *, SEVSEGX::MAX_SEGMENTS>, SEVSEGX::MAX_SECTIONS> SEVSEGX::segment_pins_;
std::array<std::array<uint8_t, SEVSEGX::MAX_SEGMENTS>, SEVSEGX::MAX_SECTIONS> SEVSEGX::segment_pin_nums_;
std::vector<GPIOPin *> SEVSEGX::digit_pins_;
std::vector<uint8_t> SEVSEGX::digit_pin_nums_;
std::vector<uint8_t> SEVSEGX::digit_brightness_;
std::vector<uint8_t> SEVSEGX::digit_dots_;
uint8_t SEVSEGX::display_brightness_;
uint8_t *SEVSEGX::buffer_;
uint8_t SEVSEGX::buffer_size_;
bool SEVSEGX::is_enabled;

// getter and setter
float SEVSEGX::get_setup_priority() const { return setup_priority::PROCESSOR; }
void SEVSEGX::set_writer(sevsegx_writer_t &&writer) { this->writer_ = writer; }

void SEVSEGX::set_segment_pins(std::vector<std::vector<GPIOPin *>> segment_pins) {
  for (size_t sec = 0; sec < MAX_SECTIONS; ++sec) {
    for (size_t seg = 0; seg < MAX_SEGMENTS; ++seg) {
      if (sec < segment_pins.size() && seg < segment_pins[sec].size()) {
        segment_pins_[sec][seg] = segment_pins[sec][seg];
        segment_pin_nums_[sec][seg] = ((InternalGPIOPin *) segment_pins_[sec][seg])->get_pin();

        ESP_LOGCONFIG(TAG, "Setting up segment pins: section %d, segment %d, GPIO %s", sec, seg,
                      segment_pins_[sec][seg]->dump_summary().c_str());
      } else {
        segment_pins_[sec][seg] = nullptr;
        segment_pin_nums_[sec][seg] = 255;
      }
    }
  }
}

void SEVSEGX::set_digit_pins(std::vector<GPIOPin *> digit_pins) {
  digit_pins_.assign(digit_pins.begin(), digit_pins.end());
  for (size_t i = 0; i < digit_pins_.size(); ++i) {
    digit_pin_nums_.push_back(((InternalGPIOPin *) digit_pins_[i])->get_pin());
  }
  // set up dots at the same time
  std::vector<uint8_t> v(digit_pins.size(), 0);
  digit_dots_.assign(v.begin(), v.end());
}

void SEVSEGX::set_digit_brightness(std::vector<uint8_t> digit_brightness) {
  digit_brightness_.assign(digit_brightness.begin(), digit_brightness.end());
}

std::vector<uint8_t> SEVSEGX::get_digit_brightness() { return digit_brightness_; }

void SEVSEGX::set_display_brightness(uint8_t display_brightness) { display_brightness_ = display_brightness; }

void SEVSEGX::set_digit_dots(std::vector<uint8_t> dots) { digit_dots_.assign(dots.begin(), dots.end()); }

std::vector<uint8_t> SEVSEGX::get_digit_dots() { return digit_dots_; }

bool SEVSEGX::get_display_state() { return is_enabled; }

// setup
void SEVSEGX::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SEVSEGX");
  for (const auto &pin : digit_pins_) {
    pin->setup();
    pin->pin_mode(esphome::gpio::Flags::FLAG_OUTPUT);
    pin->digital_write(false);
  }

  for (const auto &inner : segment_pins_) {
    for (const auto &pin : inner) {
      if (pin != nullptr) {
        pin->setup();
        pin->pin_mode(esphome::gpio::Flags::FLAG_OUTPUT);
        pin->digital_write(false);
      }
    }
  }

  buffer_size_ = digit_pins_.size() * segment_pins_.size();
  ESP_LOGCONFIG(TAG, "digit_pins_.size() %d", digit_pins_.size());
  ESP_LOGCONFIG(TAG, "segment_pins_.size() %d", segment_pins_.size());
  ESP_LOGCONFIG(TAG, "buffer_size_ %d", buffer_size_);
  buffer_ = new uint8_t[buffer_size_];

  this->setup_complete_ = true;
}

// dump config
void SEVSEGX::dump_config() {
  ESP_LOGCONFIG(TAG, "SEVENSEG:");
  ESP_LOGCONFIG(TAG, "Setup Complete: %s", this->setup_complete_ ? "YES" : "NO");
}

void SEVSEGX::print(std::string str) { return this->print(str.c_str()); }

void SEVSEGX::strftime(const char *format, ESPTime time) {
  if (is_enabled) {
    char buffer[buffer_size_ + 1];
    size_t ret = time.strftime(buffer, sizeof(buffer), format);
    if (ret > 0) {
      this->print(buffer);
    }
  }
}

}  // namespace sevsegx_base
}  // namespace esphome
