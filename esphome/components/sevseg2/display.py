import esphome.codegen as cg
from esphome.components import sevsegx_base
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["sevsegx_base"]

sevseg2_ns = cg.esphome_ns.namespace("sevseg2")
SEVSEG2 = sevseg2_ns.class_("SEVSEG2", sevsegx_base.SEVSEGX)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SEVSEG2),
        }
    )
    .extend(sevsegx_base.SEVSEGX_SCHEMA)
    .extend({cv.only_on_esp32: True, cv.only_with_arduino: True})
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sevsegx_base.setup_sevsegx(var, config)
