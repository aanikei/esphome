import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_LAMBDA
from esphome.pins import gpio_output_pin_schema

CONF_SEGMENT_PINS = "segment_pins"
CONF_DIGIT_PINS = "digit_pins"
CONF_DIGIT_BRIGHTNESS = "digit_brightness"
CONF_DISPLAY_BRIGHTNESS = "display_brightness"
MULTI_CONF = False

sevsegx_base_ns = cg.esphome_ns.namespace("sevsegx_base")
SEVSEGX = sevsegx_base_ns.class_("SEVSEGX", cg.Component)


def validate_segment_pins(value):
    value = cv.ensure_list(value)
    return [cv.ensure_list(gpio_output_pin_schema)(sublist) for sublist in value]


SEGMENT_PINS_VALIDATOR = cv.Schema(
    cv.ensure_list(cv.ensure_list(gpio_output_pin_schema))
)

SEVSEGX_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SEGMENT_PINS): SEGMENT_PINS_VALIDATOR,
        cv.Required(CONF_DIGIT_PINS): cv.ensure_list(gpio_output_pin_schema),
        cv.Required(CONF_DIGIT_BRIGHTNESS): cv.All(
            cv.ensure_list(cv.int_range(min=0, max=255)), cv.Length(min=1)
        ),
        cv.Optional(CONF_DISPLAY_BRIGHTNESS, default=255): cv.int_range(min=0, max=255),
    }
)


async def setup_sevsegx(var, config):
    # Setup each segment pin
    segment_pins = []
    for sublist in config[CONF_SEGMENT_PINS]:
        converted_sublist = []
        for pin_conf in sublist:
            pin_expr = await cg.gpio_pin_expression(pin_conf)
            converted_sublist.append(pin_expr)
        segment_pins.append(converted_sublist)

    cg.add(var.set_segment_pins(segment_pins))

    # Setup each digit pin
    digit_pins = [await cg.gpio_pin_expression(i) for i in config[CONF_DIGIT_PINS]]
    cg.add(var.set_digit_pins(digit_pins))

    # Setup each digit brightness
    digit_brightness = [int(i) for i in config[CONF_DIGIT_BRIGHTNESS]]
    cg.add(var.set_digit_brightness(digit_brightness))

    if CONF_DISPLAY_BRIGHTNESS in config:
        cg.add(var.set_display_brightness(config[CONF_DISPLAY_BRIGHTNESS]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(SEVSEGX, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
