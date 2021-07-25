#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "endian.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_bt.h"
#include "esp_int_wdt.h"
#endif

//#define WII_REMOTE_HOST
//#define WII_MITM

#include "bt.h"
#include "bt_dump.h"

void wii_controller_init();
int queue_packet_handler(uint8_t* packet, uint16_t size);
void post_bt_packet(BT_PACKET_ENVELOPE* packet);

void wii_controller_packet_handler(uint8_t* packet, uint16_t size);
void wii_remote_packet_handler(uint8_t* packet, uint16_t size);
void fake_wii_remote_packet_handler(uint8_t* packet, uint16_t size);
void wii_mitm_packet_handler(uint8_t* packet, uint16_t size);

void wii_remote_host();
void fake_wii_remote();
void wii_mitm();

void find_wii_remote();

void open_control_channel(uint16_t con_handle);
void open_data_channel(uint16_t con_handle);
void post_hid_report_packet(uint16_t con_handle, const uint8_t* hid_report, uint16_t report_size);
void post_sdp_packet(uint16_t con_handle, uint16_t l2cap_size, uint8_t* data, uint16_t data_size);
void post_sdp_packet_fragment(uint16_t con_handle, uint8_t* data, uint16_t data_size);
void post_l2ap_config_mtu_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu);
void post_l2ap_config_mtu_flush_timeout_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu, uint16_t flush_timeout);
void dump_l2cap_config_options(uint8_t* options, uint16_t options_size);

#define WII_ADDR_BLOB_NAME  "wii_addr"

#define HOST_ACL_BUFFER_SIZE         0xffff
#define HOST_NUM_ACL_BUFFERS         6
#define HOST_SCO_BUFFER_SIZE         0xff
#define HOST_NUM_SCO_BUFFERS         1

#define WII_NAME                    "Wii"
#define WII_REMOTE_NAME             "Nintendo RVL-CNT-01"
#define WII_PACKET_TYPES            0xff1e
#define SDP_PSM                     0x01
#define WII_CONTROL_PSM             0x11
#define WII_DATA_PSM                0x13
#define SDP_LOCAL_CID               0x40
#define WII_CONTROL_LOCAL_CID       0x41
#define WII_DATA_LOCAL_CID          0x42
#define LINK_KEY_BLOB_NAME          "link_key"

#define WII_SDP_MTU                 256
#define WII_SDP_FLUSH_TIMEOUT       0xffff
#define WII_REMOTE_SDP_MTU          48
#define WII_REMOTE_CONTROL_MTU      185
#define WII_REMOTE_DATA_MTU         185

#define WII_MTU                     640
#define WII_FLUSH_TIMEOUT           0xffff

#define WII_REMOTE_QOS_TOKEN_RATE       1700
#define WII_REMOTE_QOS_PEAK_BANDWIDTH   0
#define WII_REMOTE_QOS_LATENCY          10000
#define WII_REMOTE_QOS_DELAY_VARIATION  0xffffffff


#define WII_REMOTE_COD          0x002504
#define WII_COD                 0x000448

#define WII_BUTTON_LEFT         0x0100
#define WII_BUTTON_RIGHT        0x0200
#define WII_BUTTON_DOWN         0x0400
#define WII_BUTTON_UP           0x0800

#define WII_BUTTON_A            0x0008
#define WII_BUTTON_B            0x0004
#define WII_BUTTON_PLUS         0x1000
#define WII_BUTTON_HOME         0x0080
#define WII_BUTTON_MINUS        0x0010
#define WII_BUTTON_ONE          0x0002
#define WII_BUTTON_TWO          0x0001

#define WII_LED_REPORT                          0x11
#define WII_DATA_REPORTING_MODE_REPORT          0x12
#define WII_READ_MEMORY_AND_REGISTERS_REPORT    0x17

#define WII_REMOTE_LED_1        0x10
#define WII_REMOTE_LED_2        0x20
#define WII_REMOTE_LED_3        0x40
#define WII_REMOTE_LED_4        0x80

typedef enum
{
    WII_CONSOLE_PAIRING_PENDING = 1,
    WII_CONSOLE_PAIRING_STARTED,
    WII_CONSOLE_PAIRING_COMPLETE,
    WII_CONSOLE_POWER_ON_PENDING,
    WII_CONSOLE_POWER_OFF_PENDING,
    WII_CONSOLE_POWER_OFF_CONNECTED,
    //WII_CONSOLE_POWER_OFF_CONTROL_OPEN,
    //WII_CONSOLE_POWER_OFF_DATA_OPENING,
    //WII_CONSOLE_POWER_OFF_DATA_TRANSFER,
    //WII_CONSOLE_POWER_OFF_DISCONNECTING,
    WII_REMOTE_CONNECTION_PENDING = 50,
    WII_REMOTE_PAIRING_PENDING,
    WII_REMOTE_PAIRING_STARTED,
    WII_REMOTE_PAIRING_COMPLETE,
    WII_MITM_CONNECTION_PENDING = 100,
    WII_MITM_WAIT_FIRST_PACKET,
    //WII_MITM_WAIT_FIRST_PACKET_DUAL,
    WII_MITM_DISCOVERY,
    WII_MITM_DISCOVERED,
    WII_MITM_PAIRING_PENDING,
    WII_MITM_CONNECTING_WII,
    WII_MITM_CONNECTING_WII_REMOTE,
    WII_MITM_CONNECTED
} 
WII_CONTROLLER_STATE;

#ifdef _WINDOWS_
#pragma pack(push, 1)
#endif

typedef struct
{
#ifndef _WINDOWS_
    nvs_handle_t nvs_handle;
#endif
    WII_CONTROLLER_STATE state;
    uint16_t wii_con_handle;
    uint16_t wii_remote_con_handle;
    uint16_t sdp_cid;
    uint16_t control_cid;
    uint16_t data_cid;
} WII_CONTROLLER;

typedef struct
{
    uint8_t report_type;
    uint8_t report_id;
    uint8_t address_space;
    uint8_t offset_bytes[3];
    uint16_t size;
}
_PACKED_ WII_READ_MEMORY_AND_REGISTERS_PACKET;

typedef struct
{
    uint8_t report_type;
    uint8_t report_id;
    uint8_t led_flags;
}
_PACKED_ WII_LED_PACKET;

typedef struct
{
    uint8_t report_type;
    uint8_t report_id;
    uint8_t unknown_flags : 2;
    uint8_t continus_reporting_flag : 1;
    uint8_t unknown_flags_2 : 5;
    uint8_t data_report_id;
}
_PACKED_ WII_DATA_REPORTING_MODE_PACKET;

typedef struct _FOUND_DEVICE
{
    bd_addr_t addr;
    struct _FOUND_DEVICE* next;
} DISCOVERED_DEVICE;

#ifdef _WINDOWS_
#pragma pack(pop)
#endif



extern WII_CONTROLLER wii_controller;
//extern xSemaphoreHandle all_controller_buffers_sem;
extern bd_addr_t wii_addr;
extern bd_addr_t device_addr;

//extern portMUX_TYPE dump_mux;
