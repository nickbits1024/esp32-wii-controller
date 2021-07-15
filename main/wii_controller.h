#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "endian.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_bt.h"

#include "bt.h"
#include "wii_bt.h"

void wii_controller_init();
int wii_remote_packet_handler(uint8_t* packet, uint16_t size);
int fake_wii_remote_packet_handler(uint8_t* packet, uint16_t size);
void wii_remote_test();
void emulate_wii_remote();
void open_data_channel();
void post_sdp_packet(uint8_t* data, uint16_t data_size);
void post_sdp_packet_fragment(uint8_t* data, uint16_t data_size);
void post_l2ap_config_mtu_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu);
void post_l2ap_config_mtu_flush_timeout_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu, uint16_t flush_timeout);
void dump_l2cap_config_options(uint8_t* options, uint16_t options_size);

//#define WII_REMOTE_TEST

#define WII_REMOTE_NAME             "Nintendo RVL-CNT-01"
#define SDP_PSM                     0x01
#define WII_CONTROL_PSM             0x11
#define WII_DATA_PSM                0x13
#define SDP_LOCAL_CID               0x43
#define WII_CONTROL_LOCAL_CID       0x44
#define WII_DATA_LOCAL_CID          0x45
#define LINK_KEY_BLOB_NAME          "link_key"

#define WII_MTU                     672

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

#define WII_REMOTE_LED_REPORT   0x11

#define WII_REMOTE_LED_1        0x10
#define WII_REMOTE_LED_2        0x20
#define WII_REMOTE_LED_3        0x40
#define WII_REMOTE_LED_4        0x80

typedef enum
{
    STATE_WII_CONSOLE_PAIRING_PENDING = 1,
    STATE_WII_CONSOLE_PAIRING_STARTED,
    STATE_WII_CONSOLE_PAIRING_COMPLETE,
    STATE_WII_CONSOLE_POWER_OFF_PENDING,
    STATE_WII_CONSOLE_POWER_OFF_CONNECTED,
    STATE_WII_REMOTE_PAIRING_PENDING,
    STATE_WII_REMOTE_PAIRING_STARTED,
    STATE_WII_REMOTE_PAIRING_COMPLETE
} 
WII_CONTROLLER_STATE;

typedef struct
{
    nvs_handle_t nvs_handle;
    WII_CONTROLLER_STATE state;
    uint16_t con_handle;
    uint16_t sdp_cid;
    uint16_t control_cid;
    uint16_t data_cid;
} WII_CONTROLLER;

extern WII_CONTROLLER wii_controller;
