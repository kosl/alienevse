#include "evse_cp_sampler.h"
#include "esphome/core/log.h"

namespace esphome {
namespace evse_cp_sampler {

static const char *const TAG = "evse_cp_sampler";
  using namespace esphome::adc;

void CpSampler::setup() {
  count_ = 0;
  median_index_ = 0;
  median_count_ = 0;
  prev_state_ = -1;

  // Fill buffer with 0 initially (or some safe value)
  for (int i = 0; i < MEDIAN_WINDOW; i++) {
    median_buffer_[i] = 0;
  }

  // One-shot sampling timer (20 µs after rising edge)
  esp_timer_create_args_t timer_args = {
      .callback = timer_callback,
      .arg = this,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "cp_sample",
      .skip_unhandled_events = true,
  };
  esp_timer_create(&timer_args, &sample_timer_);

  // Create periodic heartbeat timer (500 Hz = 2000 µs period)
  esp_timer_create_args_t heartbeat_args = {
      .callback = timer_callback,
      .arg = this,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "cp_heartbeat",
      .skip_unhandled_events = true,
  };
  esp_timer_create(&heartbeat_args, &heartbeat_timer_);

  // PWM GPIO interrupt setup (rising edge on GPIO10)
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/gpio.html
  gpio_set_intr_type(PWM_PIN, GPIO_INTR_POSEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(PWM_PIN, gpio_isr_handler, this);
  gpio_intr_enable(PWM_PIN);  
  
  // Start heartbeat (periodic) timer immediately
  esp_timer_start_periodic(heartbeat_timer_, 2000);  // 2000 µs = 500 Hz

  
  // Override some ADC settings for our use case
  if (adc_sensor_ == nullptr) {
    ESP_LOGE(TAG, "ADC Sensor not set in CpSampler!");
    return;
  }
  adc_sensor_->set_update_interval(SCHEDULER_DONT_RUN);
  adc_sensor_->set_sample_count(1);
  adc_sensor_->set_output_raw(true);
}

int CpSampler::compute_median() {
  // Copy buffer to avoid modifying original during sort
  int sorted[MEDIAN_WINDOW];
  for (int i = 0; i < MEDIAN_WINDOW; i++) {
    sorted[i] = median_buffer_[i];
  }
  std::sort(sorted, sorted + MEDIAN_WINDOW);
  return sorted[MEDIAN_WINDOW / 2];
}

// This ISR is started by ledc PWM leading edge and schedules single-shot sample shortly after 
void IRAM_ATTR CpSampler::gpio_isr_handler(void *arg) {
  auto self = static_cast<CpSampler *>(arg);
  self->sample_start_time_ = esp_timer_get_time();
  // Restart heartbeat timer (resets its period) so that it will not trigger if under duty cycle
  esp_timer_restart(self->heartbeat_timer_, 2000);
  esp_timer_stop(self->sample_timer_); // Best effort if it is already running then stop it
  esp_timer_start_once(self->sample_timer_, 10);  // 10 µs delay
}
  
void IRAM_ATTR CpSampler::timer_callback(void *arg) {
  auto self = static_cast<CpSampler *>(arg);
  if (self->adc_sensor_ == nullptr) {
    return;  // ADC not set up
  }

  // Only sample ADC every DECIMATE_SAMPLES calls
  if (self->count_ % DECIMATE_SAMPLES == 0) {
    uint32_t raw_adc = static_cast<uint32_t>(self->adc_sensor_->sample());
    if (raw_adc == 0) {
      return;  // Invalid reading
    }

    if (self->sample_start_time_ != 0) { // Is conversion outside 20-85 us sampling window?
      int64_t elapsed = esp_timer_get_time() - self->sample_start_time_;
      self->sample_start_time_ = 0;  // Always disable checking for hearbeat samples
      if (elapsed > self->SAMPLE_STALE_TIME_US) {
	return;  // Too late, ignore stale sample
      }
    }

    // Store in circular buffer
    self->median_buffer_[self->median_index_] = raw_adc;
    self->median_index_ = (self->median_index_ + 1) % self->MEDIAN_WINDOW;
    if (self->median_count_ < self->MEDIAN_WINDOW) {
      self->median_count_++;
    }
  }

  self->count_ += 1;

  // Only process when we've collected enough PWM cycles
  if (self->count_ < self->samples_) {
    return;
  }

  // Reset counter
  self->count_ = 0;

  // Only trigger if we have enough samples for a valid median
  if (self->median_count_ < self->MEDIAN_WINDOW) {
    return;  // Not enough data yet
  }

  int filtered_value = self->compute_median();

  // Trigger raw value (now median-filtered)
  if (self->raw_value_trigger_) {
    self->raw_value_trigger_->trigger(filtered_value);
  }

  // --- State detection based on median-filtered value ---
  int new_state = 0;
  if (filtered_value > 4000) {
    new_state = 1;           // +12V → State A (no vehicle)
  } else if (abs(filtered_value - 3650) < 150) {
    new_state = 2;           // +9V  → State B (connected, not ready)
  } else if (abs(filtered_value - 3200) < 150) {
    new_state = 3;           // +6V  → State C (charging)
  } else if (abs(filtered_value - 755) < 150) {
    new_state = 4;           // -12V → State E/F (error or ventilation)
  }

  if (new_state != self->prev_state_) {
    if (self->state_change_trigger_) {
      self->state_change_trigger_->trigger(new_state);
    }
    self->prev_state_ = new_state;
  }
}

void CpSampler::dump_config() {
  ESP_LOGCONFIG(TAG, "EVSE CP Sampler:");
  ESP_LOGCONFIG(TAG, "  PWM on GPIO10");
  ESP_LOGCONFIG(TAG, "  ADC Sensor: %s", adc_sensor_ != nullptr ? adc_sensor_->get_name().c_str() : "None");
  ESP_LOGCONFIG(TAG, "  Samples per update: %d", samples_);
  ESP_LOGCONFIG(TAG, "  Median filter window: %d", MEDIAN_WINDOW);
  ESP_LOGCONFIG(TAG, "  Decimation factor: %d", DECIMATE_SAMPLES);
}

}  // namespace evse_cp_sampler
}  // namespace esphome
