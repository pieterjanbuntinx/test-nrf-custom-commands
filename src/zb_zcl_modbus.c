/*
 * ZBOSS Zigbee 3.0
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
 * www.dsr-zboss.com
 * www.dsr-corporation.com
 * All rights reserved.
 *
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* PURPOSE: ZCL Modbus cluster specific commands handling
 */

#define ZB_TRACE_FILE_ID 12087

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "zb_zcl_modbus.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

LOG_MODULE_REGISTER(zcl_modbus, LOG_LEVEL_INF);

K_FIFO_DEFINE(fifo);

static int currentFifoSize = 0;

#define SEND_TIMEOUT  1000
#define MAX_FIFO_SIZE 10

zb_uint8_t gs_modbus_server_received_commands[] = {ZB_ZCL_CLUSTER_ID_MODBUS_SERVER_ROLE_RECEIVED_CMD_LIST};

zb_discover_cmd_list_t gs_modbus_client_cmd_list = {0, NULL, sizeof(gs_modbus_server_received_commands), gs_modbus_server_received_commands};

zb_discover_cmd_list_t gs_modbus_server_cmd_list = {sizeof(gs_modbus_server_received_commands), gs_modbus_server_received_commands, 0, NULL};

static zb_ret_t check_value_modbus_server(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t* value);
static zb_ret_t check_value_modbus_client(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t* value);

zb_bool_t zb_zcl_process_modbus_specific_commands_srv(zb_uint8_t param);
zb_bool_t zb_zcl_process_modbus_specific_commands_cli(zb_uint8_t param);

void                                 zb_zcl_handleModbusCommand(zb_uint8_t param, modbus_cmd_resp_queue_data_t* fifo_data);
void                                 handleNextFifoItem(zb_uint8_t param);
static modbus_cmd_resp_queue_data_t* allocFifoData();
static void                          freeFifoData(modbus_cmd_resp_queue_data_t* fifo_data);

modbus_cmd_resp_queue_data_t fifo_data_arr[MAX_FIFO_SIZE]   = {0};
bool                         fifo_data_usage[MAX_FIFO_SIZE] = {0};

void zb_zcl_modbus_init_server() {
    zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_MODBUS, ZB_ZCL_CLUSTER_SERVER_ROLE, check_value_modbus_server, (zb_zcl_cluster_write_attr_hook_t)NULL, zb_zcl_process_modbus_specific_commands_srv);
}

void zb_zcl_modbus_init_client() {
    zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_MODBUS, ZB_ZCL_CLUSTER_CLIENT_ROLE, check_value_modbus_client, (zb_zcl_cluster_write_attr_hook_t)NULL, zb_zcl_process_modbus_specific_commands_cli);
}

static zb_ret_t check_value_modbus_server(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t* value) {
    zb_ret_t ret = RET_OK;

    ZVUNUSED(endpoint);

    switch (attr_id) {
    case ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID:
        if (ZB_ZCL_ATTR_GET16(value) > ZB_ZCL_MODBUS_CLUSTER_REVISION_MAX) {
            ret = RET_ERROR;
        }
        break;
    default:
        ret = RET_OK;
        break;
    }

    return ret;
}

static zb_ret_t check_value_modbus_client(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t* value) {
    zb_ret_t ret = RET_OK;

    ZVUNUSED(endpoint);

    switch (attr_id) {
    case ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID:
        if (ZB_ZCL_ATTR_GET16(value) > ZB_ZCL_MODBUS_CLUSTER_REVISION_MAX) {
            ret = RET_ERROR;
        }
        break;
    default:
        ret = RET_OK;
        break;
    }

    return ret;
}

static void cb(zb_uint8_t param) {
}

static void json_cmd_handler(zb_uint8_t param, zb_zcl_modbus_addr_t* addr) {
    zb_zcl_parse_status_t status;

    TRACE_MSG(TRACE_ZCL1, "> json_cmd_handler param %i", (FMT__H, param));

    if (status == ZB_ZCL_PARSE_STATUS_SUCCESS) {
        LOG_INF("Received");
    }

end:
    TRACE_MSG(TRACE_ZCL1, "< json_cmd_handler", (FMT__0));
}

zb_bool_t zb_zcl_process_modbus_specific_commands(zb_uint8_t param) {
    zb_zcl_attr_t*           baudrate_desc;
    zb_zcl_modbus_baudrate_t baudrate;
    zb_bool_t                processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t      cmd_info;
    zb_zcl_modbus_addr_t     main_addr;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    TRACE_MSG(TRACE_ZCL1, "> zb_zcl_process_modbus_specific_commands: param %d, cmd %d", (FMT__H_H, param, cmd_info.cmd_id));

    ZB_ASSERT(ZB_ZCL_CLUSTER_ID_MODBUS == cmd_info.cluster_id);
    ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_SRV == cmd_info.cmd_direction);

    main_addr.src_addr                 = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr;
    main_addr.src_endpoint             = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint;
    main_addr.dst_endpoint             = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;
    main_addr.cmd_id                   = cmd_info.cmd_id;
    main_addr.seq_number               = cmd_info.seq_number;
    main_addr.disable_default_response = (zb_bool_t)cmd_info.disable_default_response;
    main_addr.profile_id               = cmd_info.profile_id;

    baudrate_desc = zb_zcl_get_attr_desc_a(main_addr.dst_endpoint, ZB_ZCL_CLUSTER_ID_MODBUS, ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID);

    ZB_ASSERT(baudrate_desc != NULL);

    baudrate = *(zb_zcl_modbus_baudrate_t*)baudrate_desc->data_p;

    TRACE_MSG(TRACE_ZCL1, "baudrate is %i", (FMT__H, baudrate));

    switch (main_addr.cmd_id) {
    case ZB_ZCL_CMD_MODBUS_JSON_COMMAND_REQ_ID:
        json_cmd_handler(param, &main_addr);
        TRACE_MSG(TRACE_ZCL3, "Processed json command", (FMT__0));
        break;

    default:
        processed = ZB_FALSE;
        break;
    }

    TRACE_MSG(TRACE_ZCL1, "< zb_zcl_process_modbus_specific_commands: processed %d", (FMT__D, processed));

    return processed;
}

zb_bool_t zb_zcl_process_modbus_specific_commands_srv(zb_uint8_t param) {
    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param) {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_modbus_server_cmd_list;
        return ZB_TRUE;
    }
    return zb_zcl_process_modbus_specific_commands(param);
}

zb_bool_t zb_zcl_process_modbus_specific_commands_cli(zb_uint8_t param) {
    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param) {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_modbus_client_cmd_list;
        return ZB_TRUE;
    }
    return zb_zcl_process_modbus_specific_commands(param);
}

// TODO finish
static inline void zb_zcl_modbus_send_json_command_cmd_rev1(zb_bufid_t buffer, const zb_addr_u* dst_addr, zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep, zb_uint8_t ep, zb_uint16_t prof_id, zb_uint8_t def_resp, zb_callback_t cb, zb_uint16_t address,
                                                            zb_uint8_t data_length, zb_uint8_t slave_id, zb_uint8_t cmd_id) {
    zb_uint8_t* ptr = ZB_ZCL_START_PACKET_REQ(buffer) ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(ptr, (def_resp)) ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(ptr, ZB_ZCL_GET_SEQ_NUM(), (cmd_id));

    ZB_ZCL_PACKET_PUT_DATA16_VAL(ptr, (address));
    ZB_ZCL_PACKET_PUT_DATA8(ptr, (data_length));
    ZB_ZCL_PACKET_PUT_DATA8(ptr, (slave_id));
    zb_zcl_finish_and_send_packet(buffer, ptr, dst_addr, dst_addr_mode, dst_ep, ep, prof_id, ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, cb);
}

// TODO finish
void zb_zcl_modbus_send_json_command_cmd(zb_bufid_t buffer, const zb_addr_u* dst_addr, zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep, zb_uint8_t ep, zb_uint16_t prof_id, zb_uint8_t def_resp, zb_callback_t cb, zb_uint16_t address,
                                         zb_uint8_t data_length, zb_uint8_t slave_id, zb_uint8_t cmd_id) {
    zb_uint16_t rev;

    TRACE_MSG(TRACE_ZCL3, "> zb_zcl_modbus_send_json_cmd", (FMT__0));

    rev = zb_zcl_get_cluster_rev_by_mode(ZB_ZCL_MODBUS_CLUSTER_REVISION_MAX, dst_addr, dst_addr_mode, dst_ep, ZB_ZCL_CLUSTER_ID_MODBUS, ZB_ZCL_CLUSTER_CLIENT_ROLE, ep);

    TRACE_MSG(TRACE_ZCL3, "rev is %d", (FMT__D, rev));

    switch (rev) {
    case ZB_ZCL_CLUSTER_REV_MIN:
        /* FALLTHROUGH */
    default:
        zb_zcl_modbus_send_json_command_cmd_rev1(buffer, dst_addr, dst_addr_mode, dst_ep, ep, prof_id, def_resp, cb, address, data_length, slave_id, cmd_id);
        break;
    }

    TRACE_MSG(TRACE_ZCL3, "< zb_zcl_modbus_send_json_cmd", (FMT__0));
}