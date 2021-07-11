#include "wii_controller.h"

void noop()
{
}

void app_main(void)
{
    uint8_t addr[6];

#if defined(SPOOF_WIIMOTE)
    //00:19:1d:54:d1:a4
    addr[0] = 0x00;
    addr[1] = 0x19;
    addr[2] = 0x1d;
    addr[3] = 0x54;
    addr[4] = 0xd1;
    addr[5] = 0xa4 - 2;
#elif defined(SPOOF_WII)
    addr[0] = 0x00;
    addr[1] = 0x19;
    addr[2] = 0x1d;
    addr[3] = 0x22;
    addr[4] = 0x73;
    addr[5] = 0x29 - 2;
#else
    addr[0] = 0x00;
    addr[1] = 0x19;
    addr[2] = 0x1d;
    addr[3] = 0x22;
    addr[4] = 0x73;
    addr[5] = 0x11; // random    
#endif

    esp_err_t ret = esp_base_mac_addr_set(addr);
    ESP_ERROR_CHECK(ret);

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    ESP_ERROR_CHECK(ret);

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    ESP_ERROR_CHECK(ret);

    wii_bt_init();

#ifdef WII_REMOTE_TEST
    static esp_vhci_host_callback_t vhci_host_cb =
    {
        .notify_host_send_available = NULL,
        .notify_host_recv = wii_remote_packet_handler,
    };
    ret = esp_vhci_host_register_callback(&vhci_host_cb);
    ESP_ERROR_CHECK(ret);

    wii_remote_test();
#else

#endif


}
