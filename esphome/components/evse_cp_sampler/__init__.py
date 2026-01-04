import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, pins
from esphome.const import CONF_ID
from esphome.components.adc.sensor import ADCSensor

DEPENDENCIES = []

evse_cp_sampler_ns = cg.esphome_ns.namespace("evse_cp_sampler")
CpSampler = evse_cp_sampler_ns.class_("CpSampler", cg.Component)

StateChangeTrigger = evse_cp_sampler_ns.class_(
    "StateChangeTrigger", automation.Trigger.template(cg.int_)
)
RawValueTrigger = evse_cp_sampler_ns.class_(
    "RawValueTrigger", automation.Trigger.template(cg.int_)
)

CONF_PWM_INTERRUPT_PIN = "pwm_interrupt_pin"
CONF_SAMPLE_ADC = "sample_adc"
CONF_SAMPLES = "samples"
CONF_ON_STATE_CHANGE = "on_state_change"
CONF_ON_RAW_VALUE = "on_raw_value"
CONF_TRIGGER_ID = "trigger_id"
CONF_STATE_A_THRESHOLD = "state_a_threshold"
CONF_STATE_B_VALUE = "state_b_value"
CONF_STATE_B_THRESHOLD = "state_b_threshold"
CONF_STATE_C_VALUE = "state_c_value"
CONF_STATE_C_THRESHOLD = "state_c_threshold"
CONF_STATE_E_VALUE = "state_e_value"
CONF_STATE_E_THRESHOLD = "state_e_threshold"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CpSampler),
            cv.Required(CONF_PWM_INTERRUPT_PIN): pins.internal_gpio_output_pin_number,
            cv.Required(CONF_SAMPLE_ADC): cv.use_id(ADCSensor),
            cv.Optional(CONF_SAMPLES, default=250): cv.positive_int,
            cv.Optional(CONF_ON_STATE_CHANGE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(StateChangeTrigger),
                }
            ),
            cv.Optional(CONF_ON_RAW_VALUE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(RawValueTrigger),
                }
            ),
            cv.Optional(CONF_STATE_A_THRESHOLD, default=4000): cv.positive_int,
            cv.Optional(CONF_STATE_B_VALUE, default=3650): cv.positive_int,
            cv.Optional(CONF_STATE_B_THRESHOLD, default=150): cv.positive_int,
            cv.Optional(CONF_STATE_C_VALUE, default=3200): cv.positive_int,
            cv.Optional(CONF_STATE_C_THRESHOLD, default=150): cv.positive_int,
            cv.Optional(CONF_STATE_E_VALUE, default=755): cv.positive_int,
            cv.Optional(CONF_STATE_E_THRESHOLD, default=150): cv.positive_int,
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    adc_var = await cg.get_variable(config[CONF_SAMPLE_ADC])
    pin = config[CONF_PWM_INTERRUPT_PIN]
    cg.add(var.set_pwm_interrupt_pin(pin))
    cg.add(var.set_adc(adc_var))
    cg.add(var.set_samples(config[CONF_SAMPLES]))
    cg.add(var.set_state_a_threshold(config[CONF_STATE_A_THRESHOLD]))
    cg.add(var.set_state_b_value(config[CONF_STATE_B_VALUE]))
    cg.add(var.set_state_b_threshold(config[CONF_STATE_B_THRESHOLD]))
    cg.add(var.set_state_c_value(config[CONF_STATE_C_VALUE]))
    cg.add(var.set_state_c_threshold(config[CONF_STATE_C_THRESHOLD]))
    cg.add(var.set_state_e_value(config[CONF_STATE_E_VALUE]))
    cg.add(var.set_state_e_threshold(config[CONF_STATE_E_THRESHOLD]))

    for conf in config.get(CONF_ON_STATE_CHANGE, []):
        trig = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trig, [(cg.int_, "x")], conf)

    for conf in config.get(CONF_ON_RAW_VALUE, []):
        trig = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trig, [(cg.int_, "x")], conf)

__all__ = [
    "CpSampler",
    "StateChangeTrigger",
    "RawValueTrigger",
]
