#include "wii_controller.h"

#define WII_ADDR_BLOB_NAME  "wii_addr"


bd_addr_t wii_addr;

void send_disconnect(uint16_t con_handle, uint8_t reason);
void send_power_off_disconnect(uint16_t con_handle);

void handle_fake_wii_remote_connection_request(HCI_CONNECTION_REQUEST_EVENT_PACKET* packet)
{
    uint32_t cod = uint24_bytes_to_uint32(packet->class_of_device);
    printf("connection request from %s cod %06x type %u\n", bda_to_string(packet->addr), cod, packet->link_type);

    if (packet->link_type == HCI_LINK_TYPE_ACL && cod == WII_COD)
    {
        printf("accepting wii connection...\n");
        post_bt_packet(create_hci_accept_connection_request_packet(packet->addr, HCI_ROLE_SLAVE));
    }
    else
    {
        printf("rejecting unknown connection...\n");
        post_bt_packet(create_hci_reject_connection_request_packet(packet->addr, ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR));
    }
}

void handle_fake_wii_remote_connection_complete(HCI_CONNECTION_COMPLETE_EVENT_PACKET* packet)
{
    printf("connection complete addr %s status 0x%02x handle 0x%04x, link_type %u encrypted %u\n", bda_to_string(packet->addr), packet->status, packet->con_handle, packet->link_type, packet->encryption_enabled);
    if (packet->status == ERROR_CODE_SUCCESS)
    {
        wii_controller.con_handle = packet->con_handle;
        switch (wii_controller.state)
        {
            case STATE_WII_CONSOLE_PAIRING_PENDING:
                memcpy(wii_addr, packet->addr, BDA_SIZE);

                wii_controller.state = STATE_WII_CONSOLE_PAIRING_STARTED;
                printf("pairing started...\n");
                post_bt_packet(create_hci_authentication_requested_packet(packet->con_handle));
                break;
            case STATE_WII_CONSOLE_POWER_OFF_PENDING:
                //send_power_off_disconnect(packet->con_handle);
                wii_controller.state = STATE_WII_CONSOLE_POWER_OFF_CONNECTED;
                post_bt_packet(create_hci_set_connection_encryption(packet->con_handle, 1));
                open_control_channel();
                open_data_channel();
                break;
            default:
                break;
        }
    }
    else if (packet->status == ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR)
    {
        switch (wii_controller.state)
        {
            // case STATE_WII_CONSOLE_POWER_OFF_PENDING:
            //     printf("requesting auth...\n");
            //     post_bt_packet(create_hci_authentication_requested_packet(packet->con_handle));
            //     break;
            default:
                break;
        }

    }
}

void handle_fake_wii_remote_link_key_request(HCI_LINK_KEY_REQUEST_EVENT_PACKET* packet)
{
    //reverse_bda(packet->addr);

    printf("link key request from %s...\n", bda_to_string(packet->addr));

    switch (wii_controller.state)
    {
        case STATE_WII_CONSOLE_PAIRING_PENDING:
        case STATE_WII_CONSOLE_PAIRING_STARTED:
            printf("rejecting link key request from %s...\n", bda_to_string(packet->addr));
            post_bt_packet(create_hci_link_key_request_negative_packet(packet->addr));
            break;
        case STATE_WII_CONSOLE_POWER_OFF_PENDING:
        {
            uint8_t link_key[HCI_LINK_KEY_SIZE];
            size_t size = HCI_LINK_KEY_SIZE;
            esp_err_t err = nvs_get_blob(wii_controller.nvs_handle, LINK_KEY_BLOB_NAME, link_key, &size);
            if (err == ESP_OK && size == HCI_LINK_KEY_SIZE)
            {
                printf("returning stored link key");
                for (int i = 0; i < HCI_LINK_KEY_SIZE; i++)
                {
                    printf(" %02x", link_key[i]);
                }
                printf("\n");
                post_bt_packet(create_hci_link_key_request_reply_packet(packet->addr, link_key));
            }
            break;
        }
        default:
            break;
    }
}

void handle_fake_wii_remote_pin_code_request(HCI_PIN_CODE_REQUEST_EVENT_PACKET* packet)
{
    //reverse_bda(packet->addr);

    printf("pin code request from %s...\n", bda_to_string(packet->addr));

    switch (wii_controller.state)
    {
        case STATE_WII_CONSOLE_PAIRING_PENDING:
        case STATE_WII_CONSOLE_PAIRING_STARTED:
        {
            uint8_t pin[6];
            //write_bda(pin, device_addr);
            memcpy(pin, packet->addr, BDA_SIZE);
            printf("sending pin code %02x %02x %02x %02x %02x %02x\n",
                pin[0], pin[1], pin[2], pin[3], pin[4], pin[5]);

            post_bt_packet(create_hci_pin_code_request_reply_packet(packet->addr, pin, BDA_SIZE));
            break;
        }
        case STATE_WII_CONSOLE_POWER_OFF_PENDING:
            post_bt_packet(create_hci_pin_code_request_negative_reply_packet(packet->addr));
            break;
        default:
            break;
    }
}

void handle_fake_wii_remote_authentication_complete(HCI_AUTHENTICATION_COMPLETE_EVENT_PACKET* packet)
{
    printf("auth complete handle 0x%x status 0x%x\n", packet->con_handle, packet->status);

    if (wii_controller.state == STATE_WII_CONSOLE_PAIRING_STARTED)
    {
        wii_controller.state = packet->status == ERROR_CODE_SUCCESS ? STATE_WII_CONSOLE_PAIRING_COMPLETE : 0;

        if (packet->status == ERROR_CODE_SUCCESS)
        {
            //open_wii_remote_data_channel(packet->con_handle);
            printf("storing wii address %s\n", bda_to_string(wii_addr));
            nvs_set_blob(wii_controller.nvs_handle, WII_ADDR_BLOB_NAME, wii_addr, BDA_SIZE);

            post_bt_packet(create_hci_set_connection_encryption(packet->con_handle, 1));
        }
    }
}

void handle_fake_wii_remote_l2cap_connection_response(L2CAP_CONNECTION_RESPONSE_PACKET* response_packet)
{
    printf("l2cap conn response handle 0x%04x remote_cid 0x%x local_cid 0x%x result 0x%04x status 0x%x\n",
        response_packet->con_handle, response_packet->remote_cid, response_packet->local_cid, response_packet->result, response_packet->status);

    if (wii_controller.state == STATE_WII_CONSOLE_POWER_OFF_CONNECTED)
    {
        if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_PENDING)
        {
            post_l2ap_config_mtu_flush_timeout_request(response_packet->con_handle, response_packet->remote_cid, WII_MTU, WII_FLUSH_TIMEOUT);
        }
    }

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_SUCCESS)
    {
        wii_controller.con_handle = response_packet->con_handle;
        switch (response_packet->local_cid)
        {
            case WII_CONTROL_LOCAL_CID:
                wii_controller.control_cid = response_packet->remote_cid;
                printf("set wii_controller.control_cid %04x wii_remote_control_cid=0x%x\n", wii_controller.con_handle, wii_controller.control_cid);
                break;
            case WII_DATA_LOCAL_CID:
                wii_controller.data_cid = response_packet->remote_cid;
                printf("set wii_controller.data_cid %04x wii_remote_data_cid=0x%x\n", wii_controller.con_handle, wii_controller.data_cid);
                break;
        }
    }
}

void handle_fake_wii_remote_l2cap_connection_request(L2CAP_CONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap connection request handle 0x%x psm 0x%x local_cid %04x\n", packet->con_handle, packet->psm, packet->local_cid);

    wii_controller.con_handle = packet->con_handle;
    uint16_t cid;
    switch (packet->psm)
    {
        case SDP_PSM:
            wii_controller.sdp_cid = packet->local_cid;
            cid = SDP_LOCAL_CID;
            printf("set wii_controller.con_handle %04x wii_controller.sdp_cid=0x%x\n", wii_controller.con_handle, wii_controller.sdp_cid);
            break;
        case WII_CONTROL_PSM:
            wii_controller.control_cid = packet->local_cid;
            cid = WII_CONTROL_LOCAL_CID;
            printf("set wii_controller.con_handle %04x wii_controller.control_cid=0x%x\n", wii_controller.con_handle, wii_controller.control_cid);
            break;
        case WII_DATA_PSM:
            wii_controller.data_cid = packet->local_cid;
            cid = WII_DATA_LOCAL_CID;
            printf("set wii_controller.con_handle %04x wii_controller.data_cid=0x%x\n", wii_controller.con_handle, wii_controller.data_cid);
            break;
        default:
            cid = 0;
            break;
    }

    if (cid == 0)
    {
        printf("connection request no matching psm 0x%x\n", packet->psm);
        return;
    }

    post_bt_packet(create_l2cap_connection_response_packet(packet->con_handle, packet->identifier, cid, packet->local_cid, 0, ERROR_CODE_SUCCESS));
    //post_l2ap_config_mtu_request(packet->con_handle, packet->local_cid, WII_MTU);
    post_l2ap_config_mtu_flush_timeout_request(packet->con_handle, packet->local_cid, WII_MTU, WII_FLUSH_TIMEOUT);
}

void handle_fake_wii_remote_l2cap_config_request(L2CAP_CONFIG_REQUEST_PACKET* request_packet)
{
    uint16_t options_size = request_packet->payload_size - 4;

    uint16_t cid;
    switch (request_packet->remote_cid)
    {
        case SDP_LOCAL_CID:
            cid = wii_controller.sdp_cid;
            break;
        case WII_CONTROL_LOCAL_CID:
            cid = wii_controller.control_cid;
            break;
        case WII_DATA_LOCAL_CID:
            cid = wii_controller.data_cid;
            break;
        default:
            cid = 0;
            break;
    }

    if (cid == 0)
    {
        printf("l2cap config request no matching cid for 0x%x\n", request_packet->remote_cid);
        return;
    }

    printf("l2cap config request 0x%04x remote_cid 0x%x local_cid 0x%x options_size %u options", request_packet->con_handle, request_packet->remote_cid, cid, options_size);
    dump_l2cap_config_options(request_packet->options, options_size);
    printf("\n");

    BT_PACKET_ENVELOPE* env = create_l2cap_config_response_packet(request_packet->con_handle, request_packet->identifier, cid, request_packet->flags, options_size);

    L2CAP_CONFIG_RESPONSE_PACKET* response_packet = (L2CAP_CONFIG_RESPONSE_PACKET*)env->packet;

    memcpy(response_packet->options, request_packet->options, options_size);

    post_bt_packet(env);
}

void handle_fake_wii_remote_l2cap_config_response(L2CAP_CONFIG_RESPONSE_PACKET* packet)
{
    uint16_t options_size = packet->payload_size - 6;

    printf("l2cap config response handle 0x%04x local_cid 0x%x result 0x%x options_size %u options", packet->con_handle, packet->local_cid, packet->result, options_size);
    dump_l2cap_config_options(packet->options, options_size);
    printf("\n");
}

void handle_fake_wii_remote_l2cap_disconnection_request(L2CAP_DISCONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap disconnect request 0x%04x remote_cid 0x%0x local_cid 0x%x\n", packet->con_handle, packet->remote_cid, packet->local_cid);

    post_bt_packet(create_l2cap_disconnection_response_packet(packet->con_handle, packet->identifier, packet->remote_cid, packet->local_cid));
    //post_bt_packet(create_hci_disconnect_packet(packet->con_handle, ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION));
}

void handle_fake_wii_remote_l2cap_signal_channel(L2CAP_SIGNAL_CHANNEL_PACKET* packet)
{
    switch (packet->code)
    {
        case L2CAP_CONNECTION_REQUEST:
            handle_fake_wii_remote_l2cap_connection_request((L2CAP_CONNECTION_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONNECTION_RESPONSE:
            handle_fake_wii_remote_l2cap_connection_response((L2CAP_CONNECTION_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_CONFIG_REQUEST:
            handle_fake_wii_remote_l2cap_config_request((L2CAP_CONFIG_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONFIG_RESPONSE:
            handle_fake_wii_remote_l2cap_config_response((L2CAP_CONFIG_RESPONSE_PACKET*)packet);
            break;
            // case L2CAP_COMMAND_REJECT:
            //     handle_wii_remote_l2cap_command_reject((L2CAP_COMMAND_REJECT_PACKET*)packet);
            //     break;
        case L2CAP_DISCONNECTION_REQUEST:
            handle_fake_wii_remote_l2cap_disconnection_request((L2CAP_DISCONNECTION_REQUEST_PACKET*)packet);
            break;
        default:
            printf("unhandled signal channel code 0x%02x\n", packet->code);
            break;
    }
}

typedef struct
{
    char* request;
    uint16_t request_size;
    char* response;
    uint16_t response_size;
} REQUEST_RESPONSE;

REQUEST_RESPONSE sdp_request_responses[] =
{
    { "\x02\x00\x00\x00\x08\x35\x03\x19\x11\x24\x00\x15\x00", 13, "\x03\x00\x00\x00\x09\x00\x01\x00\x01\x00\x01\x00\x00\x00", 14 },
    { "\x04\x00\x01\x00\x0e\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x00", 19, "\x05\x00\x01\x00\x7b\x00\x76\x36\x01\xcc\x09\x00\x00\x0a\x00\x01\x00\x00\x09\x00\x01\x35\x03", 23 },
    { NULL, 0, "\x19\x11\x24\x09\x00\x04\x35\x0d\x35\x06\x19\x01\x00\x09\x00\x11\x35\x03\x19\x00\x11\x09\x00\x05\x35\x03\x19", 27 },
    { NULL, 0, "\x10\x02\x09\x00\x06\x35\x09\x09\x65\x6e\x09\x00\x6a\x09\x01\x00\x09\x00\x09\x35\x08\x35\x06\x19\x11\x24\x09", 27 },
    { NULL, 0, "\x01\x00\x09\x00\x0d\x35\x0f\x35\x0d\x35\x06\x19\x01\x00\x09\x00\x13\x35\x03\x19\x00\x11\x09\x01\x00\x25\x13", 27 },
    { NULL, 0, "\x4e\x69\x6e\x74\x65\x6e\x64\x6f\x20\x52\x56\x4c\x2d\x43\x4e\x54\x2d\x30\x31\x09\x01\x02\x00\x76", 24 },
    { "\x04\x00\x02\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x00\x76", 21, "\x05\x00\x02\x00\x7b\x00\x76\x01\x25\x13\x4e\x69\x6e\x74\x65\x6e\x64\x6f\x20\x52\x56\x4c\x2d", 23 },
    { NULL, 0, "\x43\x4e\x54\x2d\x30\x31\x09\x01\x02\x25\x08\x4e\x69\x6e\x74\x65\x6e\x64\x6f\x09\x02\x00\x09\x01\x00\x09\x02", 27 },
    { NULL, 0, "\x01\x09\x01\x11\x09\x02\x02\x08\x04\x09\x02\x03\x08\x33\x09\x02\x04\x28\x00\x09\x02\x05\x28\x01\x09\x02\x06", 27 },
    { NULL, 0, "\x35\xdf\x35\xdd\x08\x22\x25\xd9\x05\x01\x09\x05\xa1\x01\x85\x10\x15\x00\x26\xff\x00\x75\x08\x95\x01\x06\x00", 27 },
    { NULL, 0, "\xff\x09\x01\x91\x00\x85\x11\x95\x01\x09\x01\x91\x00\x85\x12\x95\x02\x09\x01\x91\x00\x02\x00\xec", 24 },
    { "\x04\x00\x03\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x00\xec", 21, "\x05\x00\x03\x00\x7b\x00\x76\x85\x13\x95\x01\x09\x01\x91\x00\x85\x14\x95\x01\x09\x01\x91\x00", 23 },
    { NULL, 0, "\x85\x15\x95\x01\x09\x01\x91\x00\x85\x16\x95\x15\x09\x01\x91\x00\x85\x17\x95\x06\x09\x01\x91\x00\x85\x18\x95", 27 },
    { NULL, 0, "\x15\x09\x01\x91\x00\x85\x19\x95\x01\x09\x01\x91\x00\x85\x1a\x95\x01\x09\x01\x91\x00\x85\x20\x95\x06\x09\x01", 27 },
    { NULL, 0, "\x81\x00\x85\x21\x95\x15\x09\x01\x81\x00\x85\x22\x95\x04\x09\x01\x81\x00\x85\x30\x95\x02\x09\x01\x81\x00\x85", 27 },
    { NULL, 0, "\x31\x95\x05\x09\x01\x81\x00\x85\x32\x95\x0a\x09\x01\x81\x00\x85\x33\x95\x11\x09\x01\x02\x01\x62", 24 },
    { "\x04\x00\x04\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x01\x62", 21, "\x05\x00\x04\x00\x70\x00\x6d\x81\x00\x85\x34\x95\x15\x09\x01\x81\x00\x85\x35\x95\x15\x09\x01", 23 },
    { NULL, 0, "\x81\x00\x85\x36\x95\x15\x09\x01\x81\x00\x85\x37\x95\x15\x09\x01\x81\x00\x85\x3d\x95\x15\x09\x01\x81\x00\x85", 27 },
    { NULL, 0, "\x3e\x95\x15\x09\x01\x81\x00\x85\x3f\x95\x15\x09\x01\x81\x00\xc0\x09\x02\x07\x35\x08\x35\x06\x09\x04\x09\x09", 27 },
    { NULL, 0, "\x01\x00\x09\x02\x08\x28\x00\x09\x02\x09\x28\x01\x09\x02\x0a\x28\x01\x09\x02\x0b\x09\x01\x00\x09\x02\x0c\x09", 27 },
    { NULL, 0, "\x0c\x80\x09\x02\x0d\x28\x00\x09\x02\x0e\x28\x00\x00", 13 }
};

void handle_fake_wii_remote_sdp_channel(L2CAP_PACKET* packet)
{
    int request_responses_size = sizeof(sdp_request_responses) / sizeof(REQUEST_RESPONSE);
    for (int i = 0; i < request_responses_size; i++)
    {
        const REQUEST_RESPONSE* rr = &sdp_request_responses[i];
        if (packet->l2cap_size == rr->request_size && memcmp(packet->data, rr->request, packet->l2cap_size) == 0)
        {
            uint16_t l2cap_size = rr->response_size;
            for (int j = i + 1; j < request_responses_size; j++)
            {
                const REQUEST_RESPONSE* rr2 = &sdp_request_responses[j];
                if (rr2->request_size == 0 && rr2->request == NULL && rr2->response_size > 0)
                {
                    l2cap_size += rr2->response_size;
                }
                else
                {
                    break;
                }
            }

            post_sdp_packet(l2cap_size, (uint8_t*)rr->response, rr->response_size);
            for (int j = i + 1; j < request_responses_size; j++)
            {
                const REQUEST_RESPONSE* rr2 = &sdp_request_responses[j];
                if (rr2->request_size == 0 && rr2->request == NULL && rr2->response_size > 0)
                {
                    post_sdp_packet_fragment((uint8_t*)rr2->response, rr2->response_size);
                }
                else
                {
                    break;
                }
            }
            return;
        }
    }

    printf("no sdp response request post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)\"");
    for (int i = 0; i < packet->l2cap_size; i++)
    {
        printf("\\x%02x", packet->data[i]);
    }
    printf("\", %u);\n", packet->l2cap_size);
}

REQUEST_RESPONSE hid_request_responses[] =
{
    { "\xa2\x17\x00\x00\x17\x70\x00\x01", 8, "\xa1\x21\x00\x00\xf8\x17\x70\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { "\xa2\x12\x06\x30", 4, "\xa1\x22\x00\x00\x12\x00", 6 },
    { NULL, 0, "\xa1\x30\x00\x00", 4 },
    { "\xa2\x1a\x02", 3, "\xa1\x22\x00\x00\x1a\x00", 6 },
    { "\xa2\x11\x12", 3, "\xa1\x22\x00\x00\x11\x00", 6 },
    { "\xa2\x17\x00\x00\x00\x2a\x00\x38", 8, "\xa1\x21\x00\x00\xf0\x00\x3a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\xf0\x00\x4a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\x70\x00\x5a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { "\xa2\x17\x00\x00\x00\x62\x00\x38", 8, "\xa1\x21\x00\x00\xf0\x00\x62\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\xf0\x00\x72\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\xf0\x00\x82\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\x70\x00\x92\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23 },
    { "\xa2\x17\x00\x00\x00\x00\x00\x2a", 8, "\xa1\x21\x00\x00\xf0\x00\x00\x64\xd2\x8b\x68\xd7\x6c\x89\x03\x6c\x91\x4a\x64\xd2\x8b\x68\xd7", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\xf0\x00\x10\x6c\x89\x03\x6c\x91\x4a\x7f\x81\x83\x19\x99\x9b\x9c\x0a\x40\x0b", 23 },
    { NULL, 0, "\xa1\x21\x00\x00\x90\x00\x20\x7f\x81\x83\x19\x99\x9b\x9c\x0a\x40\x0b\x00\x00\x00\x00\x00\x00", 23 },
    { "\xa2\x15\x00", 3, "\xa1\x20\x00\x00\x80\x00\x00\x7d", 8 },
    { "\xa2\x13\x06", 3, "\xa1\x22\x00\x00\x13\x00", 6 },
    { "\xa2\x1a\x06", 3, "\xa1\x22\x00\x00\x1a\x00", 6 },
    { "\xa2\x16\x04\xb0\x00\x30\x01\x01\x00\x15\x00\x14\xbc\xc4\x00\x00\x00\x07\x00\x00\x00\x02\x81", 23, "\xa1\x22\x00\x00\x16\x07", 6 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

void post_hid_request_reponse(HID_REPORT_PACKET* packet, uint16_t size)
{
    uint8_t* p = (uint8_t*)packet;

    int request_responses_size = sizeof(hid_request_responses) / sizeof(REQUEST_RESPONSE);
    for (int i = 0; i < request_responses_size; i++)
    {
        const REQUEST_RESPONSE* rr = &hid_request_responses[i];
        if (size == rr->request_size && memcmp(p, rr->request, size) == 0)
        {
            post_hid_report_packet((uint8_t*)rr->response, rr->response_size);
            for (int j = i + 1; j < request_responses_size; j++)
            {
                const REQUEST_RESPONSE* rr2 = &hid_request_responses[j];
                if (rr2->request_size == 0 && rr2->request == NULL && rr2->response_size > 0)
                {
                    post_hid_report_packet((uint8_t*)rr2->response, rr2->response_size);   
                }
                else
                {
                    break;
                }
            }
            return;
        }
    }

    printf("no hid request response post_hid_report_packet((uint8_t*)\"");
    for (int i = 0; i < size; i++)
    {
        printf("\\x%02x", p[i]);
    }
    printf("\", %u);\n", size);
    send_disconnect(wii_controller.con_handle, ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION);
}

void handle_fake_wii_remote_data_channel(HID_REPORT_PACKET* packet, uint16_t size)
{
    switch (packet->report_type)
    {
        case HID_OUTPUT_REPORT:
        {
            printf("recv output report 0x%x\n", packet->report_id);
            switch (packet->report_id)
            {
                case WII_READ_MEMORY_AND_REGISTERS_REPORT:
                {
                    WII_READ_MEMORY_AND_REGISTERS_PACKET* report_packet = (WII_READ_MEMORY_AND_REGISTERS_PACKET*)packet;

                    printf("read_memory_and_registers address_space %x offset %x size %u\n", report_packet->address_space, bswap32(uint24_bytes_to_uint32(report_packet->offset_bytes) << 8), bswap16(report_packet->size));
                    break;
                }
                case WII_DATA_REPORTING_MODE_REPORT:
                {
                    WII_DATA_REPORTING_MODE_PACKET* drm_mode_packet = (WII_DATA_REPORTING_MODE_PACKET*)packet;
                    printf("data_reporting_mode continuous_reporting %u data_report_id %x\n", drm_mode_packet->continus_reporting_flag, drm_mode_packet->data_report_id);
                    break;
                }
                default:
                    printf("unhandled HID output report %x\n", packet->report_id);
                    break;
            }
            post_hid_request_reponse(packet, size);
            break;
        }
        default:
            printf("unhandled HID report type 0x%x\n", packet->report_type);
            break;
    }
}


int fake_wii_remote_packet_handler(uint8_t* packet, uint16_t size)
{
    dump_packet("recv", packet, size);

    bool handled = true;

    switch (packet[0])
    {
        case HCI_EVENT_PACKET:
            switch (packet[1])
            {
                case HCI_EVENT_CONNECTION_REQUEST:
                    handle_fake_wii_remote_connection_request((HCI_CONNECTION_REQUEST_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_CONNECTION_COMPLETE:
                    handle_fake_wii_remote_connection_complete((HCI_CONNECTION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_LINK_KEY_REQUEST:
                    handle_fake_wii_remote_link_key_request((HCI_LINK_KEY_REQUEST_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_PIN_CODE_REQUEST:
                    handle_fake_wii_remote_pin_code_request((HCI_PIN_CODE_REQUEST_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_AUTHENTICATION_COMPLETE:
                    handle_fake_wii_remote_authentication_complete((HCI_AUTHENTICATION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                default:
                    handled = false;
                    break;
            }
            break;
        case HCI_ACL_DATA_PACKET:
        {
            HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)packet;

            if (acl_packet->packet_boundary_flag == L2CAP_PB_FIRST_FLUSH)
            {
                L2CAP_PACKET* l2cap_packet = (L2CAP_PACKET*)packet;

                switch (l2cap_packet->channel)
                {
                    case L2CAP_SIGNAL_CHANNEL:
                        handle_fake_wii_remote_l2cap_signal_channel((L2CAP_SIGNAL_CHANNEL_PACKET*)l2cap_packet);
                        break;
                    case SDP_LOCAL_CID:
                        handle_fake_wii_remote_sdp_channel(l2cap_packet);
                        break;
                    case WII_DATA_LOCAL_CID:
                        handle_fake_wii_remote_data_channel((HID_REPORT_PACKET*)l2cap_packet->data, l2cap_packet->l2cap_size);
                        break;
                    default:
                        printf("unhandled l2cap channel %04x con_handle %04x\n", l2cap_packet->channel, l2cap_packet->con_handle);
                        break;
                }
            }
            else
            {
                printf("bad packet_boundary_flag %x\n", acl_packet->packet_boundary_flag);
            }
            break;
        }
        default:
            handled = false;
            break;
    }

    wii_bt_packet_handler(packet, size, handled);

    return 0;
}

void send_power_off_disconnect(uint16_t con_handle)
{
    send_disconnect(con_handle, ERROR_CODE_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF);
}

void send_disconnect(uint16_t con_handle, uint8_t reason)
{
    post_bt_packet(create_hci_disconnect_packet(con_handle, reason));
}

void connect_and_power_off()
{
    wii_controller.state = STATE_WII_CONSOLE_POWER_OFF_PENDING;
    //post_bt_packet(create_hci_create_connection_packet(wii_addr, 0x334, 1, false, 0, 1));
    post_bt_packet(create_hci_create_connection_packet(wii_addr, 0x18, 1, false, 0, 1));
}

void emulate_wii_remote()
{
    post_bt_packet(create_hci_write_default_link_policy_settings_packet(HCI_LINK_POLICY_ENABLE_ROLE_SWITCH));
    post_bt_packet(create_hci_secure_connections_host_support_packet(1));
    post_bt_packet(create_hci_write_class_of_device_packet(WII_REMOTE_COD));
    post_bt_packet(create_hci_write_local_name(WII_REMOTE_NAME));
    post_bt_packet(create_hci_current_iac_lap_packet(GAP_IAC_LIMITED_INQUIRY));
    post_bt_packet(create_hci_write_scan_eanble_packet(HCI_PAGE_SCAN_ENABLE | HCI_INQUIRY_SCAN_ENABLE));
    //post_bt_packet(create_hci_write_scan_eanble_packet(HCI_PAGE_SCAN_ENABLE));
    post_bt_packet(create_hci_write_authentication_enable(1));

    size_t size = BDA_SIZE;
    esp_err_t ret = nvs_get_blob(wii_controller.nvs_handle, WII_ADDR_BLOB_NAME, wii_addr, &size);
    if (0 && ret == ESP_OK && size == BDA_SIZE)
    {
        printf("stored wii at %s\n", bda_to_string(wii_addr));
        connect_and_power_off();
    }
    else
    {
        wii_controller.state = STATE_WII_CONSOLE_PAIRING_PENDING;
    }
}

