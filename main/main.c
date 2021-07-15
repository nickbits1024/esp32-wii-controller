#include "wii_controller.h"

//#define SPOOF_WIIMOTE

void app_main(void)
{
    bd_addr_t addr;

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
    addr[3] = 0x54;
    addr[4] = 0xd1;
    addr[5] = 0xa0;
#endif

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ret = esp_base_mac_addr_set(addr);
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    ESP_ERROR_CHECK(ret);

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    ESP_ERROR_CHECK(ret);

    wii_controller_init();

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
    static esp_vhci_host_callback_t vhci_host_cb =
    {
        .notify_host_send_available = NULL,
        .notify_host_recv = fake_wii_remote_packet_handler,
    };
    ret = esp_vhci_host_register_callback(&vhci_host_cb);
    ESP_ERROR_CHECK(ret);

    emulate_wii_remote();
#endif


}
