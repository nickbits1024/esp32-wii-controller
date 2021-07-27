#include "wii_controller.h"

#ifdef _WINDOWS_
typedef
{

} QUEUE;

WII_CONTROLLER wii_controller;
HANDLE output_queue_ready_sem;
HANDLE all_controller_buffers_sem;
HANDLE controller_buffers_sem[2];
QUEUE* queue_handle;
#else

WII_CONTROLLER wii_controller =
{
    .wii_con_handle = INVALID_CON_HANDLE,
    .wii_remote_con_handle = INVALID_CON_HANDLE
};
xSemaphoreHandle output_queue_ready_sem;
//xQueueHandle output_queue_handle;
//xQueueHandle input_queue_handle;
xQueueHandle queue_handle;
xSemaphoreHandle all_controller_buffers_sem;
//xSemaphoreHandle controller_buffers_sem[2];
#endif
int all_controller_buffers_sem_count;
//int controller_buffers_sem_count[2];
bd_addr_t device_addr;
bd_addr_t wii_addr;
//portMUX_TYPE dump_mux = portMUX_INITIALIZER_UNLOCKED;

void peek_number_of_completed_packets(uint8_t* packet, uint16_t size);

#ifndef _WINDOWS_

// xSemaphoreHandle get_queue_sem(uint16_t con_handle, int add)
// {
//     if (con_handle == wii_controller.wii_con_handle)
//     {
//         controller_buffers_sem_count[0] += add;
//         return controller_buffers_sem[0];
//     }
//     else if (con_handle == wii_controller.wii_remote_con_handle)
//     {
//         controller_buffers_sem_count[1] += add;
//         return controller_buffers_sem[1];
//     }
//     printf("no queue for handle 0x%x\n", con_handle);
//     abort();
// }

void queue_io_task(void* p)
{
    bool first_acl_send = true;
    while (true)
    {
        BT_PACKET_ENVELOPE* env;

        if (xQueueReceive(queue_handle, &env, portMAX_DELAY) == pdPASS)
        {
            dump_packet(env->io_direction, env->packet, env->size);

            HCI_PACKET* hci_packet = (HCI_PACKET*)env->packet;

            if (env->io_direction == OUTPUT_PACKET)
            {
                if (hci_packet->type == HCI_ACL_PACKET_TYPE)
                {
                    //HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)hci_packet;
                    if (first_acl_send)
                    {
                        xSemaphoreTake(output_queue_ready_sem, portMAX_DELAY);
                        first_acl_send = false;
                    }
                    //printf("acl take con_handle 0x%x wii %u wii remote %u..\n", acl_packet->con_handle, controller_buffers_sem_count[0], controller_buffers_sem_count[1]);
                    //xSemaphoreTake(get_queue_sem(acl_packet->con_handle, -1), portMAX_DELAY);
                    //printf("acl take con_handle 0x%x all %u\n", acl_packet->con_handle, all_controller_buffers_sem_count);
                    xSemaphoreTake(all_controller_buffers_sem, portMAX_DELAY);
                    all_controller_buffers_sem_count--;
                    //printf("acl taken...\n");
                }

                while (!esp_vhci_host_check_send_available());
                esp_vhci_host_send_packet(env->packet, env->size);
            }
            else
            {
                wii_controller_packet_handler(env->packet, env->size);
#if defined(WII_REMOTE_HOST)
                wii_remote_packet_handler(env->packet, env->size);
#elif defined(WII_MITM)
                wii_mitm_packet_handler(env->packet, env->size);
#else
                fake_wii_remote_packet_handler(env->packet, env->size);
#endif

                if (hci_packet->type == HCI_ACL_PACKET_TYPE)
                {
                    //printf("acl sent...\n");

                    HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)hci_packet;
                    uint16_t con_handles[1] = { acl_packet->con_handle };
                    uint16_t num_complete[1] = { 1 };
                    BT_PACKET_ENVELOPE* hncp_env = create_hci_host_number_of_completed_packets_packet(1, con_handles, num_complete);

                    while (!esp_vhci_host_check_send_available());
                    esp_vhci_host_send_packet(hncp_env->packet, hncp_env->size);

                    //printf("acl sent\n");

                    free(hncp_env);
                    hncp_env = NULL;
                }


            }

            free(env);
            env = NULL;
        }

        //vTaskDelay(1);
    }
}

int queue_packet_handler(uint8_t* packet, uint16_t size)
{
    HCI_EVENT_PACKET* event_packet = (HCI_EVENT_PACKET*)packet;
    if (event_packet->type == HCI_EVENT_PACKET_TYPE &&
        event_packet->event_code == HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS)
    {
        peek_number_of_completed_packets(packet, size);
    }

    BT_PACKET_ENVELOPE* env = create_packet_envelope(size);
    env->io_direction = INPUT_PACKET;
    memcpy(env->packet, packet, size);

    if (xQueueSend(queue_handle, &env, 0) != pdPASS)
    {
        printf("queue full (recv)\n");
        xQueueSend(queue_handle, &env, portMAX_DELAY);
    }

    return 0;
}

void post_bt_packet(BT_PACKET_ENVELOPE* env)
{
    env->io_direction = OUTPUT_PACKET;
    if (xQueueSend(queue_handle, &env, 0) != pdPASS)
    {
        printf("queue full (send)\n");
        xQueueSend(queue_handle, &env, portMAX_DELAY);
    }
}


void wii_controller_init()
{
    esp_err_t err;

    err = nvs_open("default", NVS_READWRITE, &wii_controller.nvs_handle);
    ESP_ERROR_CHECK(err);

    output_queue_ready_sem = xSemaphoreCreateBinary();
    //queue_sem = xSemaphoreCreateCounting(32, 0);
    //output_queue_handle = xQueueCreate(32, sizeof(BT_PACKET_ENVELOPE*));
    //input_queue_handle = xQueueCreate(32, sizeof(BT_PACKET_ENVELOPE*));
    queue_handle = xQueueCreate(32, sizeof(BT_PACKET_ENVELOPE*));

    err = xTaskCreatePinnedToCore(queue_io_task, "queue_io", 8000, NULL, 1, NULL, 0) == pdPASS ? ESP_OK : ESP_FAIL;
    ESP_ERROR_CHECK(err);

    // err = xTaskCreatePinnedToCore(output_queue_task, "output_queue", 8000, NULL, 1, NULL, 0) == pdPASS ? ESP_OK : ESP_FAIL;
    // ESP_ERROR_CHECK(err);

    post_bt_packet(create_hci_reset_packet());
    post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_BD_ADDR, 0));
    post_bt_packet(create_hci_read_buffer_size_packet());
    //post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_LOCAL_NAME, 0));
    //post_bt_packet(create_hci_cmd_packet(HCI_OPCODE_READ_SIMPLE_PAIRING_MODE, 0));    
}

#else

void wii_controller_init()
{
    wii_controller.wii_con_handle = INVALID_CONNECTION_HANDLE;
    wii_controller.wii_remote_con_handle = INVALID_CONNECTION_HANDLE;
}

#endif


//void handle_XXX(uint8_t* packet, uint16_t size)

void handle_read_bd_addr_complete(HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET* packet)
{
    memcpy(device_addr, packet->addr, BDA_SIZE);
}

void handle_read_buffer_size_complete(HCI_READ_BUFFER_SIZE_COMPLETE_PACKET* packet)
{
    if (all_controller_buffers_sem == NULL)
    {
        int all_slots = packet->hc_total_num_acl_data_packets;
        // int slots = all_slots / 2;
        // controller_buffers_sem[0] = xSemaphoreCreateCounting(slots, slots);
        // controller_buffers_sem[1] = xSemaphoreCreateCounting(slots, slots);
        // controller_buffers_sem_count[0] = slots;
        // controller_buffers_sem_count[1] = slots;
        all_controller_buffers_sem_count = all_slots;
        all_controller_buffers_sem = xSemaphoreCreateCounting(all_slots, all_slots);

        xSemaphoreGive(output_queue_ready_sem);
    }
}

void handle_command_complete(uint8_t* packet, uint16_t size)
{
    uint16_t op_code = read_uint16(packet + 4);
    switch (op_code)
    {
        case HCI_OPCODE_READ_BD_ADDR:
            handle_read_bd_addr_complete((HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_BUFFER_SIZE:
            handle_read_buffer_size_complete((HCI_READ_BUFFER_SIZE_COMPLETE_PACKET*)packet);
            break;
    }
}

void peek_number_of_completed_packets(uint8_t* packet, uint16_t size)
{
    uint8_t num_handles = packet[3];
    uint8_t* p = packet + 4;
    for (int i = 0; i < num_handles; i++)
    {
        //uint16_t con_handle = read_uint16(p);
        uint16_t num_completed = read_uint16(p + 2);

        //printf("peek number_of_completed_packets handle 0x%x completed %u\n", con_handle, num_completed);

        for (int j = 0; j < num_completed; j++)
        {
            xSemaphoreGive(all_controller_buffers_sem);
            all_controller_buffers_sem_count++;

            //xSemaphoreGive(get_queue_sem(con_handle, 1));
        }

        p += 4;
    }
}

void handle_link_key_notification(HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET* packet)
{
#ifndef WII_REMOTE_HOST
    esp_err_t err = nvs_set_blob(wii_controller.nvs_handle, LINK_KEY_BLOB_NAME, packet->link_key, HCI_LINK_KEY_SIZE);
    ESP_ERROR_CHECK(err);
#endif
}

void wii_controller_packet_handler(uint8_t* packet, uint16_t size)
{
    HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)packet;
    HCI_EVENT_PACKET* event_packet = (HCI_EVENT_PACKET*)packet;
    //L2CAP_PACKET* l2cap_packet = (L2CAP_PACKET*)packet;

    switch (acl_packet->type)
    {
        // case HCI_COMMAND_PACKET_TYPE:
        //     handle_command((HCI_COMMAND_PACKET*)packet);
        //     break;
        case HCI_EVENT_PACKET_TYPE:
            switch (event_packet->event_code)
            {
                case HCI_EVENT_COMMAND_COMPLETE:
                    handle_command_complete(packet, size);
                    break;
                // case HCI_EVENT_COMMAND_STATUS:
                //     handle_command_status(packet, size);
                //     break;
                // case HCI_EVENT_MAX_SLOTS_CHANGED:
                //     handle_max_slots_changed((HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET*)packet);
                //     break;
                // case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
                //     handle_number_of_completed_packets(packet, size);
                //     break;
                // case HCI_EVENT_QOS_SETUP_COMPLETE:
                //     handle_qos_setup_complete((HCI_QOS_SETUP_COMPLETE_EVENT_PACKET*)packet);
                //     break;
                // case HCI_EVENT_HARDWARE_ERROR:
                //     printf("hardware error\n");
                //     break;
                // case HCI_EVENT_ROLE_CHANGE:
                //     handle_role_change((HCI_ROLE_CHANGE_EVENT_PACKET*)packet);
                //     break;
                case HCI_EVENT_LINK_KEY_NOTIFICATION:
                    handle_link_key_notification((HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET*)packet);
                    break;
                // case HCI_EVENT_DISCONNECTION_COMPLETE:
                //     handle_disconnection_complete((HCI_DISCONNECTION_COMPLETE_EVENT_PACKET*)packet);
                //     break;
                // case HCI_EVENT_MODE_CHANGE:
                //     handle_mode_change((HCI_MODE_CHANGE_EVENT_PACKET*)packet);
                //     break;
                // case HCI_EVENT_ENCRYPTION_CHANGE:
                //     handle_encryption_change((HCI_ENCRYPTION_CHANGE_EVENT_PACKET*)packet);
                //     break;
                default:
                    printf("%s\n", get_hci_event_name(event_packet->event_code));
                    break;
            }
            break;
    }
}

void open_control_channel(uint16_t con_handle)
{
    post_bt_packet(create_l2cap_connection_request_packet(con_handle, WII_CONTROL_PSM, WII_CONTROL_LOCAL_CID));
}

void open_data_channel(uint16_t con_handle)
{
    post_bt_packet(create_l2cap_connection_request_packet(con_handle, WII_DATA_PSM, WII_DATA_LOCAL_CID));
}

void post_hid_report_packet(uint16_t con_handle, const uint8_t* report, uint16_t report_size)
{
    printf("send hid report \"");
    for (int i = 0; i < report_size; i++)
    {
        printf("\\x%02x", report[i]);
    }
    printf("\", %u\n", report_size);

    post_bt_packet(create_l2cap_packet(con_handle, L2CAP_AUTO_SIZE, wii_controller.data_cid, report, report_size));
}

int sdp_packet_index = -1;
int sdp_fragment_index = 0;

void post_sdp_packet(uint16_t con_handle, uint16_t l2cap_size, uint8_t* data, uint16_t data_size)
{
    sdp_packet_index++;
    sdp_fragment_index = 0;

    printf("sdp request %d.%d \"", sdp_packet_index, sdp_fragment_index);
    for (int i = 0; i < data_size; i++)
    {
        printf("\\x%02x", data[i]);
    }
    printf("\", %u\n", data_size);

    post_bt_packet(create_l2cap_packet(con_handle, l2cap_size, wii_controller.sdp_cid, data, data_size));
}

void post_sdp_packet_fragment(uint16_t con_handle, uint8_t* data, uint16_t data_size)
{
    sdp_fragment_index++;

    printf("sdp request %d.%d \"", sdp_packet_index, sdp_fragment_index);
    for (int i = 0; i < data_size; i++)
    {
        printf("\\x%02x", data[i]);
    }
    printf("\", %u\n", data_size);

    post_bt_packet(create_acl_packet(con_handle, wii_controller.sdp_cid, L2CAP_PB_FRAGMENT, L2CAP_BROADCAST_NONE, data, data_size));
}

void post_l2ap_config_mtu_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu)
{
    uint16_t options_size = sizeof(L2CAP_CONFIG_MTU_OPTION);
    BT_PACKET_ENVELOPE* env = create_l2cap_config_request_packet(con_handle, remote_cid, 0, options_size);
    L2CAP_CONFIG_REQUEST_PACKET* config_packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

    L2CAP_CONFIG_MTU_OPTION* mtu_option = (L2CAP_CONFIG_MTU_OPTION*)config_packet->options;
    mtu_option->type = L2CAP_CONFIG_MTU_OPTION_TYPE;
    mtu_option->size = sizeof(L2CAP_CONFIG_MTU_OPTION) - sizeof(L2CAP_CONFIG_OPTION);
    mtu_option->mtu = mtu;

    post_bt_packet(env);
}

void post_l2ap_config_mtu_flush_timeout_request(uint16_t con_handle, uint16_t remote_cid, uint16_t mtu, uint16_t flush_timeout)
{
    uint16_t options_size = sizeof(L2CAP_CONFIG_MTU_OPTION) + sizeof(L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION);
    BT_PACKET_ENVELOPE* env = create_l2cap_config_request_packet(con_handle, remote_cid, 0, options_size);
    L2CAP_CONFIG_REQUEST_PACKET* config_packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

    L2CAP_CONFIG_MTU_OPTION* mtu_option = (L2CAP_CONFIG_MTU_OPTION*)config_packet->options;
    mtu_option->type = L2CAP_CONFIG_MTU_OPTION_TYPE;
    mtu_option->size = sizeof(L2CAP_CONFIG_MTU_OPTION) - sizeof(L2CAP_CONFIG_OPTION);
    mtu_option->mtu = mtu;
    L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION* flush_option = (L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION*)(mtu_option + 1);
    flush_option->type = L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION_TYPE;
    flush_option->size = sizeof(L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION) - sizeof(L2CAP_CONFIG_OPTION);
    flush_option->flush_timeout = flush_timeout;

    post_bt_packet(env);
}

