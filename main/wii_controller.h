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
#define WII_DATA_LOCAL_CHANNEL      0x40

#define WII_MTU                     672