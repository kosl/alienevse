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

  // Setters
  void set_pwm_interrupt_pin(int pin) { pwm_interrupt_pin_ = pin; }
  void set_adc(adc::ADCSensor *adc_comp) { adc_sensor_ = adc_comp; }
  void set_samples(int s) { samples_ = s; }
  void set_state_change_trigger(StateChangeTrigger *t) { state_change_trigger_ = t; }
  void set_raw_value_trigger(RawValueTrigger *t) { raw_value_trigger_ = t; }
  void set_state_a_threshold(int v) { state_a_thr_ = v; }
  void set_state_b_value(int v) { state_b_val_ = v; }
  void set_state_b_threshold(int v) { state_b_thr_ = v; }
  void set_state_c_value(int v) { state_c_val_ = v; }
  void set_state_c_threshold(int v) { state_c_thr_ = v; }
  void set_state_e_value(int v) {  state_e_val_ = v; }
  void set_state_e_threshold(int v) { state_e_thr_ = v; }

 protected:
  int pwm_interrupt_pin_ = GPIO_NUM_10; // Default to GPIO10
  adc::ADCSensor *adc_sensor_{nullptr};
  static constexpr int DECIMATE_SAMPLES = 10;

  // Median filter settings
  static constexpr int MEDIAN_WINDOW = 20;
  int median_buffer_[MEDIAN_WINDOW];
  int median_index_ = 0;
  int median_count_ = 0;

  int samples_;
  int count_ = 0;

  int64_t sample_start_time_{0};
  static constexpr int64_t SAMPLE_STALE_TIME_US = 100; // 100us for 10% duty cycle at 1kHz

  // State thresholds
  int state_a_thr_ = 4000; // Any value above this is State A (+12V)
  int state_b_val_ = 3650; // State B nominal value (+9V)
  int state_b_thr_ = 150;
  int state_c_val_ = 3200; // State C nominal value (+6V)
  int state_c_thr_ = 150;
  int state_e_val_ = 755;  // State E/F nominal value (-12V)
  int state_e_thr_ = 150;

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
