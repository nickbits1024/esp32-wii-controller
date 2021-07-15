#include "wii_controller.h"

#define WII_ADDR_BLOB_NAME  "wii_addr"


bd_addr_t wii_addr;

void send_disconnect(uint16_t con_handle, uint8_t reason);
void send_power_off_disconnect(uint16_t con_handle);

void handle_fake_wii_remote_connection_request(HCI_CONNECTION_REQUEST_EVENT_PACKET* packet)
{
    uint32_t cod = cod_to_uint32(packet->class_of_device);
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
        switch (wii_controller.state)
        {
            case STATE_WII_CONSOLE_PAIRING_PENDING:
                wii_controller.con_handle = packet->con_handle;
                memcpy(wii_addr, packet->addr, BDA_SIZE);

                wii_controller.state = STATE_WII_CONSOLE_PAIRING_STARTED;
                printf("pairing started...\n");
                post_bt_packet(create_hci_authentication_requested_packet(packet->con_handle));
                break;
            case STATE_WII_CONSOLE_POWER_OFF_PENDING:
                //send_power_off_disconnect(packet->con_handle);
                wii_controller.state = STATE_WII_CONSOLE_POWER_OFF_CONNECTED;
                open_data_channel(packet->con_handle);
                break;
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
        case STATE_WII_CONSOLE_POWER_OFF_PENDING:
        {
            uint8_t pin[6];
            //write_bda(pin, device_addr);
            memcpy(pin, packet->addr, BDA_SIZE);
            printf("sending pin code %02x %02x %02x %02x %02x %02x\n",
                pin[0], pin[1], pin[2], pin[3], pin[4], pin[5]);

            post_bt_packet(create_hci_pin_code_reply_packet(packet->addr, pin, BDA_SIZE));
            break;
        default:
            break;
        }
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
            post_l2ap_config_mtu_request(response_packet->con_handle, response_packet->remote_cid, WII_MTU);
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
    post_l2ap_config_mtu_request(packet->con_handle, packet->local_cid, WII_MTU);
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
} SDP_REQUEST_RESPONSE;

SDP_REQUEST_RESPONSE sdp_request_responses[] = 
{
    { "\x02\x00\x00\x00\x08\x35\x03\x19\x11\x24\x00\x15\x00", 13, "\x03\x00\x00\x00\x09\x00\x01\x00\x01\x00\x01\x00\x00\x00", 14 },
    { "\x04\x00\x01\x00\x0e\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x00", 19, "\x05\x00\x01\x00\x7b\x00\x76\x36\x01\xcc\x09\x00\x00\x0a\x00\x01\x00\x00\x09\x00\x01\x35\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 128 },
    { NULL, 0, "\x19\x11\x24\x09\x00\x04\x35\x0d\x35\x06\x19\x01\x00\x09\x00\x11\x35\x03\x19\x00\x11\x09\x00\x05\x35\x03\x19", 27 },
    { NULL, 0, "\x10\x02\x09\x00\x06\x35\x09\x09\x65\x6e\x09\x00\x6a\x09\x01\x00\x09\x00\x09\x35\x08\x35\x06\x19\x11\x24\x09", 27 },
    { NULL, 0, "\x01\x00\x09\x00\x0d\x35\x0f\x35\x0d\x35\x06\x19\x01\x00\x09\x00\x13\x35\x03\x19\x00\x11\x09\x01\x00\x25\x13", 27 },
    { NULL, 0, "\x4e\x69\x6e\x74\x65\x6e\x64\x6f\x20\x52\x56\x4c\x2d\x43\x4e\x54\x2d\x30\x31\x09\x01\x02\x00\x76", 24 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

void handle_fake_wii_remote_sdp_channel(L2CAP_PACKET* packet)
{
    int sdp_tx_responses_size = sizeof(sdp_request_responses) / sizeof(SDP_REQUEST_RESPONSE);
    for (int i = 0; i < sdp_tx_responses_size; i++)
    {
        const SDP_REQUEST_RESPONSE* rr = &sdp_request_responses[i];
        if (packet->l2cap_size == rr->request_size && memcmp(packet->data, rr->request, packet->l2cap_size) == 0)
        {
            post_sdp_packet((uint8_t*)rr->response, rr->response_size);
            for (int j = i + 1; j < sdp_tx_responses_size; j++)
            {
                const SDP_REQUEST_RESPONSE* rr2 = &sdp_request_responses[j];
                if (rr2->request_size == 0 && rr2->request  == NULL && rr2->response_size > 0)
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

    printf("no sdp response request post_sdp_packet((uint8_t*)\"");
    for (int i = 0; i < packet->l2cap_size; i++)
    {
        printf("\\x%02x", packet->data[i]);
    }
    printf("\", %u);\n", packet->l2cap_size);
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
            L2CAP_PACKET* l2cap_packet = (L2CAP_PACKET*)packet;

            switch (l2cap_packet->channel)
            {
                case L2CAP_SIGNAL_CHANNEL:
                    handle_fake_wii_remote_l2cap_signal_channel((L2CAP_SIGNAL_CHANNEL_PACKET*)l2cap_packet);
                    break;
                case SDP_LOCAL_CID:
                    handle_fake_wii_remote_sdp_channel(l2cap_packet);
                    break;
                default:                
                    printf("unhandled l2cap channel %04x con_handle %04x\n", l2cap_packet->channel, l2cap_packet->con_handle);
                    break;
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
    post_bt_packet(create_hci_create_connection_packet(wii_addr, 0x334, 1, false, 0, 1));
}

void emulate_wii_remote()
{
    post_bt_packet(create_hci_write_default_link_policy_settings_packet(HCI_LINK_POLICY_ENABLE_ROLE_SWITCH));
    post_bt_packet(create_hci_secure_connections_host_support_packet(1));
    post_bt_packet(create_hci_write_class_of_device_packet(WII_REMOTE_COD));
    post_bt_packet(create_hci_write_local_name(WII_REMOTE_NAME));
    post_bt_packet(create_hci_current_iac_lap_packet(GAP_IAC_LIMITED_INQUIRY));
    post_bt_packet(create_hci_write_scan_eanble_packet(HCI_PAGE_SCAN_ENABLE | HCI_INQUIRY_SCAN_ENABLE));
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

