#include "wii_controller.h"

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

void wii_bt_init()
{
    esp_err_t err;

    send_queue_handle = xQueueCreate(32, sizeof(BT_PACKET_ENVELOPE*));

    err = xTaskCreatePinnedToCore(send_queue_task, "send_queue", 8000, NULL, 1, NULL, 0) == pdPASS ? ESP_OK : ESP_FAIL;
    ESP_ERROR_CHECK(err);

    post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_BD_ADDR, 0));
    post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_LOCAL_NAME, 0));
    post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_SIMPLE_PAIRING_MODE, 0));    
}

//void handle_XXX(uint8_t* packet, uint16_t size)
void handle_read_bd_addr_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[6];
    bd_addr_t addr;
    // read_bda(packet + 7, addr);
    memcpy(device_addr, packet + 7, BD_ADDR_LEN);

    printf("read local address complete, status 0x%02x addr %s\n", status, bda_to_string(addr));
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

void handle_command_complete(uint8_t* packet, uint16_t size)
{
    uint16_t op_code = read_uint16(packet + 4);
    switch (op_code)
    {
        case HCI_OPCODE_READ_BD_ADDR:
            handle_read_bd_addr_complete(packet, size);
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
        default:
            printf("unhandled command complete 0x%04x\n", op_code);
            break;
    }
}

void handle_command_status(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    uint16_t op_code = read_uint16(packet + 5);

    printf("command %04x status 0x%02x\n", op_code, status);
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

void handle_qos_setup_complete(HCI_QOS_SETUP_COMPLETE_PACKET* packet)
{
    printf("qos setup handle 0x%04x status 0x%x "
        "service_type 0x%x token_rate %u peak_bandwidth %u "
        "latency %u delay_variation %u\n",
        packet->con_handle, packet->status, 
        packet->service_type, packet->token_rate, packet->peak_bandwidth, 
        packet->latency, packet->delay_variation);
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
                case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
                    handle_number_of_completed_packets(packet, size);
                    break;
                case HCI_EVENT_QOS_SETUP_COMPLETE:
                    handle_qos_setup_complete((HCI_QOS_SETUP_COMPLETE_PACKET*)packet);
                    break;                      
                case HCI_EVENT_HARDWARE_ERROR:
                    printf("hardware error\n");
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