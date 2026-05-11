#ifndef __toggle_H__
#define __toggle_H__

#include <stdbool.h>

// Opaque pointer type for the toggle instance
typedef struct {
    bool state;
    int gpio_num;
} gpio_toggle_t;

/**
 * @brief Initializes the toggle on a specific GPIO pin.
 *
 * @param gpio_num The GPIO number to use for the toggle.
 * @return A handle to the toggle instance, or NULL if initialization fails.
 */
esp_err_t toggle_init(int gpio_num);

/**
 * @brief Sets the state of the toggle.
 *
 * @param handle The handle to the toggle instance.
 * @param state The desired state (true for ON, false for OFF).
 */
void toggle_set_state(bool state);

/**
 * @brief Deinitializes the toggle and frees resources.
 *
 * @param handle The handle to the toggle instance.
 */
void toggle_deinit();

bool toggle_get_state();

#endif // toggle_H