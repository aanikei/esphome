#ifdef USE_ARDUINO

#include "sevseg2.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <esp32-hal-timer.h>
#endif

namespace esphome {
namespace sevseg2 {

static const char *const TAG = "sevseg2";

#ifdef USE_ESP32
// ESP32 implementation, needs to wrap timer_interrupt() function to auto-reschedule
static hw_timer_t *display_timer = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void IRAM_ATTR HOT SEVSEG2::s_timer_intr() { timer_interrupt(); }
#endif

void SEVSEG2::enable_display() {
#ifdef USE_ESP32
  if (display_timer == NULL) {
    display_timer = timerBegin(1000000);
    timerAttachInterrupt(display_timer, &SEVSEG2::s_timer_intr);
    timerAlarm(display_timer, 30, true, 0);
    is_enabled = true;
  }
#endif
  ESP_LOGCONFIG(TAG, "SEVSEG2: enabled");
}

void SEVSEG2::disable_display() {
#ifdef USE_ESP32
  if (display_timer != NULL) {
    timerDetachInterrupt(display_timer);
    timerEnd(display_timer);
    display_timer = NULL;
    is_enabled = false;
  }
#endif

  for (GPIOPin *pin : digit_pins_) {
    pin->digital_write(false);
  }

  for (const auto &inner : segment_pins_) {
    for (const auto &pin : inner) {
      pin->digital_write(false);
    }
  }

  ESP_LOGD(TAG, "SEVSEG2: timer disabled");
}

void HOT SEVSEG2::timer_interrupt() {
  static uint8_t anti_ghosting = 0;

  static uint8_t activeDigit = 0;
  static uint8_t activeDigit2 = buffer_size_ >> 1;

  // 'pwmCounter' runs independently, simulating a PWM cycle (0 to 127).
  static uint8_t pwmCounter = 0;
  static uint8_t pwmCounter2 = 0;

  // Load the character for the currently active digit.
  uint8_t ch = buffer_[activeDigit];
  // uint8_t segments = (ch < 128) ? SEVSEG_ASCII_TO_RAW[ch] : SEVSEG_ASCII_TO_RAW[127];
  uint8_t segments = SEVSEG_ASCII_TO_RAW[ch];

  uint8_t ch2 = buffer_[activeDigit2];
  // uint8_t segments2 = (ch2 < 128) ? SEVSEG_ASCII_TO_RAW[ch2] : SEVSEG_ASCII_TO_RAW[127];
  uint8_t segments2 = SEVSEG_ASCII_TO_RAW[ch2];

  if (anti_ghosting == 0) {
    ++anti_ghosting;

    // digit_pins_[activeDigit]->digital_write(true);

    uint32_t scaledBrightness1 = (digit_brightness_[activeDigit] * display_brightness_ * 128) >> 16;
    uint32_t scaledBrightness2 = (digit_brightness_[activeDigit2] * display_brightness_ * 128) >> 16;

    bool state1 = pwmCounter < scaledBrightness1;
    bool state2 = pwmCounter2 < scaledBrightness2;

    segment_pins_[1][7]->digital_write(state2 && ((segments2 & 0b10000000) || digit_dots_[activeDigit2]));
    segment_pins_[0][7]->digital_write(state1 && ((segments & 0b10000000) || digit_dots_[activeDigit]));
    segment_pins_[1][4]->digital_write(state2 && (segments2 & 0b00000100));
    segment_pins_[0][4]->digital_write(state1 && (segments & 0b00000100));
    segment_pins_[1][5]->digital_write(state2 && (segments2 & 0b00000010));
    segment_pins_[0][5]->digital_write(state1 && (segments & 0b00000010));
    segment_pins_[1][3]->digital_write(state2 && (segments2 & 0b00001000));
    segment_pins_[0][3]->digital_write(state1 && (segments & 0b00001000));
    segment_pins_[1][6]->digital_write(state2 && (segments2 & 0b00000001));
    segment_pins_[0][6]->digital_write(state1 && (segments & 0b00000001));
    segment_pins_[1][0]->digital_write(state2 && (segments2 & 0b01000000));
    segment_pins_[0][0]->digital_write(state1 && (segments & 0b01000000));
    segment_pins_[1][1]->digital_write(state2 && (segments2 & 0b00100000));
    segment_pins_[0][1]->digital_write(state1 && (segments & 0b00100000));
    segment_pins_[1][2]->digital_write(state2 && (segments2 & 0b00010000));
    segment_pins_[0][2]->digital_write(state1 && (segments & 0b00010000));

  } else {
    --anti_ghosting;

    segment_pins_[1][7]->digital_write(false);
    segment_pins_[0][7]->digital_write(false);
    segment_pins_[1][4]->digital_write(false);
    segment_pins_[0][4]->digital_write(false);
    segment_pins_[1][5]->digital_write(false);
    segment_pins_[0][5]->digital_write(false);
    segment_pins_[1][3]->digital_write(false);
    segment_pins_[0][3]->digital_write(false);
    segment_pins_[1][6]->digital_write(false);
    segment_pins_[0][6]->digital_write(false);
    segment_pins_[1][0]->digital_write(false);
    segment_pins_[0][0]->digital_write(false);
    segment_pins_[1][1]->digital_write(false);
    segment_pins_[0][1]->digital_write(false);
    segment_pins_[1][2]->digital_write(false);
    segment_pins_[0][2]->digital_write(false);

    digit_pins_[activeDigit]->digital_write(false);

    if ((buffer_size_ >> 1) - 1 > activeDigit) {
      ++activeDigit;
      ++activeDigit2;
    } else {
      activeDigit = 0;
      activeDigit2 = buffer_size_ >> 1;
    }

    // Advance the PWM counter (range 0-127). Using bit masking for efficiency.
    pwmCounter = (pwmCounter + 1) & 127;
    pwmCounter2 = (pwmCounter2 + 1) & 127;
  }
  // 'activeDigit' is used to cycle through the digits for multiplexing.
}

// print functions
void SEVSEG2::print(const char *str) {
  if (is_enabled) {
    //>> 1 == dividee by 2
    for (int i = 0; i < (buffer_size_ >> 1); i++) {
      uint8_t data1 = (uint8_t) str[buffer_size_ - 1 - i];
      if (data1 >= 0 && data1 < 128) {
        buffer_[i] = data1;
      } else {
        buffer_[i] = 127;
      }

      uint8_t data2 = (uint8_t) str[(buffer_size_ >> 1) - 1 - i];
      if (data2 >= 0 && data2 < 128) {
        buffer_[(buffer_size_ >> 1) + i] = data2;
      } else {
        buffer_[(buffer_size_ >> 1) + i] = 127;
      }
    }
  }
}

}  // namespace sevseg2
}  // namespace esphome

#endif  // USE_ARDUINO
