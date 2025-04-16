#include <core/common_defines.h>
#include "js_modules.h"
#include <m-dict.h>
#include "modules/js_flipper.h"

#define TAG "JS modules"

// Absolute path is used to make possible plugin load from CLI
#define MODULES_PATH "/ext/apps_data/js_app/plugins"

typedef struct {
    JsModeConstructor create;
    JsModeDestructor destroy;
    void* context;
} JsModuleData;

DICT_DEF2(JsModuleDict, FuriString*, FURI_STRING_OPLIST, JsModuleData, M_POD_OPLIST);

static const JsModuleDescriptor modules_builtin[] = {
    {"flipper", js_flipper_create, NULL},
};

struct JsModules {
    struct mjs* mjs;
    JsModuleDict_t module_dict;
    PluginManager* plugin_manager;
};

JsModules* js_modules_create(struct mjs* mjs, CompositeApiResolver* resolver) {
    JsModules* modules = malloc(sizeof(JsModules));
    modules->mjs = mjs;
    JsModuleDict_init(modules->module_dict);

    modules->plugin_manager = plugin_manager_alloc(
        PLUGIN_APP_ID, PLUGIN_API_VERSION, composite_api_resolver_get(resolver));

    return modules;
}

void js_modules_destroy(JsModules* modules) {
    JsModuleDict_it_t it;
    for(JsModuleDict_it(it, modules->module_dict); !JsModuleDict_end_p(it);
        JsModuleDict_next(it)) {
        const JsModuleDict_itref_t* module_itref = JsModuleDict_cref(it);
        if(module_itref->value.destroy) {
            module_itref->value.destroy(module_itref->value.context);
        }
    }
    plugin_manager_free(modules->plugin_manager);
    JsModuleDict_clear(modules->module_dict);
    free(modules);
}

mjs_val_t js_module_require(JsModules* modules, const char* name, size_t name_len) {
    FuriString* module_name = furi_string_alloc_set_str(name);
    // Check if module is already installed
    JsModuleData* module_inst = JsModuleDict_get(modules->module_dict, module_name);
    if(module_inst) { //-V547
        furi_string_free(module_name);
        mjs_prepend_errorf(
            modules->mjs, MJS_BAD_ARGS_ERROR, "\"%s\" module is already installed", name);
        return MJS_UNDEFINED;
    }

    bool module_found = false;
    // Check built-in modules
    for(size_t i = 0; i < COUNT_OF(modules_builtin); i++) { //-V1008
        size_t name_compare_len = strlen(modules_builtin[i].name);

        if(name_compare_len != name_len) {
            continue;
        }

        if(strncmp(name, modules_builtin[i].name, name_compare_len) == 0) {
            JsModuleData module = {
                .create = modules_builtin[i].create, .destroy = modules_builtin[i].destroy};
            JsModuleDict_set_at(modules->module_dict, module_name, module);
            module_found = true;
            FURI_LOG_I(TAG, "Using built-in module %s", name);
            break;
        }
    }

    // External module load
    if(!module_found) {
        FuriString* module_path = furi_string_alloc();
        furi_string_printf(module_path, "%s/js_%s.fal", MODULES_PATH, name);
        FURI_LOG_I(TAG, "Loading external module %s", furi_string_get_cstr(module_path));
        do {
            uint32_t plugin_cnt_last = plugin_manager_get_count(modules->plugin_manager);
            PluginManagerError load_error = plugin_manager_load_single(
                modules->plugin_manager, furi_string_get_cstr(module_path));
            if(load_error != PluginManagerErrorNone) {
                break;
            }
            const JsModuleDescriptor* plugin =
                plugin_manager_get_ep(modules->plugin_manager, plugin_cnt_last);
            furi_assert(plugin);

            if(strncmp(name, plugin->name, name_len) != 0) {
                FURI_LOG_E(TAG, "Module name missmatch %s", plugin->name);
                break;
            }
            JsModuleData module = {.create = plugin->create, .destroy = plugin->destroy};
            JsModuleDict_set_at(modules->module_dict, module_name, module);

            module_found = true;
        } while(0);
        furi_string_free(module_path);
    }

    // Run module constructor
    mjs_val_t module_object = MJS_UNDEFINED;
    if(module_found) {
        module_inst = JsModuleDict_get(modules->module_dict, module_name);
        furi_assert(module_inst);
        if(module_inst->create) { //-V779
            module_inst->context = module_inst->create(modules->mjs, &module_object);
        }
    }

    if(module_object == MJS_UNDEFINED) { //-V547
        mjs_prepend_errorf(modules->mjs, MJS_BAD_ARGS_ERROR, "\"%s\" module load fail", name);
    }

    furi_string_free(module_name);

    return module_object;
}
<<<<<<< HEAD
=======

void* js_module_get(JsModules* modules, const char* name) {
    FuriString* module_name = furi_string_alloc_set_str(name);
    JsModuleData* module_inst = js_find_loaded_module(modules, name);
    furi_string_free(module_name);
    return module_inst ? module_inst->context : NULL;
}

typedef enum {
    JsSdkCompatStatusCompatible,
    JsSdkCompatStatusFirmwareTooOld,
    JsSdkCompatStatusFirmwareTooNew,
} JsSdkCompatStatus;

/**
 * @brief Checks compatibility between the firmware and the JS SDK version
 *        expected by the script
 */
static JsSdkCompatStatus
    js_internal_sdk_compatibility_status(int32_t exp_major, int32_t exp_minor) {
    if(exp_major < JS_SDK_MAJOR) return JsSdkCompatStatusFirmwareTooNew;
    if(exp_major > JS_SDK_MAJOR || exp_minor > JS_SDK_MINOR)
        return JsSdkCompatStatusFirmwareTooOld;
    return JsSdkCompatStatusCompatible;
}

#define JS_SDK_COMPAT_ARGS \
    int32_t major, minor;  \
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_INT32(&major), JS_ARG_INT32(&minor));

void js_sdk_compatibility_status(struct mjs* mjs) {
    JS_SDK_COMPAT_ARGS;
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    switch(status) {
    case JsSdkCompatStatusCompatible:
        mjs_return(mjs, mjs_mk_string(mjs, "compatible", ~0, 0));
        return;
    case JsSdkCompatStatusFirmwareTooOld:
        mjs_return(mjs, mjs_mk_string(mjs, "firmwareTooOld", ~0, 0));
        return;
    case JsSdkCompatStatusFirmwareTooNew:
        mjs_return(mjs, mjs_mk_string(mjs, "firmwareTooNew", ~0, 0));
        return;
    }
}

void js_is_sdk_compatible(struct mjs* mjs) {
    JS_SDK_COMPAT_ARGS;
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    mjs_return(mjs, mjs_mk_boolean(mjs, status == JsSdkCompatStatusCompatible));
}

/**
 * @brief Asks the user whether to continue executing an incompatible script
 */
static bool js_internal_compat_ask_user(const char* message) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* dialog = dialog_message_alloc();
    dialog_message_set_header(dialog, message, 64, 0, AlignCenter, AlignTop);
    dialog_message_set_text(
        dialog, "This script may not\nwork as expected", 79, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(dialog, &I_Warning_30x23, 0, 18);
    dialog_message_set_buttons(dialog, "Go back", NULL, "Run anyway");
    DialogMessageButton choice = dialog_message_show(dialogs, dialog);
    dialog_message_free(dialog);
    furi_record_close(RECORD_DIALOGS);
    return choice == DialogMessageButtonRight;
}

void js_check_sdk_compatibility(struct mjs* mjs) {
    JS_SDK_COMPAT_ARGS;
    JsSdkCompatStatus status = js_internal_sdk_compatibility_status(major, minor);
    if(status != JsSdkCompatStatusCompatible) {
        FURI_LOG_E(
            TAG,
            "Script requests JS SDK %ld.%ld, firmware provides JS SDK %d.%d",
            major,
            minor,
            JS_SDK_MAJOR,
            JS_SDK_MINOR);

        const char* message = (status == JsSdkCompatStatusFirmwareTooOld) ? "Outdated Firmware" :
                                                                            "Outdated Script";
        if(!js_internal_compat_ask_user(message)) {
            JS_ERROR_AND_RETURN(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Incompatible script");
        }
    }
}

static const char* extra_features[] = {
    "baseline", // dummy "feature"
    "gpio-pwm",
    "gui-widget",
    "serial-framing",
    "gui-widget-extras",

    // extra modules
    "blebeacon",
    "i2c",
    "spi",
    "subghz",
    "usbdisk",
    "vgm",

    // extra features
    "gui-textinput-illegalsymbols",
    "storage-virtual",
    "usbdisk-createimage",
};

/**
 * @brief Determines whether a feature is supported
 */
static bool js_internal_supports(const char* feature) {
    for(size_t i = 0; i < COUNT_OF(extra_features); i++) { // -V1008
        if(strcmp(feature, extra_features[i]) == 0) return true;
    }
    return false;
}

/**
 * @brief Determines whether all of the requested features are supported
 */
static bool js_internal_supports_all_of(struct mjs* mjs, mjs_val_t feature_arr) {
    furi_assert(mjs_is_array(feature_arr));

    for(size_t i = 0; i < mjs_array_length(mjs, feature_arr); i++) {
        mjs_val_t feature = mjs_array_get(mjs, feature_arr, i);
        const char* feature_str = mjs_get_string(mjs, &feature, NULL);
        if(!feature_str) return false;

        if(!js_internal_supports(feature_str)) return false;
    }

    return true;
}

void js_does_sdk_support(struct mjs* mjs) {
    mjs_val_t features;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_ARR(&features));
    mjs_return(mjs, mjs_mk_boolean(mjs, js_internal_supports_all_of(mjs, features)));
}

void js_check_sdk_features(struct mjs* mjs) {
    mjs_val_t features;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_ARR(&features));
    if(!js_internal_supports_all_of(mjs, features)) {
        FURI_LOG_E(TAG, "Script requests unsupported features");

        if(!js_internal_compat_ask_user("Unsupported Feature")) {
            JS_ERROR_AND_RETURN(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Incompatible script");
        }
    }
}
>>>>>>> deva
