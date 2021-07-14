#include "wii_controller.h"

WII_CONTROLLER wii_controller;
xQueueHandle send_queue_handle;
bd_addr_t device_addr;

void send_queue_task(void* p)
{
    for (;;)
    {
        BT_PACKET_ENVELOPE* env;

        if (xQueueReceive(send_queue_handle, &env, portMAX_DELAY))
        {
            printf("send size %3u data ", env->size);
            for (int i = 0; i < env->size; i++)
            {
                printf("%02x ", env->packet[i]);
            }
            printf("\n");

            while (!esp_vhci_host_check_send_available());
            esp_vhci_host_send_packet(env->packet, env->size);

            free(env);
        }
    }
}

void post_bt_packet(BT_PACKET_ENVELOPE* packet)
{
    xQueueSend(send_queue_handle, &packet, portMAX_DELAY);
}

void wii_controller_init()
{
    esp_err_t err;

    err = nvs_open("default", NVS_READWRITE, &wii_controller.nvs_handle);
    ESP_ERROR_CHECK(err);

    send_queue_handle = xQueueCreate(32, sizeof(BT_PACKET_ENVELOPE*));

    err = xTaskCreatePinnedToCore(send_queue_task, "send_queue", 8000, NULL, 1, NULL, 0) == pdPASS ? ESP_OK : ESP_FAIL;
    ESP_ERROR_CHECK(err);

    post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_BD_ADDR, 0));
    //post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_LOCAL_NAME, 0));
    //post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_SIMPLE_PAIRING_MODE, 0));    
}

//void handle_XXX(uint8_t* packet, uint16_t size)
void handle_read_bd_addr_complete(HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET* packet)
{
    memcpy(device_addr, packet->addr, BDA_SIZE);

    printf("read local address complete, status 0x%x addr %s\n", packet->status, bda_to_string(packet->addr));
}

void handle_read_simple_pairing_mode_complete(HCI_READ_SIMPLE_PAIRING_MODE_COMPLETE_PACKET* packet)
{
    printf("read simple pairing mode complete, status 0x%x mode %u\n", packet->status, packet->simple_pairing_mode);
}

void handle_read_local_name_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[6];
    char* name = (char*)(packet + 7);

    printf("read local name complete, status 0x%02x name %s\n", status, name);
}

void handle_auth_code_complete(HCI_AUTH_CODE_COMPLETE_PACKET* packet, const char* name)
{
    //reverse_bda(packet->addr);
    printf("%s addr %s status 0x%x\n", name, bda_to_string(packet->addr), packet->status);
}

void handle_write_scan_enable_complete(HCI_WRITE_SCAN_ENABLE_COMPLETE_PACKET* packet)
{
    printf("write_scan_enable complete status 0x%x\n", packet->status);
}

void handle_write_class_of_device_complete(HCI_WRITE_CLASS_OF_DEVICE_COMPLETE_PACKET* packet)
{
    printf("write_class_of_device complete status 0x%x\n", packet->status);
}

void handle_write_local_name_complete(HCI_WRITE_LOCAL_NAME_COMPLETE_PACKET* packet)
{
    printf("write_local_name complete status 0x%x\n", packet->status);
}

void handle_write_current_iac_lap_complete(HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_PACKET* packet)
{
    printf("write_current_iac_lap complete status 0x%x\n", packet->status);
}

void handle_write_authentication_enable_complete(HCI_WRITE_AUTHENTICATION_ENABLE_COMPLETE_PACKET* packet)
{
    printf("write_authentication_enable complete status 0x%x\n", packet->status);
}

void handle_write_encryption_mode_complete(HCI_WRITE_ENCRYPTION_MODE_COMPLETE_PACKET* packet)
{
    printf("write_encryption_mode complete status 0x%x\n", packet->status);
}

void handle_write_default_link_policy_settings(HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_COMPLETE_PACKET* packet)
{
    printf("write_default_link_policy_settings complete status 0x%x\n", packet->status);
}

void handle_write_secure_connections_host_support(HCI_WRITE_SECURE_CONNECTION_HOST_SUPPORT_COMPLETE_PACKET* packet)
{
    printf("write_secure_connections_host_support complete status 0x%x\n", packet->status);
}

void handle_command_complete(uint8_t* packet, uint16_t size)
{
    uint16_t op_code = read_uint16(packet + 4);
    switch (op_code)
    {
        case HCI_OPCODE_READ_BD_ADDR:
            handle_read_bd_addr_complete((HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_LOCAL_NAME:
            handle_read_local_name_complete(packet, size);
            break;
        case HCI_OPCODE_READ_SIMPLE_PAIRING_MODE:
            handle_read_simple_pairing_mode_complete((HCI_READ_SIMPLE_PAIRING_MODE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_PIN_CODE_REQUEST_REPLY:
            handle_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_pin_code_request_reply");
            break;
        case HCI_OPCODE_PIN_CODE_REQUEST_NEGATIVE_REPLY:
            handle_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_pin_code_request_negative_reply");
            break;
        case HCI_OPCODE_LINK_KEY_REQUEST_REPLY:
            handle_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_link_key_request_reply");
            break;
        case HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY:
            handle_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_link_key_request_negative_reply");
            break;
        case HCI_OPCODE_WRITE_SCAN_ENABLE:
            handle_write_scan_enable_complete((HCI_WRITE_SCAN_ENABLE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_CLASS_OF_DEVICE:
            handle_write_class_of_device_complete((HCI_WRITE_CLASS_OF_DEVICE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_LOCAL_NAME:
            handle_write_local_name_complete((HCI_WRITE_LOCAL_NAME_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_CURRENT_IAC_LAP:
            handle_write_current_iac_lap_complete((HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_AUTHENTICATION_ENABLE:
            handle_write_authentication_enable_complete((HCI_WRITE_AUTHENTICATION_ENABLE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_SET_CONNECTION_ENCRYPTION:
            printf("set_connection_encryption complete\n"); // no params
            break;
        case HCI_OPCODE_WRITE_DEFAULT_LINK_POLICY_SETTINGS:
            handle_write_default_link_policy_settings((HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT:
            handle_write_secure_connections_host_support((HCI_WRITE_SECURE_CONNECTION_HOST_SUPPORT_COMPLETE_PACKET*)packet);
            break;
        default:
            printf("unhandled command complete 0x%04x\n", op_code);
            break;
    }
}

void handle_command_status(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    uint16_t op_code = read_uint16(packet + 5);

    printf("command %04x status 0x%x\n", op_code, status);
}

void handle_number_of_completed_packets(uint8_t* packet, uint16_t size)
{
    uint8_t num_handles = packet[3];
    uint8_t* p = packet + 4;
    for (int i = 0; i < num_handles; i++)
    {
        uint16_t con_handle = read_uint16(p);
        uint16_t num_completed = read_uint16(p + 2);

        printf("number_of_completed_packets handle 0x%04x completed %u\n", con_handle, num_completed);

        p += 4;
    }
}

void handle_qos_setup_complete(HCI_QOS_SETUP_COMPLETE_EVENT_PACKET* packet)
{
    printf("qos setup handle 0x%04x status 0x%x "
        "service_type 0x%x token_rate %u peak_bandwidth %u "
        "latency %u delay_variation %u\n",
        packet->con_handle, packet->status, 
        packet->service_type, packet->token_rate, packet->peak_bandwidth, 
        packet->latency, packet->delay_variation);
}

void handle_max_slots_changed(HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET* packet)
{
    printf("max slots changed handle 0x%x lmp_max_slots %u\n", packet->con_handle, packet->lmp_max_slots);
}

void handle_role_change(HCI_ROLE_CHANGE_EVENT_PACKET* packet)
{
    printf("role change for %s status 0x%x new_role %u\n", bda_to_string(packet->addr), packet->status, packet->new_role);
}

void handle_link_key_notification(HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET* packet)
{
    printf("new link key for %s type %u", bda_to_string(packet->addr), packet->key_type);
    for (int i = 0; i < HCI_LINK_KEY_SIZE; i++)
    {
        printf(" %02x", packet->link_key[i]);
    }
    printf("\n");
    esp_err_t err = nvs_set_blob(wii_controller.nvs_handle, LINK_KEY_BLOB_NAME, packet->link_key, HCI_LINK_KEY_SIZE);
    ESP_ERROR_CHECK(err);
}

void handle_disconnection_complete(HCI_DISCONNECTION_COMPLETE_EVENT_PACKET* packet)
{
    printf("disconnected handle 0x%04x status 0x%x reason 0x%x\n", packet->con_handle, packet->status, packet->reason);
}

void handle_encryption_change(HCI_ENCRYPTION_CHANGE_EVENT_PACKET* packet)
{
    printf("encryption changed status %x  handle 0x%x encryption_enabled %u\n", packet->status, packet->con_handle, packet->encryption_enabled);
}

void dump_packet(const char* prefix, uint8_t* packet, uint16_t size)
{
    printf("%s size %3u data ", prefix, size);
    for (int i = 0; i < size; i++)
    {
        printf("%02x ", packet[i]);
    }
    printf("\n");
}

int wii_bt_packet_handler(uint8_t* packet, uint16_t size, bool handled)
{
    uint8_t packet_type;
    uint8_t event_code;

    switch (packet_type = packet[0])
    {
        case HCI_EVENT_PACKET:
            switch (event_code = packet[1])
            {
                case HCI_EVENT_COMMAND_COMPLETE:
                    handle_command_complete(packet, size);
                    break;
                case HCI_EVENT_COMMAND_STATUS:
                    handle_command_status(packet, size);
                    break;
                case HCI_EVENT_MAX_SLOTS_CHANGED:
                    handle_max_slots_changed((HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
                    handle_number_of_completed_packets(packet, size);
                    break;
                case HCI_EVENT_QOS_SETUP_COMPLETE:
                    handle_qos_setup_complete((HCI_QOS_SETUP_COMPLETE_EVENT_PACKET*)packet);
                    break;                      
                case HCI_EVENT_HARDWARE_ERROR:
                    printf("hardware error\n");
                    break;
                case HCI_EVENT_ROLE_CHANGE:
                    handle_role_change((HCI_ROLE_CHANGE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_LINK_KEY_NOTIFICATION:
                    handle_link_key_notification((HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    handle_disconnection_complete((HCI_DISCONNECTION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_ENCRYPTION_CHANGE:
                    handle_encryption_change((HCI_ENCRYPTION_CHANGE_EVENT_PACKET*)packet);
                    break;
                default:
                    if (!handled)
                    {
                        printf("unhandled event 0x%02x\n", event_code);
                    }
                    break;
            }
            break;
        default:
            if (!handled)
            {
                printf("unhandled packet type 0x%02x\n", packet_type);
            }
            break;
    }

    return 0;
}

void open_data_channel(uint16_t con_handle)
{
    post_bt_packet(create_l2cap_connection_request_packet(con_handle, WII_DATA_PSM, WII_DATA_LOCAL_CID));
}

void post_l2ap_config_mtu_request(uint16_t con_handle, uint16_t remote_cid)
{
    uint16_t options_size = sizeof(L2CAP_CONFIG_MTU_OPTION);
    BT_PACKET_ENVELOPE* env = create_l2cap_config_request_packet(con_handle, remote_cid, 0, options_size);
    L2CAP_CONFIG_REQUEST_PACKET* config_packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

    L2CAP_CONFIG_MTU_OPTION* mtu_option = (L2CAP_CONFIG_MTU_OPTION*)config_packet->options;
    mtu_option->type = L2CAP_CONFIG_MTU_OPTION_TYPE;
    mtu_option->size = sizeof(L2CAP_CONFIG_MTU_OPTION) - sizeof(L2CAP_CONFIG_OPTION);
    mtu_option->mtu = WII_MTU;

    post_bt_packet(env);
}

void dump_l2cap_config_options(uint8_t* options, uint16_t options_size)
{
    uint8_t* option = options;
    for (int i = 0; i < options_size; )
    {
        uint8_t option_type = option[0];
        uint8_t option_payload_size = option[1];
        uint8_t option_size = sizeof(L2CAP_CONFIG_OPTION) + option_payload_size;

        switch (option_type)
        {
            case L2CAP_CONFIG_MTU_OPTION_TYPE:
            {
                L2CAP_CONFIG_MTU_OPTION* mtu_option = (L2CAP_CONFIG_MTU_OPTION*)option;
                printf(" mtu %u", mtu_option->mtu);
                break;
            }
            case L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION_TYPE:
            {   L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION* fto_option = (L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION*)option;
                printf(" flush_timeout %u", fto_option->flush_timeout);
                break;
            }
            default:
                printf(" %s 0x%02x (size %u)", option_type & 0x80 ? "invalid" : "type", option_type, option_size);
                break;
        }

        i += option_size;
        option += option_size;
    }
}

