#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_bt.h"

#include "bt.h"
#include "wii_bt.h"

void wii_bt_init();
int wii_remote_packet_handler(uint8_t* packet, uint16_t size);
void wii_remote_test();

#define WII_REMOTE_TEST

#define WII_REMOTE_NAME             "Nintendo RVL-CNT-01"
#define WII_CONTROL_PSM             0x11
#define WII_DATA_PSM                0x13
#define WII_DATA_LOCAL_CID          0x41

#define WII_MTU                     672

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