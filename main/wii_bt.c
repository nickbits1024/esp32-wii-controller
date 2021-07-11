#include "wii_controller.h"

xQueueHandle send_queue_handle;

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

void post_bt_packet(void* packet)
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
}

//void handle_XXX(uint8_t* packet, uint16_t size)
void handle_read_bd_addr(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[6];
    bd_addr_t addr;
    read_bda(packet + 7, addr);

    printf("local address status 0x%02x addr %s\n", status, bda_to_string(addr));
}

void handle_read_local_name(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[6];
    char* name = (char*)(packet + 7);

    printf("local name status 0x%02x name %s\n", status, name);
}

void handle_command_complete(uint8_t* packet, uint16_t size)
{
    uint16_t op_code = read_uint16(packet + 4);
    switch (op_code)
    {
        case HCI_OPCODE_READ_BD_ADDR:
            handle_read_bd_addr(packet, size);
            break;
        case HCI_OPCODE_READ_LOCAL_NAME:
            handle_read_local_name(packet, size);
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