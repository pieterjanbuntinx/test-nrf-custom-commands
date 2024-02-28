#ifndef ZB_ZCL_MODBUS_H
#define ZB_ZCL_MODBUS_H 1

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include "zcl/zb_zcl_common.h"
#include "zcl/zb_zcl_commands.h"

/** @cond DOXYGEN_ZCL_SECTION */

/** @addtogroup ZB_ZCL_MODBUS
 *  @{
 *    @details
 *    All commands in the cluster have only request form, and carry no payload.
 */

/* Cluster ZB_ZCL_CLUSTER_ID_MODBUS */

#define ZB_ZCL_CLUSTER_ID_MODBUS 0xFC00

/*! @name Modbus cluster attributes
    @{
*/

/*! @brief Modbus cluster attribute identifiers
 */
enum zb_zcl_modbus_attr_e
{
    /*! @brief internal baudrate */
    ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID = 0x0000,
};

/**
 * @brief Modbus baudrate attribute
 */
typedef enum
{
    ZB_ZCL_MODBUS_BAUDRATE_9600   = 0,
    ZB_ZCL_MODBUS_BAUDRATE_19200  = 1,
    ZB_ZCL_MODBUS_BAUDRATE_115200 = 2,
} zb_zcl_modbus_baudrate_t;

/** @brief Default value for Modbus cluster revision global attribute */
#define ZB_ZCL_MODBUS_CLUSTER_REVISION_DEFAULT ((zb_uint16_t)0x0001u)

/** @brief Maximal value for implemented modbus cluster revision global attribute */
#define ZB_ZCL_MODBUS_CLUSTER_REVISION_MAX ZB_ZCL_MODBUS_CLUSTER_REVISION_DEFAULT

/** @brief Baudrate attribute minimum value */
#define ZB_ZCL_MODBUS_BAUDRATE_MIN_VALUE ((zb_uint8_t)ZB_ZCL_MODBUS_BAUDRATE_9600)

/** @brief Baudrate attribute maximum value */
#define ZB_ZCL_MODBUS_BAUDRATE_MAX_VALUE ((zb_uint8_t)ZB_ZCL_MODBUS_BAUDRATE_115200)

/** @brief Baudrate attribute default value */
#define ZB_ZCL_MODBUS_BAUDRATE_DEFAULT_VALUE ((zb_uint8_t)ZB_ZCL_MODBUS_BAUDRATE_19200)

/*!
  @brief Declare attribute list for Modbus cluster
  @param attr_list - attribute list name
  @param baudrate - pointer to variable to store baudrate attribute value
*/
#define ZB_ZCL_DECLARE_MODBUS_ATTRIB_LIST(attr_list, baudrate)                                                                                                                                                                                           \
    ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_MODBUS)                                                                                                                                                                          \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID, (baudrate))                                                                                                                                                                                     \
    ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/*! @} */ /* Modbus cluster attributes */

/** @cond internals_doc */
/*! @name Modbus cluster internals
    Internal structures for Modbus cluster
    @internal
    @{
*/

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID(data_ptr)                                                                                                                                                                                  \
    { ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID, ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ZB_ZCL_ATTR_ACCESS_READ_WRITE | ZB_ZCL_ATTR_MANUF_SPEC, (void*)data_ptr }

/** @internal Structure of addr variables for register commands
 */
typedef struct zb_zcl_modbus_addr_s {
    zb_uint16_t src_addr;
    zb_uint8_t  src_endpoint;
    zb_uint8_t  dst_endpoint;
    zb_uint8_t  cmd_id;
    zb_uint8_t  seq_number;
    zb_bool_t   disable_default_response;
    zb_uint16_t profile_id;
} zb_zcl_modbus_addr_t;

/** @see Modbus Exception responses */
typedef enum
{
    ZB_ZCL_MODBUS_EXCP_ILLEGAL_FUNC,
    ZB_ZCL_MODBUS_EXCP_ILLEGAL_DATA_ADDR,
    ZB_ZCL_MODBUS_EXCP_ILLEGAL_DATA_VALUE,
    ZB_ZCL_MODBUS_EXCP_SERVER_DEV_FAIL,
    ZB_ZCL_MODBUS_EXCP_ACK,
    ZB_ZCL_MODBUS_EXCP_SERVER_DEV_BUSY,
    ZB_ZCL_MODBUS_EXCP_NACK,
    ZB_ZCL_MODBUS_EXCP_MEM_PAIR_ERR,
    ZB_ZCL_MODBUS_EXCP_GATE_PATH_UNAVAILABLE,
    ZB_ZCL_MODBUS_EXCP_GATE_TARGET_FAILED_TO_RESPOND,
} zb_zcl_modbus_exception_t;

typedef enum
{
    ZB_ZCL_MODBUS_RESP_STATUS_OK,
    ZB_ZCL_MODBUS_RESP_STATUS_MODBUS_ERROR,
    ZB_ZCL_MODBUS_RESP_STATUS_UNKNOWN_ERROR,
} zb_zcl_modbus_resp_status_t;

/*! Number of attributes mandatory for reporting in Modbus cluster */
#define ZB_ZCL_MODBUS_REPORT_ATTR_COUNT 0

/*! @} */ /* Modbus cluster internals */
/*! @}
 *  @endcond */ /* internals_doc */

/*! @name Modbus cluster commands
    @{
*/

/*! @brief Modbus cluster command identifiers
 */
enum zb_zcl_modbus_cmd_req_e
{
    ZB_ZCL_CMD_MODBUS_JSON_COMMAND_REQ_ID = 0xF1,
};

enum zb_zcl_modbus_cmd_resp_e
{
    ZB_ZCL_CMD_MODBUS_JSON_COMMAND_RESP_ID = 0xF2,
};

/** @cond internals_doc */
/* Modbus cluster commands list : only for information - do not modify */
#define ZB_ZCL_CLUSTER_ID_MODBUS_SERVER_ROLE_GENERATED_CMD_LIST

#define ZB_ZCL_CLUSTER_ID_MODBUS_CLIENT_ROLE_RECEIVED_CMD_LIST ZB_ZCL_CLUSTER_ID_MODBUS_SERVER_ROLE_GENERATED_CMD_LIST

#define ZB_ZCL_CLUSTER_ID_MODBUS_CLIENT_ROLE_GENERATED_CMD_LIST ZB_ZCL_CMD_MODBUS_JSON_COMMAND_REQ_ID

#define ZB_ZCL_CLUSTER_ID_MODBUS_SERVER_ROLE_RECEIVED_CMD_LIST ZB_ZCL_CLUSTER_ID_MODBUS_CLIENT_ROLE_GENERATED_CMD_LIST

#define ZB_ZCL_MB_CMD_MAX_STRING_LENGTH 82

/*! @}
 *  @endcond */ /* internals_doc */

/************************** Modbus cluster command definitions ****************************/

/******** Command json command ********/

/*! @brief Structured representation of json command command payload */
typedef ZB_PACKED_PRE struct zb_zcl_modbus_json_command_req_s {
    zb_uint8_t len;
    zb_char_t  data[ZB_ZCL_MB_CMD_MAX_STRING_LENGTH + 1]; // 1 extra for delimiter
} ZB_PACKED_STRUCT zb_zcl_modbus_json_command_req_t;

/** @brief Json command payload length macro */
#define ZB_ZCL_MODBUS_JSON_COMMAND_REQ_MAX_PAYLOAD_LEN sizeof(zb_zcl_modbus_json_command_req_t)

/*! @brief Send json command
    @param buffer - to put packet to
    @param dst_addr - address to send packet to
    @param dst_addr_mode - addressing mode
    @param dst_ep - destination endpoint
    @param ep - sending endpoint
    @param prof_id - profile identifier
    @param def_resp - enable/disable default response
    @param cb - callback for getting command send status
    @param address - address value
    @param data_length - Data length value
    @param slave_id - Slave_id value
    @param cmd_id - command id
*/
void zb_zcl_modbus_send_json_command_cmd(zb_bufid_t buffer, const zb_addr_u* dst_addr, zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep, zb_uint8_t ep, zb_uint16_t prof_id, zb_uint8_t def_resp, zb_callback_t cb, zb_uint16_t address,
                                         zb_uint8_t data_length, zb_uint8_t slave_id, zb_uint8_t cmd_id);

/** @internal Macro for json command */
#define ZB_ZCL_MODBUS_SEND_JSON_COMMAND_CMD(buffer, addr, dst_addr_mode, dst_ep, ep, prfl_id, def_resp, cb, address, data_length, slave_id, cmd_id)                                                                                                      \
    { zb_zcl_modbus_send_json_command_cmd(buffer, ZB_ADDR_U_CAST(addr), dst_addr_mode, dst_ep, ep, prfl_id, def_resp, cb, address, data_length, slave_id, cmd_id); }

/** @internal Macro for getting json command */
#define ZB_ZCL_MODBUS_GET_JSON_COMMAND_CMD(buf, req, status)                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                                    \
        zb_zcl_modbus_json_command_req_t* modbus_cmd_ptr = (zb_zcl_modbus_json_command_req_t*)zb_buf_begin(buf);                                                                                                                                         \
        status                                           = ZB_ZCL_PARSE_STATUS_FAILURE;                                                                                                                                                                  \
        if (zb_buf_len(buf) >= (zb_uint_t)(modbus_cmd_ptr->len)) {                                                                                                                                                                                       \
            req.len = modbus_cmd_ptr->len;                                                                                                                                                                                                               \
            ZB_MEMSET(req.data, 0, sizeof(req.data));                                                                                                                                                                                                    \
            ZB_MEMCPY(req.data, ((zb_uint8_t*)modbus_cmd_ptr->data), modbus_cmd_ptr->len);                                                                                                                                                               \
            status = ZB_ZCL_PARSE_STATUS_SUCCESS;                                                                                                                                                                                                        \
        }                                                                                                                                                                                                                                                \
    }

/*! @brief Send json command
    @param buffer - to put packet to
    @param addr - address to send packet to
    @param dst_addr_mode - addressing mode
    @param dst_ep - destination endpoint
    @param ep - sending endpoint
    @param prfl_id - profile identifier
    @param def_resp - enable/disable default response
    @param cb - callback for getting command send status
    @param address - address value
    @param data_length - data length value
    @param slave_id - slave_id value
*/

#define ZB_ZCL_MODBUS_SEND_JSON_COMMAND_REQ(buffer, addr, dst_addr_mode, dst_ep, ep, prfl_id, def_resp, cb, address, data_length, slave_id)                                                                                                              \
    { ZB_ZCL_MODBUS_SEND_JSON_COMMAND_CMD(buffer, addr, dst_addr_mode, dst_ep, ep, prfl_id, def_resp, cb, address, data_length, slave_id, ZB_ZCL_CMD_MODBUS_JSON_COMMAND_REQ_ID) }

/*!
  @brief Parses Get json command and fills data request structure.
  @param data_buf - pointer to zb_buf_t buffer containing command request data
  @param req - variable to save command request
  @param status - success or not read parameters - ZB_TRUE or ZB_FALSE
  @note data_buf buffer should contain command request payload without ZCL header.
*/

#define ZB_ZCL_MODBUS_GET_JSON_COMMAND_REQ(data_buf, req, status)                                                                                                                                                                                        \
    { ZB_ZCL_MODBUS_GET_JSON_COMMAND_CMD(data_buf, req, status); }

/******** Response to Command json command ********/

typedef struct {
    uint8_t   fc;
    uint8_t   slave_id;
    uint16_t  addr;
    uint8_t   nb_regs;
    uint16_t* data;
} zb_zcl_modbus_data_packet_req_t;

#define MAX_NUM_REGISTERS ((zb_uint8_t)(ZB_ZCL_MB_CMD_MAX_STRING_LENGTH - 7) / 2)

typedef struct {
    uint8_t  fc;
    uint8_t  slave_id;
    uint16_t addr;
    int16_t  err;
    uint8_t  nb_regs;
    uint16_t data[MAX_NUM_REGISTERS];
} zb_zcl_modbus_data_packet_resp_t;

#define ZB_ZCL_MODBUS_JSON_COMMAND_RESP_MAX_PAYLOAD_LEN (ZB_ZCL_MB_CMD_MAX_STRING_LENGTH + 1)

#define ZB_ZCL_MODBUS_GET_JSON_COMMAND_RESP(buf, resp, parse_status)                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                    \
        zb_uint8_t len = ((uint8_t*)zb_buf_begin(buf))[0];                                                                                                                                                                                               \
        status         = ZB_ZCL_PARSE_STATUS_FAILURE;                                                                                                                                                                                                    \
        if ((zb_uint_t)(len + 1) == zb_buf_len(buf)) {                                                                                                                                                                                                   \
            resp.len = len;                                                                                                                                                                                                                              \
            ZB_MEMSET(resp.data, 0, sizeof(resp.data));                                                                                                                                                                                                  \
            ZB_MEMCPY(resp.data, (zb_uint8_t*)zb_buf_begin(buf) + 1, len);                                                                                                                                                                               \
            status = ZB_ZCL_PARSE_STATUS_SUCCESS;                                                                                                                                                                                                        \
        }                                                                                                                                                                                                                                                \
    }

/*! @brief Send Modbus cluster response to command 1
    @param buffer - to put packet to
    @param addr - address to send packet to
    @param dst_addr_mode - addressing mode
    @param dst_ep - destination endpoint
    @param ep - sending endpoint
    @param prfl_id - profile identifier
    @param def_resp - enable/disable default response
    @param cb - callback for getting command send status
    @param aps_secured - APS security mode
    @param response - response data
 */
#define ZB_ZCL_MODBUS_SEND_JSON_COMMAND_RESP(buffer, seq, dst_addr, dst_addr_mode, dst_ep, src_ep, prfl_id, cb, data, len)                                                                                                                               \
    {                                                                                                                                                                                                                                                    \
        zb_uint8_t* cmd_ptr = ZB_ZCL_START_PACKET(buffer);                                                                                                                                                                                               \
        if (cmd_ptr == NULL) {                                                                                                                                                                                                                           \
            LOG_ERR("Failed to start packet!");                                                                                                                                                                                                          \
        } else {                                                                                                                                                                                                                                         \
            ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(cmd_ptr);                                                                                                                                                                                \
            ZB_ZCL_CONSTRUCT_COMMAND_HEADER(cmd_ptr, seq, ZB_ZCL_CMD_MODBUS_JSON_COMMAND_RESP_ID);                                                                                                                                                       \
            ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, len);                                                                                                                                                                                                       \
            ZB_ZCL_PACKET_PUT_DATA_N(cmd_ptr, data, len);                                                                                                                                                                                                \
            ZB_ZCL_FINISH_PACKET((buffer), cmd_ptr)                                                                                                                                                                                                      \
            ZB_ZCL_SEND_COMMAND_SHORT((buffer), (dst_addr), (dst_addr_mode), (dst_ep), (src_ep), prfl_id, ZB_ZCL_CLUSTER_ID_MODBUS, (cb));                                                                                                               \
        }                                                                                                                                                                                                                                                \
    }

/**
 *  @brief Modbus cluster attributes
 */
typedef struct zb_zcl_modbus_attrs_s {
    /** @copydoc ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID
     * @see ZB_ZCL_ATTR_MODBUS_BAUDRATE_ID
     */
    zb_zcl_modbus_baudrate_t baudrate;
} zb_zcl_modbus_attrs_t;

/*! @} */ /* ZCL Modbus cluster definitions */

/** @endcond */

typedef struct {
    void*                            fifo_reserved;
    void*                            cb;
    zb_zcl_modbus_addr_t             addr;
    zb_zcl_modbus_data_packet_resp_t resp;
} modbus_cmd_resp_queue_data_t;

void zb_zcl_modbus_init_server(void);
void zb_zcl_modbus_init_client(void);
#define ZB_ZCL_CLUSTER_ID_MODBUS_SERVER_ROLE_INIT zb_zcl_modbus_init_server
#define ZB_ZCL_CLUSTER_ID_MODBUS_CLIENT_ROLE_INIT zb_zcl_modbus_init_client

#endif /* ZB_ZCL_MODBUS_H */
