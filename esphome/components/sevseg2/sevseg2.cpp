#ifdef USE_ARDUINO

#include "sevseg2.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/core/component.h"
#include "esphome/core/application.h"
#include <hal/gpio_hal.h>

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

void SEVSEG2::setup() { SEVSEGX::setup(); }

void SEVSEG2::enable_display() {
#ifdef USE_ESP32
  if (display_timer == NULL) {
    display_timer = timerBegin(500000);
    timerAttachInterrupt(display_timer, &SEVSEG2::s_timer_intr);
    timerAlarm(display_timer, 10, true, 0);
    is_enabled = true;
  }
#endif
  ESP_LOGI(TAG, "SEVSEG2: enabled");
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

  ESP_LOGI(TAG, "SEVSEG2: timer disabled");
}

uint8_t SEVSEG2::half_buffer_size = buffer_size_ >> 1;

uint8_t anti_ghosting = 0;
uint8_t active_digit = 0;
uint8_t active_digit2 = SEVSEG2::half_buffer_size;
// 'pwm_counter' runs independently, simulating a PWM cycle (0 to 127).
uint8_t pwm_counter = 0;
uint8_t pwm_counter2 = 0;

uint8_t ch;
uint8_t segments;
uint8_t ch2;
uint8_t segments2;

uint32_t scaled_brightness1;
uint32_t scaled_brightness2;

uint8_t anti_ghosting_ticks = 2;  // configurable blanking ticks
uint8_t blanking_counter = 0;     // runtime blanking state

void IRAM_ATTR HOT SEVSEG2::timer_interrupt() {
  uint32_t mask_low = 0UL;
  uint32_t mask_high = 0UL;

  if (blanking_counter > 0) {
    // Blanking phase: turn off all segments
    blanking_counter--;

    GPIO.out_w1tc = 0xFFFFFFFFUL;
    GPIO.out1_w1tc.val = 0xFFFFFFFFUL;
    return;
  }

  // Load the character for the currently active digit.
  ch = buffer_[active_digit];
  ch2 = buffer_[active_digit2];

  segments = (ch < 128) ? SEVSEG_ASCII_TO_RAW[ch] : SEVSEG_ASCII_TO_RAW[127];
  segments2 = (ch2 < 128) ? SEVSEG_ASCII_TO_RAW[ch2] : SEVSEG_ASCII_TO_RAW[127];

  // Higher maximum brightness: scale to 0-63 range but with higher multiplier
  scaled_brightness1 = (digit_brightness_[active_digit] * display_brightness_) >> 6;   // >> 6 gives 0-63 range
  scaled_brightness2 = (digit_brightness_[active_digit2] * display_brightness_) >> 6;  // >> 6 gives 0-63 range

  // Ensure minimum visible brightness
  if (scaled_brightness1 == 0)
    scaled_brightness1 = 1;
  if (scaled_brightness2 == 0)
    scaled_brightness2 = 1;

  uint8_t pin;

  pin = digit_pin_nums_[active_digit];
  pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

  if (pwm_counter < scaled_brightness1) {
    pin = segment_pin_nums_[0][0];
    if (segments & 0b01000000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][1];
    if (segments & 0b00100000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][2];
    if (segments & 0b00010000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][3];
    if (segments & 0b00001000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][4];
    if (segments & 0b00000100)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][5];
    if (segments & 0b00000010)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][6];
    if (segments & 0b00000001)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[0][7];
    if ((segments & 0b10000000) || digit_dots_[active_digit])
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));
  }

  if (pwm_counter2 < scaled_brightness2) {
    pin = segment_pin_nums_[1][0];
    if (segments2 & 0b01000000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][1];
    if (segments2 & 0b00100000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][2];
    if (segments2 & 0b00010000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][3];
    if (segments2 & 0b00001000)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][4];
    if (segments2 & 0b00000100)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][5];
    if (segments2 & 0b00000010)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][6];
    if (segments2 & 0b00000001)
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));

    pin = segment_pin_nums_[1][7];
    if ((segments2 & 0b10000000) || digit_dots_[active_digit2])
      pin < 32 ? mask_low |= (1UL << pin) : mask_high |= (1UL << (pin - 32));
  }

  // Apply masks
  GPIO.out_w1ts = mask_low;   // Set bins
  GPIO.out_w1tc = ~mask_low;  // Clear bits

  GPIO.out1_w1ts.val = mask_high;
  GPIO.out1_w1tc.val = ~mask_high;

  // Advance digit every full PWM cycle
  pwm_counter = (pwm_counter + 1) & 63;
  pwm_counter2 = (pwm_counter2 + 1) & 63;

  if (pwm_counter == 0) {
    // Start blanking phase before switching digits
    blanking_counter = anti_ghosting_ticks;

    if ((buffer_size_ >> 1) - 1 > active_digit) {
      ++active_digit;
      ++active_digit2;
    } else {
      active_digit = 0;
      active_digit2 = buffer_size_ >> 1;
    }
  }
}

// print functions
void SEVSEG2::print(const char *str) {
  if (is_enabled) {
    //>> 1 == dividee by 2
    for (int i = 0; i < (buffer_size_ >> 1); i++) {  // half_buffer_size
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
