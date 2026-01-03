#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/adc/adc_sensor.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <algorithm>  // for std::sort

namespace esphome {
namespace evse_cp_sampler {

class CpSampler;

class StateChangeTrigger : public Trigger<int> {
 public:
  explicit StateChangeTrigger(CpSampler *parent);
};

class RawValueTrigger : public Trigger<int> {
 public:
  explicit RawValueTrigger(CpSampler *parent);
};

class CpSampler : public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_adc(adc::ADCSensor *adc_comp) { adc_sensor_ = adc_comp; }
  void set_samples(int s) { samples_ = s; }
  void set_state_change_trigger(StateChangeTrigger *t) { state_change_trigger_ = t; }
  void set_raw_value_trigger(RawValueTrigger *t) { raw_value_trigger_ = t; }

 protected:
  adc::ADCSensor *adc_sensor_{nullptr};
  static constexpr gpio_num_t PWM_PIN = GPIO_NUM_10;
  static constexpr int DECIMATE_SAMPLES = 10;

  // Median filter settings
  static constexpr int MEDIAN_WINDOW = 20;
  int median_buffer_[MEDIAN_WINDOW];
  int median_index_ = 0;
  int median_count_ = 0;

  int samples_;
  int count_ = 0;

  int64_t sample_start_time_{0};
  static constexpr int64_t SAMPLE_STALE_TIME_US = 83; // 83us for 8.3% duty cycle at 1kHz

  int prev_state_ = -1;

  StateChangeTrigger *state_change_trigger_{nullptr};
  RawValueTrigger *raw_value_trigger_{nullptr};

  esp_timer_handle_t sample_timer_{nullptr};
  esp_timer_handle_t heartbeat_timer_{nullptr};

  static void IRAM_ATTR gpio_isr_handler(void *arg);
  static void IRAM_ATTR timer_callback(void *arg);

  void start_sample_timer();

  // Helper to compute median from circular buffer
  int compute_median();
};

inline StateChangeTrigger::StateChangeTrigger(CpSampler *parent) {
  if (parent != nullptr)
    parent->set_state_change_trigger(this);
}

inline RawValueTrigger::RawValueTrigger(CpSampler *parent) {
  if (parent != nullptr)
    parent->set_raw_value_trigger(this);
}

}  // namespace evse_cp_sampler
}  // namespace esphome
