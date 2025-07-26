#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/core/gpio.h"
#include "esphome/components/sevsegx_base/sevsegx_base.h"

namespace esphome {
namespace sevseg2 {

class SEVSEG2 : public sevsegx_base::SEVSEGX {
 public:
  void setup() override { SEVSEGX::setup(); }
  void disable_display() override;
  void enable_display() override;
  void print(const char *str) override;
  using SEVSEGX::print;

 protected:
  static void timer_interrupt();

#ifdef USE_ESP32
  static void s_timer_intr();
#endif
};

}  // namespace sevseg2
}  // namespace esphome
