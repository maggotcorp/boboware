#include "subghz_setting.h"
#include "types.h" // IWYU pragma: keep
#include <furi.h>
#include <lib/subghz/devices/cc1101_configs.h>
#define TAG "SubGhzSetting"
#define FREQUENCY_FLAG_DEFAULT (1 << 31)
#define FREQUENCY_MASK         (0xFFFFFFFF ^ FREQUENCY_FLAG_DEFAULT)
/* my precious */
static const uint32_t subghz_frequency_list[] = {
    /* 300 - 348 */
    300000000,
    301000000,
    302757000,
    303000000,
    303875000,
    303900000,
    304250000,
    304500000,
    304800000,
    305000000,
    305500000,
    305800000,
    307000000,
    307500000,
    307800000,
    309000000,
    310000000,
    310500000,
    312000000,
    312100000,
    312200000,
    313000000,
    313100000,
    313200000,
    313300000,
    313400000,
    313500000,
    313600000,
    313650000,
    313700000,
    313750000,
    313800000,
    313850000,
    313900000,
    313950000,
    314000000,
    314100000,
    314200000,
    314250000,
    314300000,
    314350000,
    314400000,
    314450000,
    314500000,
    314550000,
    314600000,
    314650000,
    314700000,
    314750000,
    314800000,
    324850000,
    314980000,
    315000000,
    315500000,
    318000000,
    318500000,
    319000000,
    320000000,
    320500000,
    321000000,
    321500000,
    322000000,
    322500000,
    323000000,
    323500000,
    324000000,
    324500000,
    325000000,
    325500000,
    326000000,
    326500000,
    327000000,
    327500000,
    328000000,
    328500000,
    329000000,
    329500000,
    330000000,
    345000000,
    348000000,
    350000000,

    /* 387 - 464 */
    387000000,
    390000000,
	400000000,
	400500000,
	401000000,
	401500000,
	402000000,
	402500000,
	403000000,
	403500000,
	404000000,
	404500000,
	405000000,
	405500000,
	406000000,
	406500000,
	406660000,
	406800000,
	407000000,
	407500000,
	408000000,
	408500000,
	409000000,
	409500000,
	410000000,
	410500000,
	411000000,
	411500000,
	412000000,
	412500000,
	413000000,
	413500000,
	414000000,
	414500000,
	415000000,
	415500000,
	416000000,
	416500000,
	417000000,
    418000000,
    418200000,
    418300000,
    418500000,
    418800000,
    419000000,
    419500000,
    420000000,
    420500000,
    421000000,
    421500000,
    422000000,
    422500000,
    423000000,
    423500000,
    424000000,
    424500000,
    425000000,
    425500000,
    426000000,
    426500000,
    427000000,
    427500000,
    428000000,
    428500000,
    429000000,
    429500000,
    430000000,
    430200000,
    430300000,
    430400000,
    430500000,
    430600000,
    431000000,
    431100000,
    431200000,
    431500000,
    431600000,
    431700000,
    431800000,
    431900000,
    432000000,
    432500000,
    433000000,
    433050000,
    433075000, /* LPD433 first */
    433220000,
    433420000,
    433657070,
    433889000,
    433920000, /* LPD433 mid */
    434075000,
    434100000,
    434176948,
    434190000,
    434390000,
    434420000,
    434620000,
    434775000, /* LPD433 last channels */
    434800000,
    434900000,
    435000000,
    435050000,
    435075000,
    435100000,
    435200000,
    435300000,
    436000000,
    437000000,
    437500000,
    437800000,
    438000000,
    438900000,
    439000000,
    439500000,
    440175000,
    450000000,
    450500000,
    451000000,
    451500000,
    452000000,
    452500000,
    453000000,
    453500000,
    454000000,
	454400000,
	454500000,
	454600000,
	454700000,
	454800000,
	454900000,
	455000000,
	456000000,
	457000000,
	458000000,
	459000000,
	460000000,
	461000000,
	462000000,
	463000000,
    464000000,
	464400000,
	465000000,
	465500000,
	466000000,
	466500000,
	467000000,
    467750000,
    468000000,
    468500000,
    469000000,
    469500000,
    470000000,

    /* 779 - 928 */
    779000000,
    868350000,
    868400000,
    868460000,
    868800000,
    868950000,
    906400000,
    915000000,
    925000000,
    928000000,
};

static const uint32_t subghz_hopper_frequency_list[] = {
    /* 300 - 348 */
    300000000,
    301000000,
    302757000,
    303875000,
    303900000,
    304250000,
    304500000,
    304800000,
    305000000,
    305500000,
    305800000,
    307000000,
    307500000,
    307800000,
    309000000,
    310000000,
    310500000,
    312000000,
    312100000,
    312200000,
    313000000,
    313850000,
    314000000,
    314350000,
    314980000,
    315000000,
    318000000,
    318500000,
    319000000,
    320000000,
    320500000,
    321000000,
    321500000,
    322000000,
    322500000,
    323000000,
    323500000,
    324000000,
    324500000,
    325000000,
    325500000,
    326000000,
    326500000,
    327000000,
    327500000,
    328000000,
    328500000,
    329000000,
    329500000,
    330000000,
    345000000,
    348000000,
    350000000,
    /* 387 - 464 */
    387000000,
    390000000,
	400000000,
	400500000,
	401000000,
	401500000,
	402000000,
	402500000,
	403000000,
	403500000,
	404000000,
	404500000,
	405000000,
	405500000,
	406000000,
	406500000,
	406660000,
	406800000,
	407000000,
	407500000,
	408000000,
	408500000,
	409000000,
	409500000,
	410000000,
	410500000,
	411000000,
	411500000,
	412000000,
	412500000,
	413000000,
	413500000,
	414000000,
	414500000,
	415000000,
	415500000,
	416000000,
	416500000,
	417000000,
    418000000,
    418200000,
    418300000,
    418500000,
    418800000,
    419000000,
    419500000,
    420000000,
    420500000,
    421000000,
    421500000,
    422000000,
    422500000,
    423000000,
    423500000,
    424000000,
    424500000,
    425000000,
    425500000,
    426000000,
    426500000,
    427000000,
    427500000,
    428000000,
    428500000,
    429000000,
    429500000,
    430000000,
    430200000,
    430300000,
    430400000,
    430500000,
    430600000,
    431000000,
    431100000,
    431200000,
    431500000,
    431600000,
    431700000,
    431800000,
    431900000,
    432000000,
    432500000,
    433000000,
    433050000,
    433075000, /* LPD433 first */
    433220000,
    433420000,
    433657070,
    433889000,
    433920000, /* LPD433 mid */
    434075000,
    434100000,
    434176948,
    434190000,
    434390000,
    434420000,
    434620000,
    434775000, /* LPD433 last channels */
    434800000,
    434900000,
    435000000,
    435050000,
    435075000,
    435100000,
    435200000,
    435300000,
    436000000,
    437000000,
    437500000,
    437800000,
    438000000,
    438900000,
    439000000,
    439500000,
    440175000,
    450000000,
    450500000,
    451000000,
    451500000,
    452000000,
    452500000,
    453000000,
    453500000,
    454000000,
	454400000,
	454500000,
	454600000,
	454700000,
	454800000,
	454900000,
	455000000,
	456000000,
	457000000,
	458000000,
	459000000,
	460000000,
	461000000,
	462000000,
	463000000,
    464000000,
	464400000,
	465000000,
	465500000,
	466000000,
	466500000,
	467000000,
    467750000,
    468000000,
    468500000,
    469000000,
    469500000,
    470000000,

    /* 779 - 928 */
    779000000,
    868350000,
    868400000,
    868800000,
    868950000,
    900000000,
    906400000,
    915000000,
    925000000,
    928000000,
};

typedef struct {
    FuriString* custom_preset_name;
    uint8_t* custom_preset_data;
    size_t custom_preset_data_size;
} SubGhzSettingCustomPresetItem;

ARRAY_DEF(SubGhzSettingCustomPresetItemArray, SubGhzSettingCustomPresetItem, M_POD_OPLIST)

#define M_OPL_SubGhzSettingCustomPresetItemArray_t() \
    ARRAY_OPLIST(SubGhzSettingCustomPresetItemArray, M_POD_OPLIST)

typedef struct {
    SubGhzSettingCustomPresetItemArray_t data;
} SubGhzSettingCustomPresetStruct;

struct SubGhzSetting {
    FrequencyList_t frequencies;
    FrequencyList_t hopper_frequencies;
    SubGhzSettingCustomPresetStruct* preset;
};

SubGhzSetting* subghz_setting_alloc(void) {
    SubGhzSetting* instance = malloc(sizeof(SubGhzSetting));
    FrequencyList_init(instance->frequencies);
    FrequencyList_init(instance->hopper_frequencies);
    instance->preset = malloc(sizeof(SubGhzSettingCustomPresetStruct));
    SubGhzSettingCustomPresetItemArray_init(instance->preset->data);
    return instance;
}

static void subghz_setting_preset_reset(SubGhzSetting* instance) {
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            furi_string_free(item->custom_preset_name);
            free(item->custom_preset_data);
        }
    SubGhzSettingCustomPresetItemArray_reset(instance->preset->data);
}

void subghz_setting_free(SubGhzSetting* instance) {
    furi_check(instance);
    FrequencyList_clear(instance->frequencies);
    FrequencyList_clear(instance->hopper_frequencies);
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            furi_string_free(item->custom_preset_name);
            free(item->custom_preset_data);
        }
    SubGhzSettingCustomPresetItemArray_clear(instance->preset->data);
    free(instance->preset);
    free(instance);
}

static void subghz_setting_load_default_preset(
    SubGhzSetting* instance,
    const char* preset_name,
    const uint8_t* preset_data) {
    furi_assert(instance);
    furi_assert(preset_data);
    uint32_t preset_data_count = 0;
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_push_raw(instance->preset->data);

    item->custom_preset_name = furi_string_alloc();
    furi_string_set(item->custom_preset_name, preset_name);

    while(preset_data[preset_data_count]) {
        preset_data_count += 2;
    }
    preset_data_count += 2;
    item->custom_preset_data_size = sizeof(uint8_t) * preset_data_count + sizeof(uint8_t) * 8;
    item->custom_preset_data = malloc(item->custom_preset_data_size);
    //load preset register + pa table
    memcpy(&item->custom_preset_data[0], &preset_data[0], item->custom_preset_data_size);
}

static void subghz_setting_load_frequencies(FrequencyList_t list, const uint32_t* frequencies) {
    while(*frequencies) {
        FrequencyList_push_back(list, *frequencies);
        frequencies++;
    }
}

static void subghz_setting_load_default_region(SubGhzSetting* instance) {
    furi_assert(instance);

    FrequencyList_reset(instance->frequencies);
    FrequencyList_reset(instance->hopper_frequencies);
    subghz_setting_preset_reset(instance);

    subghz_setting_load_default_preset(
        instance, "AM270", subghz_device_cc1101_preset_ook_270khz_async_regs);
    subghz_setting_load_default_preset(
        instance, "AM650", subghz_device_cc1101_preset_ook_650khz_async_regs);
    subghz_setting_load_default_preset(
        instance, "FM238", subghz_device_cc1101_preset_2fsk_dev2_38khz_async_regs);
    subghz_setting_load_default_preset(
        instance, "FM476", subghz_device_cc1101_preset_2fsk_dev47_6khz_async_regs);
}

// Region check removed
void subghz_setting_load_default(SubGhzSetting* instance) {
    subghz_setting_load_default_region(instance);
}

void subghz_setting_load(SubGhzSetting* instance, const char* file_path) {
    furi_check(instance);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    uint32_t temp_data32;
    bool temp_bool;

    subghz_setting_load_default(instance);

    if(file_path) {
        do {
            if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
                FURI_LOG_I(TAG, "File is not used %s", file_path);
                break;
            }

            if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
                FURI_LOG_E(TAG, "Missing or incorrect header");
                break;
            }

            if((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_SETTING_FILE_TYPE)) &&
               temp_data32 == SUBGHZ_SETTING_FILE_VERSION) {
            } else {
                FURI_LOG_E(TAG, "Type or version mismatch");
                break;
            }

            // Standard frequencies (optional)
            temp_bool = true;
            flipper_format_read_bool(fff_data_file, "Add_standard_frequencies", &temp_bool, 1);
            if(!temp_bool) {
                FURI_LOG_I(TAG, "Skipping standard frequencies");
            } else {
                FURI_LOG_I(TAG, "Adding standard frequencies");
                subghz_setting_load_frequencies(instance->frequencies, subghz_frequency_list);
                subghz_setting_load_frequencies(
                    instance->hopper_frequencies, subghz_hopper_frequency_list);
            }

            // Load frequencies
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            while(flipper_format_read_uint32(
                fff_data_file, "Frequency", (uint32_t*)&temp_data32, 1)) {
                //Todo FL-3535: add a frequency support check depending on the selected radio device
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Frequency loaded %lu", temp_data32);
                    FrequencyList_push_back(instance->frequencies, temp_data32);
                } else {
                    FURI_LOG_E(TAG, "Frequency not supported %lu", temp_data32);
                }
            }

            // Load hopper frequencies
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            while(flipper_format_read_uint32(
                fff_data_file, "Hopper_frequency", (uint32_t*)&temp_data32, 1)) {
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Hopper frequency loaded %lu", temp_data32);
                    FrequencyList_push_back(instance->hopper_frequencies, temp_data32);
                } else {
                    FURI_LOG_E(TAG, "Hopper frequency not supported %lu", temp_data32);
                }
            }

            // Default frequency (optional)
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            if(flipper_format_read_uint32(fff_data_file, "Default_frequency", &temp_data32, 1)) {
                subghz_setting_set_default_frequency(instance, temp_data32);
            }

            // custom preset (optional)
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            furi_string_reset(temp_str);
            while(flipper_format_read_string(fff_data_file, "Custom_preset_name", temp_str)) {
                FURI_LOG_I(TAG, "Custom preset loaded %s", furi_string_get_cstr(temp_str));
                subghz_setting_load_custom_preset(
                    instance, furi_string_get_cstr(temp_str), fff_data_file);
            }

        } while(false);
    }

    furi_string_free(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    if(!FrequencyList_size(instance->frequencies)) {
        FURI_LOG_E(TAG, "Empty static frequency list, loading default ones");
        subghz_setting_load_frequencies(instance->frequencies, subghz_frequency_list);
    }
    if(!FrequencyList_size(instance->hopper_frequencies)) {
        FURI_LOG_E(TAG, "Empty hopper frequency list, loading default ones");
        subghz_setting_load_frequencies(
            instance->hopper_frequencies, subghz_hopper_frequency_list);
    }
}

void subghz_setting_set_default_frequency(SubGhzSetting* instance, uint32_t frequency_to_setup) {
    for
        M_EACH(frequency, instance->frequencies, FrequencyList_t) {
            *frequency &= FREQUENCY_MASK;
            if(*frequency == frequency_to_setup) {
                *frequency |= FREQUENCY_FLAG_DEFAULT;
            }
        }
}

size_t subghz_setting_get_frequency_count(SubGhzSetting* instance) {
    furi_check(instance);
    return FrequencyList_size(instance->frequencies);
}

size_t subghz_setting_get_hopper_frequency_count(SubGhzSetting* instance) {
    furi_check(instance);
    return FrequencyList_size(instance->hopper_frequencies);
}

size_t subghz_setting_get_preset_count(SubGhzSetting* instance) {
    furi_check(instance);
    return SubGhzSettingCustomPresetItemArray_size(instance->preset->data);
}

const char* subghz_setting_get_preset_name(SubGhzSetting* instance, size_t idx) {
    furi_check(instance);
    if(idx >= SubGhzSettingCustomPresetItemArray_size(instance->preset->data)) {
        idx = 0;
    }
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return furi_string_get_cstr(item->custom_preset_name);
}

int subghz_setting_get_inx_preset_by_name(SubGhzSetting* instance, const char* preset_name) {
    furi_check(instance);
    size_t idx = 0;
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            if(strcmp(furi_string_get_cstr(item->custom_preset_name), preset_name) == 0) {
                return idx;
            }
            idx++;
        }
    furi_crash("SubGhz: No name preset.");
    return -1;
}

bool subghz_setting_load_custom_preset(
    SubGhzSetting* instance,
    const char* preset_name,
    FlipperFormat* fff_data_file) {
    furi_check(instance);
    furi_check(preset_name);
    uint32_t temp_data32;
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_push_raw(instance->preset->data);
    item->custom_preset_name = furi_string_alloc();
    furi_string_set(item->custom_preset_name, preset_name);
    do {
        if(!flipper_format_get_value_count(fff_data_file, "Custom_preset_data", &temp_data32))
            break;
        if(!temp_data32 || (temp_data32 % 2)) {
            FURI_LOG_E(TAG, "Integrity error Custom_preset_data");
            break;
        }
        item->custom_preset_data_size = sizeof(uint8_t) * temp_data32;
        item->custom_preset_data = malloc(item->custom_preset_data_size);
        if(!flipper_format_read_hex(
               fff_data_file,
               "Custom_preset_data",
               item->custom_preset_data,
               item->custom_preset_data_size)) {
            FURI_LOG_E(TAG, "Missing Custom_preset_data");
            break;
        }
        return true;
    } while(true);
    return false;
}

bool subghz_setting_delete_custom_preset(SubGhzSetting* instance, const char* preset_name) {
    furi_check(instance);
    furi_check(preset_name);
    SubGhzSettingCustomPresetItemArray_it_t it;
    SubGhzSettingCustomPresetItemArray_it_last(it, instance->preset->data);
    while(!SubGhzSettingCustomPresetItemArray_end_p(it)) {
        SubGhzSettingCustomPresetItem* item = SubGhzSettingCustomPresetItemArray_ref(it);
        if(strcmp(furi_string_get_cstr(item->custom_preset_name), preset_name) == 0) {
            furi_string_free(item->custom_preset_name);
            free(item->custom_preset_data);
            SubGhzSettingCustomPresetItemArray_remove(instance->preset->data, it);
            return true;
        }
        SubGhzSettingCustomPresetItemArray_previous(it);
    }
    return false;
}

uint8_t* subghz_setting_get_preset_data(SubGhzSetting* instance, size_t idx) {
    furi_check(instance);
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return item->custom_preset_data;
}

size_t subghz_setting_get_preset_data_size(SubGhzSetting* instance, size_t idx) {
    furi_check(instance);
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return item->custom_preset_data_size;
}

uint8_t* subghz_setting_get_preset_data_by_name(SubGhzSetting* instance, const char* preset_name) {
    furi_check(instance);
    SubGhzSettingCustomPresetItem* item = SubGhzSettingCustomPresetItemArray_get(
        instance->preset->data, subghz_setting_get_inx_preset_by_name(instance, preset_name));
    return item->custom_preset_data;
}

uint32_t subghz_setting_get_frequency(SubGhzSetting* instance, size_t idx) {
    furi_check(instance);
    if(idx < FrequencyList_size(instance->frequencies)) {
        return (*FrequencyList_get(instance->frequencies, idx)) & FREQUENCY_MASK;
    } else {
        return 0;
    }
}

uint32_t subghz_setting_get_hopper_frequency(SubGhzSetting* instance, size_t idx) {
    furi_check(instance);
    if(idx < FrequencyList_size(instance->hopper_frequencies)) {
        return *FrequencyList_get(instance->hopper_frequencies, idx);
    } else {
        return 0;
    }
}

uint32_t subghz_setting_get_frequency_default_index(SubGhzSetting* instance) {
    furi_check(instance);
    for(size_t i = 0; i < FrequencyList_size(instance->frequencies); i++) {
        uint32_t frequency = *FrequencyList_get(instance->frequencies, i);
        if(frequency & FREQUENCY_FLAG_DEFAULT) {
            return i;
        }
    }
    return 0;
}

uint32_t subghz_setting_get_default_frequency(SubGhzSetting* instance) {
    furi_check(instance);
    return subghz_setting_get_frequency(
        instance, subghz_setting_get_frequency_default_index(instance));
}

uint8_t subghz_setting_customs_presets_to_log(SubGhzSetting* instance) {
    furi_assert(instance);
#ifndef FURI_DEBUG
    FURI_LOG_I(TAG, "Logging loaded presets allow only Debug build");
#else
    uint8_t count = 0;
    FuriString* temp = furi_string_alloc();

    FURI_LOG_I(TAG, "Loaded presets");
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            furi_string_reset(temp);

            for(uint8_t i = 0; i < item->custom_preset_data_size; i++) {
                furi_string_cat_printf(temp, "%02u ", item->custom_preset_data[i]);
            }

            FURI_LOG_I(
                TAG, "%u  -  %s", count + 1, furi_string_get_cstr(item->custom_preset_name));
            FURI_LOG_I(TAG, "  Size: %u", item->custom_preset_data_size);
            FURI_LOG_I(TAG, "  Data: %s", furi_string_get_cstr(temp));

            count++;
        }

    furi_string_free(temp);

    return count;
#endif
    return 0;
}
