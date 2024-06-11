load(":touch_modules.bzl", "touch_driver_modules")
load(":touch_modules_build.bzl", "define_target_variant_modules")
load("//msm-kernel:target_variants.bzl", "get_all_la_variants", "get_all_le_variants")

def define_sun(t,v):
    define_target_variant_modules(
        target = t,
        variant = v,
        registry = touch_driver_modules,
        modules = [
            "atmel_mxt_ts",
            "dummy_ts",
            "goodix_ts",
            "st_fts",
            "qts"
        ],
        config_options = [
            "TOUCH_DLKM_ENABLE",
            "CONFIG_ARCH_SUN",
            "CONFIG_MSM_TOUCH",
            "CONFIG_TOUCHSCREEN_GOODIX_BRL",
            "CONFIG_TOUCHSCREEN_ATMEL_MXT",
            "CONFIG_TOUCHSCREEN_ST",
            "CONFIG_QTS_ENABLE",
            "CONFIG_TOUCHSCREEN_DUMMY"
        ],
)

def define_pineapple(t,v):
    define_target_variant_modules(
        target = t,
        variant = v,
        registry = touch_driver_modules,
        modules = [
            "atmel_mxt_ts",
            "dummy_ts",
            "goodix_ts",
            "qts"
        ],
        config_options = [
            "TOUCH_DLKM_ENABLE",
            "CONFIG_ARCH_PINEAPPLE",
            "CONFIG_MSM_TOUCH",
            "CONFIG_TOUCHSCREEN_GOODIX_BRL",
            "CONFIG_TOUCHSCREEN_ATMEL_MXT",
            "CONFIG_TOUCHSCREEN_DUMMY",
            "CONFIG_QTS_ENABLE"
        ],
)

def define_blair(t,v):
    define_target_variant_modules(
        target = t,
        variant = v,
        registry = touch_driver_modules,
        modules = [
            "nt36xxx-i2c",
            "goodix_ts",
            "focaltech_fts",
            "synaptics_tcm_ts"
        ],
        config_options = [
            "TOUCH_DLKM_ENABLE",
            "CONFIG_ARCH_BLAIR",
            "CONFIG_MSM_TOUCH",
            "CONFIG_TOUCHSCREEN_NT36XXX_I2C",
            "CONFIG_TOUCHSCREEN_GOODIX_BRL",
            "CONFIG_TOUCH_FOCALTECH",
            "CONFIG_TOUCHSCREEN_SYNAPTICS_TCM"
        ],
)
def define_parrot(t,v):
    define_target_variant_modules(
        target = t,
        variant = v,
        registry = touch_driver_modules,
        modules = [
            "nt36xxx-i2c",
	    "goodix_ts",
	    "qts"
        ],
        config_options = [
            "TOUCH_DLKM_ENABLE",
            "CONFIG_ARCH_PARROT",
            "CONFIG_MSM_TOUCH",
            "CONFIG_TOUCHSCREEN_NT36XXX_I2C",
	     "CONFIG_TOUCHSCREEN_GOODIX_BRL",
	     "CONFIG_QTS_ENABLE"
        ],
)
def define_touch_target():
    for (t, v) in get_all_la_variants() + get_all_le_variants():
        if t == "blair":
            define_blair(t, v)
        elif t == "pineapple":
            define_pineapple(t, v)
        elif t == "parrot":
            define_parrot(t, v)
        else:
            define_sun(t, v)
