#include "subghz_frequency_analyzer_worker.h"
#include <lib/drivers/cc1101.h>

#include <furi.h>
#include <float_tools.h>

#define TAG "SubghzFrequencyAnalyzerWorker"

#define SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD -97.0f

static const uint8_t subghz_preset_ook_58khz[][2] = {
    {CC1101_MDMCFG4, 0b11110111}, // Rx BW filter is 58.035714kHz
    /* End  */
    {0, 0},
};

static const uint8_t subghz_preset_ook_650khz[][2] = {
    {CC1101_MDMCFG4, 0b00010111}, // Rx BW filter is 650.000kHz
    /* End  */
    {0, 0},
};

struct SubGhzFrequencyAnalyzerWorker {
    FuriThread* thread;

    volatile bool worker_running;
    uint8_t sample_hold_counter;
    FrequencyRSSI frequency_rssi_buf;
    SubGhzSetting* setting;

    float filVal;
    float trigger_level;

    SubGhzFrequencyAnalyzerWorkerPairCallback pair_callback;
    void* context;
};

static void subghz_frequency_analyzer_worker_load_registers(const uint8_t data[][2]) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    size_t i = 0;
    while(data[i][0]) {
        cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, data[i][0], data[i][1]);
        i++;
    }
    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
}

// Improved adaptive exponential moving average for better frequency tracking
static inline __attribute__((always_inline)) uint32_t subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
    SubGhzFrequencyAnalyzerWorker* instance,
    uint32_t newVal) {
    float k;
    float newValFloat = newVal;

    // More responsive filtering based on signal strength difference
    float diff = fabsf(newValFloat - instance->filVal);
    if(diff > 1000000.f) { // Large frequency jumps - very responsive
        k = 0.95f;
    } else if(diff > 500000.f) { // Medium jumps
        k = 0.7f;
    } else if(diff > 100000.f) { // Small jumps
        k = 0.3f;
    } else { // Stable signal - smooth filtering
        k = 0.1f;
    }

    instance->filVal += (newValFloat - instance->filVal) * k;
    return (uint32_t)(instance->filVal + 0.5f); // Round to nearest integer
}

/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_frequency_analyzer_worker_thread(void* context) {
    SubGhzFrequencyAnalyzerWorker* instance = context;

    FrequencyRSSI frequency_rssi = {
        .frequency_coarse = 0, .rssi_coarse = 0, .frequency_fine = 0, .rssi_fine = 0};
    float rssi = 0;
    uint32_t frequency = 0;
    float rssi_temp = 0;
    uint32_t frequency_temp = 0;

    //Start CC1101
    furi_hal_subghz_reset();

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_rx(&furi_hal_spi_bus_handle_subghz);
    cc1101_flush_tx(&furi_hal_spi_bus_handle_subghz);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_IOCFG0, CC1101IocfgHW);
    cc1101_write_reg(&furi_hal_spi_bus_handle_subghz, CC1101_MDMCFG3,
                     0b01111111); // symbol rate
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL2,
        0b00000111); // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAGN_TARGET 42 dB
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL1,
        0b00001000); // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 1000 - Absolute carrier sense threshold disabled
    cc1101_write_reg(
        &furi_hal_spi_bus_handle_subghz,
        CC1101_AGCCTRL0,
        0b00110000); // 00 - No hysteresis, medium asymmetric dead zone, medium gain ; 11 - 64 samples agc; 00 - Normal AGC, 00 - 4dB boundary

    furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);

    while(instance->worker_running) {
        furi_delay_ms(5); // Reduced delay for faster scanning

        float rssi_min = 26.0f;
        float rssi_avg = 0;
        size_t rssi_avg_samples = 0;

        frequency_rssi.rssi_coarse = -127.0f;
        frequency_rssi.rssi_fine = -127.0f;
        furi_hal_subghz_idle();
        subghz_frequency_analyzer_worker_load_registers(subghz_preset_ook_650khz);

        // Streamlined coarse scan with frequency range filtering
        furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
        cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);

        // First stage: coarse scan with optimized frequency selection
        size_t freq_count = subghz_setting_get_frequency_count(instance->setting);
        size_t scan_step = freq_count > 30 ? 2 : 1; // Adaptive step size for performance

        for(size_t i = 0; i < freq_count; i += scan_step) {
            uint32_t current_frequency = subghz_setting_get_frequency(instance->setting, i);
            if(furi_hal_subghz_is_frequency_valid(current_frequency) &&
               (current_frequency >= 300000000 && current_frequency <= 470000000)) {

                frequency = cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, current_frequency);
                cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
                furi_check(cc1101_wait_status_state(
                    &furi_hal_spi_bus_handle_subghz, CC1101StateIDLE, 10000));
                cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);

                furi_delay_us(500); // Further reduced delay for faster scanning

                rssi = furi_hal_subghz_get_rssi();

                rssi_avg += rssi;
                rssi_avg_samples++;

                if(rssi < rssi_min) rssi_min = rssi;

                if(frequency_rssi.rssi_coarse < rssi) {
                    frequency_rssi.rssi_coarse = rssi;
                    frequency_rssi.frequency_coarse = frequency;
                }
            }
        }

        furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);

        FURI_LOG_T(
            TAG,
            "RSSI: avg %f, max %f at %lu, min %f",
            (double)(rssi_avg / rssi_avg_samples),
            (double)frequency_rssi.rssi_coarse,
            frequency_rssi.frequency_coarse,
            (double)rssi_min);

        // Second stage: fine scan with optimized SPI usage
        if(frequency_rssi.rssi_coarse > instance->trigger_level) {
            furi_hal_subghz_idle();
            subghz_frequency_analyzer_worker_load_registers(subghz_preset_ook_58khz);

            furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);
            cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);

            // Optimized fine scan with larger steps for speed
            uint32_t fine_scan_start = MAX(300000000U, frequency_rssi.frequency_coarse - 200000);
            uint32_t fine_scan_end = MIN(470000000U, frequency_rssi.frequency_coarse + 200000);

            for(uint32_t i = fine_scan_start; i < fine_scan_end; i += 25000) { // Increased step size for faster scanning
                if(furi_hal_subghz_is_frequency_valid(i) &&
                   (i >= 300000000 && i <= 470000000)) {
                    frequency = cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, i);
                    cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
                    furi_check(cc1101_wait_status_state(
                        &furi_hal_spi_bus_handle_subghz, CC1101StateIDLE, 10000));
                    cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);

                    furi_delay_us(500); // Further reduced delay for faster scanning

                    rssi = furi_hal_subghz_get_rssi();

                    FURI_LOG_T(TAG, "#:%lu:%f", frequency, (double)rssi);

                    if(frequency_rssi.rssi_fine < rssi) {
                        frequency_rssi.rssi_fine = rssi;
                        frequency_rssi.frequency_fine = frequency;
                    }
                }
            }

            furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
        }

        // Streamlined result delivery with improved logic
        uint32_t detected_frequency = 0;
        float detected_rssi = -127.0f;
        bool has_signal = false;

        if(frequency_rssi.rssi_fine > instance->trigger_level) {
            // Fine scan result
            detected_frequency = frequency_rssi.frequency_fine;
            detected_rssi = frequency_rssi.rssi_fine;
            has_signal = true;
            FURI_LOG_D(TAG, "=:%lu:%f", detected_frequency, (double)detected_rssi);
        } else if(frequency_rssi.rssi_coarse > instance->trigger_level) {
            // Coarse scan result
            detected_frequency = frequency_rssi.frequency_coarse;
            detected_rssi = frequency_rssi.rssi_coarse;
            has_signal = true;
            FURI_LOG_D(TAG, "~:%lu:%f", detected_frequency, (double)detected_rssi);
        }

        if(has_signal) {
            instance->sample_hold_counter = 20;
            rssi_temp = detected_rssi;
            frequency_temp = detected_frequency;

            // Apply adaptive filtering for stable frequency tracking
            if(!float_is_equal(instance->filVal, 0.f)) {
                detected_frequency = subghz_frequency_analyzer_worker_expRunningAverageAdaptive(
                    instance, detected_frequency);
            }

            // Deliver callback with filtered frequency
            if(instance->pair_callback) {
                instance->pair_callback(instance->context, detected_frequency, detected_rssi, true);
            }
        } else {
            // No signal detected - handle sample hold decay
            if(instance->sample_hold_counter > 0) {
                instance->sample_hold_counter--;
                if(instance->sample_hold_counter == 15) { // Earlier notification for better UX
                    if(instance->pair_callback) {
                        instance->pair_callback(instance->context, frequency_temp, rssi_temp, false);
                    }
                }
            } else {
                instance->filVal = 0; // Reset filter when no signal
                if(instance->pair_callback) {
                    instance->pair_callback(instance->context, 0, 0, false);
                }
            }
        }
    }

    //Stop CC1101
    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();

    return 0;
}

SubGhzFrequencyAnalyzerWorker* subghz_frequency_analyzer_worker_alloc(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzerWorker* instance = malloc(sizeof(SubGhzFrequencyAnalyzerWorker));

    instance->thread = furi_thread_alloc_ex(
        "SubGhzFAWorker", 3072, subghz_frequency_analyzer_worker_thread, instance);
    SubGhz* subghz = context;
    instance->setting = subghz_txrx_get_setting(subghz->txrx);
    instance->trigger_level = subghz->last_settings->frequency_analyzer_trigger;
    //instance->trigger_level = SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD;
    return instance;
}

void subghz_frequency_analyzer_worker_free(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);

    furi_thread_free(instance->thread);
    free(instance);
}

void subghz_frequency_analyzer_worker_set_pair_callback(
    SubGhzFrequencyAnalyzerWorker* instance,
    SubGhzFrequencyAnalyzerWorkerPairCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(context);
    instance->pair_callback = callback;
    instance->context = context;
}

void subghz_frequency_analyzer_worker_start(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    instance->worker_running = true;

    furi_thread_start(instance->thread);
}

void subghz_frequency_analyzer_worker_stop(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;

    furi_thread_join(instance->thread);
}

bool subghz_frequency_analyzer_worker_is_running(SubGhzFrequencyAnalyzerWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}

void subghz_frequency_analyzer_worker_set_trigger_level(
    SubGhzFrequencyAnalyzerWorker* instance,
    float value) {
    instance->trigger_level = value;
}

float subghz_frequency_analyzer_worker_get_trigger_level(SubGhzFrequencyAnalyzerWorker* instance) {
    return instance->trigger_level;
}

uint32_t subghz_frequency_analyzer_get_nearest_frequency(
    SubGhzFrequencyAnalyzerWorker* instance,
    uint32_t input) {
    uint32_t prev_freq = 0;
    uint32_t result = 0;
    uint32_t current;

    // Ensure input is within 300-470 MHz range
    if(input < 300000000) input = 300000000;
    if(input > 470000000) input = 470000000;

    for(size_t i = 0; i < subghz_setting_get_frequency_count(instance->setting); i++) {
        current = subghz_setting_get_frequency(instance->setting, i);
        if(current == 0 || current < 300000000 || current > 470000000) {
            continue;
        }
        if(current == input) {
            result = current;
            break;
        }
        if(current > input && prev_freq < input) {
            if(current - input < input - prev_freq) {
                result = current;
            } else {
                result = prev_freq;
            }
            break;
        }
        prev_freq = current;
    }

    // Fallback to default frequency if no valid frequency found
    if(result == 0) {
        result = 433920000; // Default frequency within range
    }

    return result;
}
