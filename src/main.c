/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @brief Zigbee application template.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "zb_range_extender.h"
#include <ram_pwrdn.h>
#include <zb_nrf_platform.h>
#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee/zigbee_error_handler.h>

#include <zigbee/zigbee_app_utils.h>

#include "zb_zcl_modbus.h"

#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"

#define ZIGBEE_MANUFACTURER_CODE ZB_ZCL_MANUF_CODE_INVALID

#if CONFIG_ZIGBEE_FOTA
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/sys/reboot.h>
#include <zigbee/zigbee_fota.h>

/* LED indicating OTA Client Activity. */
#define OTA_ACTIVITY_LED LED_RED
#endif /* CONFIG_ZIGBEE_FOTA */

/* Device endpoint, used to receive ZCL commands. */
#define TEST_EP_ENDPOINT 0x01

/* LED indicating that device successfully joined Zigbee network. */
#define ZIGBEE_NETWORK_STATE_LED LED_BLUE

/* LED used for device identification. */
#define IDENTIFY_LED LED_GREEN

/* Button used to enter the Identify mode. */
#define IDENTIFY_MODE_BUTTON HALL_IN_MSK

/* Button to start Factory Reset */
#define FACTORY_RESET_BUTTON IDENTIFY_MODE_BUTTON

#define MODBUS_SEND_MODES_AND_USER_SETPOINTS_INTERVAL_MS 2000
#define MODBUS_SEND_ACTUAL_PROCESS_VALUES_INTERVAL_MS 4000
#define MODBUS_SEND_EXTERNAL_PROCESS_VALUES_INTERVAL_MS 4000

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static bool network_led_state = false;

/* Main application customizable context.
 * Stores all settings and static values.
 */
typedef struct {
  zb_zcl_basic_attrs_ext_t basic_attr;
  zb_zcl_identify_attrs_t identify_attr;
  zb_zcl_modbus_attrs_t modbus_attr;
} zb_device_ctx_t;

/* Zigbee device application context storage. */
static zb_device_ctx_t dev_ctx;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list,
                                    &dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(
    basic_attr_list, &dev_ctx.basic_attr.zcl_version,
    &dev_ctx.basic_attr.app_version, &dev_ctx.basic_attr.stack_version,
    &dev_ctx.basic_attr.hw_version, &dev_ctx.basic_attr.mf_name,
    &dev_ctx.basic_attr.model_id, &dev_ctx.basic_attr.date_code,
    &dev_ctx.basic_attr.power_source, &dev_ctx.basic_attr.location_id,
    &dev_ctx.basic_attr.ph_env, &dev_ctx.basic_attr.sw_ver);

ZB_DECLARE_RANGE_EXTENDER_CLUSTER_LIST(range_extender_clusters, basic_attr_list,
                                       identify_attr_list);

ZB_DECLARE_RANGE_EXTENDER_EP(test_ep, TEST_EP_ENDPOINT,
                             range_extender_clusters);

#define MODBUS_CLUSTER_ENDPOINT 0x02

// add Modbus cluster
ZB_ZCL_DECLARE_MODBUS_ATTRIB_LIST(modbus_attr_list,
                                  &dev_ctx.modbus_attr.baudrate);
zb_zcl_cluster_desc_t clusters_test[] = {
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_MODBUS,
                        ZB_ZCL_ARRAY_SIZE(modbus_attr_list, zb_zcl_attr_t),
                        (modbus_attr_list), ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID),
};

ZB_AF_SIMPLE_DESC_TYPE(1, 1)
simple_desc_test = {MODBUS_CLUSTER_ENDPOINT,
                    ZB_AF_HA_PROFILE_ID,
                    0xF003, // TODO change
                    1,      // TODO change
                    0,
                    0,
                    1,
                    {
                        ZB_ZCL_CLUSTER_ID_MODBUS,
                    }};
ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info_test, 0);
ZB_AF_DECLARE_ENDPOINT_DESC(
    device_ep, MODBUS_CLUSTER_ENDPOINT, ZB_AF_HA_PROFILE_ID, 0, NULL,
    ZB_ZCL_ARRAY_SIZE(clusters_test, zb_zcl_cluster_desc_t), clusters_test,
    (zb_af_simple_desc_1_1_t *)&simple_desc_test, 0, NULL, 0, NULL);

#ifndef CONFIG_ZIGBEE_FOTA
ZB_AF_START_DECLARE_ENDPOINT_LIST(ep_list_test_ep_ctx)
&test_ep, &device_ep, ZB_AF_FINISH_DECLARE_ENDPOINT_LIST;
ZBOSS_DECLARE_DEVICE_CTX(test_ep_ctx, ep_list_test_ep_ctx,
                         (ZB_ZCL_ARRAY_SIZE(ep_list_test_ep_ctx,
                                            zb_af_endpoint_desc_t *)));
#else

#if TEST_EP_ENDPOINT == CONFIG_ZIGBEE_FOTA_ENDPOINT
#error "Device and Zigbee OTA endpoints should be different."
#endif

extern zb_af_endpoint_desc_t zigbee_fota_client_ep;

ZB_AF_START_DECLARE_ENDPOINT_LIST(ep_list_test_ep_ctx)
&test_ep, &device_ep, &zigbee_fota_client_ep,
    ZB_AF_FINISH_DECLARE_ENDPOINT_LIST;
ZBOSS_DECLARE_DEVICE_CTX(test_ep_ctx, ep_list_test_ep_ctx,
                         (ZB_ZCL_ARRAY_SIZE(ep_list_test_ep_ctx,
                                            zb_af_endpoint_desc_t *)));
#endif /* CONFIG_ZIGBEE_FOTA */

int set_pascal_string(char *str, char *dest, size_t max_length) {
  size_t str_length = strlen(str);

  if (str_length > max_length) {
    return 1;
  }
  dest[0] = (char)str_length;
  memcpy(&dest[1], str, str_length);
  return 0;
}

/**@brief Function for initializing all clusters attributes. */
static void app_clusters_attr_init(void) {
  /* Basic cluster attributes data */
  dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
  dev_ctx.basic_attr.app_version =
      ZB_ZCL_BASIC_APPLICATION_VERSION_DEFAULT_VALUE; // TODO set in production
  dev_ctx.basic_attr.stack_version =
      ZB_ZCL_BASIC_STACK_VERSION_DEFAULT_VALUE; // TODO set in production
  dev_ctx.basic_attr.hw_version =
      ZB_ZCL_BASIC_HW_VERSION_DEFAULT_VALUE; // TODO set in production
  dev_ctx.modbus_attr.baudrate = ZB_ZCL_MODBUS_BAUDRATE_19200;

  set_pascal_string("TEST NV", dev_ctx.basic_attr.mf_name,
                    sizeof(dev_ctx.basic_attr.mf_name));
  set_pascal_string("test", dev_ctx.basic_attr.model_id,
                    sizeof(dev_ctx.basic_attr.model_id));
  set_pascal_string(
      "17022023", dev_ctx.basic_attr.date_code,
      sizeof(dev_ctx.basic_attr.date_code)); // TODO set in production
  dev_ctx.basic_attr.power_source = ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE;
  set_pascal_string(
      "test123", dev_ctx.basic_attr.sw_ver,
      sizeof(dev_ctx.basic_attr.sw_ver)); // TODO set in production
  set_pascal_string("Not set", dev_ctx.basic_attr.location_id,
                    sizeof(dev_ctx.basic_attr.location_id));
  dev_ctx.basic_attr.ph_env = ZB_ZCL_BASIC_ENV_UNSPECIFIED;

  /* Identify cluster attributes data. */
  dev_ctx.identify_attr.identify_time =
      ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
}

/**@brief Function to toggle the identify LED
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void toggle_identify_led(zb_bufid_t bufid) {
  static int blink_status;

  if ((++blink_status) % 2) {
    gpio_set_led_off(ZIGBEE_NETWORK_STATE_LED);
    gpio_set_led_on(IDENTIFY_LED);
  } else {
    gpio_set_led_off(IDENTIFY_LED);
    gpio_set_led(ZIGBEE_NETWORK_STATE_LED, network_led_state ? 1 : 0);
  }

  ZB_SCHEDULE_APP_ALARM(toggle_identify_led, bufid,
                        ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
}

/**@brief Function to handle identify notification events on the first endpoint.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void identify_cb(zb_bufid_t bufid) {
  zb_ret_t zb_err_code;

  if (bufid) {
    /* Schedule a self-scheduling function that will toggle the LED */
    LOG_INF("Enter identify mode");
    ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
    ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
  } else {
    /* Cancel the toggling function alarm and turn off LED */
    LOG_INF("Cancel identify mode");
    zb_err_code =
        ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
    ZVUNUSED(zb_err_code);

    gpio_set_led(IDENTIFY_LED, 0);
    LOG_INF("cancel identify led state: %i\n", network_led_state);
    gpio_set_led(ZIGBEE_NETWORK_STATE_LED, network_led_state ? 1 : 0);
  }
}

/**@brief Starts identifying the device.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void start_identifying(zb_bufid_t bufid) {
  ZVUNUSED(bufid);

  if (ZB_JOINED()) {
    /* Check if endpoint is in identifying mode,
     * if not put desired endpoint in identifying mode.
     */
    if (dev_ctx.identify_attr.identify_time ==
        ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {

      zb_ret_t zb_err_code = zb_bdb_finding_binding_target(TEST_EP_ENDPOINT);

      if (zb_err_code == RET_OK) {
        LOG_INF("Enter identify mode");
      } else if (zb_err_code == RET_INVALID_STATE) {
        LOG_WRN("RET_INVALID_STATE - Cannot enter identify mode");
      } else {
        ZB_ERROR_CHECK(zb_err_code);
      }
    } else {
      LOG_INF("Cancel identify mode");
      zb_bdb_finding_binding_target_cancel();
    }
  } else {
    LOG_WRN("Device not in a network - cannot enter identify mode");
  }
}

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons
 *                            that have changed their state.
 */
static void button_changed(uint32_t button_state, uint32_t has_changed) {
  if (button_state) {
    gpio_set_led_off(ZIGBEE_NETWORK_STATE_LED);
    gpio_set_led_on(LED_RED);
    LOG_INF("Hall sensor activated!");
  } else {
    gpio_set_led_off(LED_RED);
    gpio_set_led(ZIGBEE_NETWORK_STATE_LED, network_led_state ? 1 : 0);
    LOG_INF("Hall sensor deactivated!");
  }

  if (IDENTIFY_MODE_BUTTON & has_changed) {
    if (IDENTIFY_MODE_BUTTON & button_state) {
      /* Button changed its state to pressed */
    } else {
      /* Button changed its state to released */
      if (was_factory_reset_done()) {
        /* The long press was for Factory Reset */
        LOG_DBG("After Factory Reset - ignore button release");
      } else {
        /* Button released before Factory Reset */

        // /* Start identification mode */
        ZB_SCHEDULE_APP_CALLBACK(start_identifying, 0);
      }
    }
  }

  check_factory_reset_button(button_state, has_changed);
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void) {
  int err;

  err = gpio_buttons_init(button_changed);
  if (err) {
    LOG_ERR("Cannot init buttons (err: %d)", err);
  }

  err = gpio_leds_init();
  if (err) {
    LOG_ERR("Cannot init LEDs (err: %d)", err);
  }
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid) {

  /* Update network status LED. */
  zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

#ifdef CONFIG_ZIGBEE_FOTA
  /* Pass signal to the OTA client implementation. */
  zigbee_fota_signal_handler(bufid);
#endif /* CONFIG_ZIGBEE_FOTA */

  /* No application-specific behavior is required.
   * Call default signal handler.
   */
  ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

  zb_zdo_app_signal_hdr_t *p_sg_p = NULL;
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &p_sg_p);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);

  switch (sig) {
  case ZB_BDB_SIGNAL_DEVICE_REBOOT:
  /* fall-through */
  case ZB_BDB_SIGNAL_STEERING:
    if (status == RET_OK) {
      network_led_state = true;
    } else {
      network_led_state = false;
    }
    break;

  case ZB_ZDO_SIGNAL_LEAVE:
    network_led_state = false;
    break;

  default:
    break;
  }

  static bool prev = false;

  prev = network_led_state;

  /* All callbacks should either reuse or free passed buffers.
   * If bufid == 0, the buffer is invalid (not passed).
   */
  if (bufid) {
    zb_buf_free(bufid);
  }
}

#ifdef CONFIG_ZIGBEE_FOTA
static void confirm_image(void) {
  if (!boot_is_img_confirmed()) {
    int ret = boot_write_img_confirmed();

    if (ret) {
      LOG_ERR("Couldn't confirm image: %d", ret);
    } else {
      LOG_INF("Marked image as OK");
    }
  }
}

static void ota_evt_handler(const struct zigbee_fota_evt *evt) {
  switch (evt->id) {
  case ZIGBEE_FOTA_EVT_PROGRESS:
    gpio_set_led(OTA_ACTIVITY_LED, evt->dl.progress % 2);
    break;

  case ZIGBEE_FOTA_EVT_FINISHED:
    LOG_INF("Reboot application.");
    /* Power on unused sections of RAM to allow MCUboot to use it. */
    if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
      power_up_unused_ram();
    }

    sys_reboot(SYS_REBOOT_COLD);
    break;

  case ZIGBEE_FOTA_EVT_ERROR:
    LOG_ERR("OTA image transfer failed.");
    break;

  default:
    break;
  }
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static void zcl_device_cb(zb_bufid_t bufid) {
  zb_zcl_device_callback_param_t *device_cb_param =
      ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

  if (device_cb_param->device_cb_id == ZB_ZCL_OTA_UPGRADE_VALUE_CB_ID) {
    zigbee_fota_zcl_cb(bufid);
  } else {
    device_cb_param->status = RET_NOT_IMPLEMENTED;
  }
}
#endif

void main(void) {
  LOG_INF("Starting ventilation unit...");

  /* Initialize */
  configure_gpio();
  register_factory_reset_button(FACTORY_RESET_BUTTON);

#ifdef CONFIG_ZIGBEE_FOTA
  /* Initialize Zigbee FOTA download service. */
  zigbee_fota_init(ota_evt_handler);

  /* Mark the current firmware as valid. */
  confirm_image();

  /* Register callback for handling ZCL commands. */
  ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);
#endif /* CONFIG_ZIGBEE_FOTA */

  /* Register device context (endpoints). */
  ZB_AF_REGISTER_DEVICE_CTX(&test_ep_ctx);

  app_clusters_attr_init();

  /* Register handlers to identify notifications */
  ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(TEST_EP_ENDPOINT, identify_cb);

#ifdef CONFIG_ZIGBEE_FOTA
  ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(CONFIG_ZIGBEE_FOTA_ENDPOINT,
                                          identify_cb);
#endif /* CONFIG_ZIGBEE_FOTA */

  // uint8_t install_code[18] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
  // 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x52, 0x8F};
  // LOG_INF("\n\n\nerr install_code: %i\n\n\n", zb_secur_ic_set(ZB_IC_TYPE_128,
  // install_code));

  // zb_set_installcode_policy(true);

  /* Start Zigbee default thread */
  zigbee_enable();

  LOG_INF("Zigbee application template started");

  zb_ieee_addr_t ieee_address;
  zb_get_long_address(ieee_address);
  LOG_INF("Current address: %02x%02x%02x%02x%02x%02x%02x%02x\n",
          ieee_address[7], ieee_address[6], ieee_address[5], ieee_address[4],
          ieee_address[3], ieee_address[2], ieee_address[1], ieee_address[0]);

  int count = 0;
  while (1) {
    k_msleep(1000);
  }
}
