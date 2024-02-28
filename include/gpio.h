/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __GPIO__
#define __GPIO__

/** @file gpio_buttons_and_leds.h
 * @brief Module for handling buttons and LEDs on Nordic DKs.
 * @defgroup gpio_buttons_and_leds DK buttons and LEDs
 * @{
 */

#include <zephyr/types.h>
#include <zephyr/sys/slist.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _PINNUM(port, bit) ((port)*32 + (bit))

#define GPIO_HALL_IN   _PINNUM(0, 5)
#define GPIO_LED_RED   _PINNUM(1, 5)
#define GPIO_LED_GREEN _PINNUM(1, 2)
#define GPIO_LED_BLUE  _PINNUM(0, 12)

#define GPIO_CB_RXD _PINNUM(1, 3)
#define GPIO_CB_TXD _PINNUM(1, 4)
#define GPIO_BM_IRQ _PINNUM(1, 8)
#define GPIO_BM_PF  _PINNUM(1, 6)

#define NO_LEDS_MSK (0)
#define LED_RED     0
#define LED_GREEN   2
#define LED_BLUE    1

#define LED_RED_MSK   BIT(LED_RED)
#define LED_GREEN_MSK BIT(LED_GREEN)
#define LED_BLUE_MSK  BIT(LED_BLUE)
#define ALL_LEDS_MSK  (LED_RED_MSK | LED_GREEN_MSK | LED_BLUE_MSK)

#define NO_BTNS_MSK  (0)
#define HALL_IN      0
#define HALL_IN_MSK  BIT(HALL_IN)
#define ALL_BTNS_MSK (HALL_IN_MSK)

#define ENABLE_DYNAMIC_BUTTON_HANDLERS 0
#define GPIO_BUTTON_SCAN_INTERVAL_MS   10

/**
 * @typedef button_handler_t
 * @brief Callback that is executed when a button state change is detected.
 *
 * @param button_state Bitmask of button states.
 * @param has_changed Bitmask that shows which buttons have changed.
 */
typedef void (*button_handler_t)(uint32_t button_state, uint32_t has_changed);

/** Button handler list entry. */
struct button_handler {
    button_handler_t cb;   /**< Callback function. */
    sys_snode_t      node; /**< Linked list node, for internal use. */
};

/** @brief Initialize the library to control the LEDs.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_leds_init(void);

/** @brief Initialize the library to read the button state.
 *
 *  @param  button_handler Callback handler for button state changes.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_buttons_init(button_handler_t button_handler);

/** @brief Add a dynamic button handler callback.
 *
 * In addition to the button handler function passed to
 * @ref gpio_buttons_init, any number of button handlers can be added and removed
 * at runtime.
 *
 * @param[in] handler Handler structure. Must point to statically allocated
 * memory.
 */
void gpio_button_handler_add(struct button_handler* handler);

/** @brief Remove a dynamic button handler callback.
 *
 * @param[in] handler Handler to remove.
 *
 * @retval 0 Successfully removed the handler.
 * @retval -ENOENT This button handler was not present.
 */
int gpio_button_handler_remove(struct button_handler* handler);

/** @brief Read current button states.
 *
 *  @param button_state Bitmask of button states.
 *  @param has_changed Bitmask that shows which buttons have changed.
 */
void gpio_read_buttons(uint32_t* button_state, uint32_t* has_changed);

/** @brief Get current button state from internal variable.
 *
 *  @return Bitmask of button states.
 */
uint32_t gpio_get_buttons(void);

/** @brief Set value of LED pins as specified in one bitmask.
 *
 *  @param  leds Bitmask that defines which LEDs to turn on and off.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_set_leds(uint32_t leds);

/** @brief Set value of LED pins as specified in two bitmasks.
 *
 *  @param  leds_on_mask  Bitmask that defines which LEDs to turn on.
 *                        If this bitmask overlaps with @p leds_off_mask,
 *                        @p leds_on_mask has priority.
 *
 *  @param  leds_off_mask Bitmask that defines which LEDs to turn off.
 *                        If this bitmask overlaps with @p leds_on_mask,
 *                        @p leds_on_mask has priority.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_set_leds_state(uint32_t leds_on_mask, uint32_t leds_off_mask);

/** @brief Set a single LED value.
 *
 *  This function turns a single LED on or off.
 *
 *  @param led_idx Index of the LED.
 *  @param val     Value for the LED: 1 - turn on, 0 - turn off
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 *
 *  @sa gpio_set_led_on, gpio_set_led_off
 */
int gpio_set_led(uint8_t led_idx, uint32_t val);

/** @brief Turn a single LED on.
 *
 *  @param led_idx Index of the LED.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_set_led_on(uint8_t led_idx);

/** @brief Turn a single LED off.
 *
 *  @param led_idx Index of the LED.
 *
 *  @retval 0           If the operation was successful.
 *                      Otherwise, a (negative) error code is returned.
 */
int gpio_set_led_off(uint8_t led_idx);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* __GPIO__ */
