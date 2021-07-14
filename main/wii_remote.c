#include "wii_controller.h"

//void handle_wii_remote_XXX(uint8_t* packet, uint16_t size)

#ifdef WII_REMOTE_TEST

typedef struct _FOUND_DEVICE
{
    bd_addr_t addr;
    struct _FOUND_DEVICE* next;
} FOUND_DEVICE;

bool found_wii_remote;
FOUND_DEVICE* found_devices;
bd_addr_t wii_remote_addr;

bool wii_remote_connection_complete;
// uint16_t wii_controller.con_handle;
// uint16_t wii_controller.control_cid;
// uint16_t wii_controller.data_cid;

void start_wii_remote_pairing(const bd_addr_t addr);
void send_led_report(uint8_t leds);
void wii_remote_sdp_query();
void wii_remote_connected();

void find_wii_remote()
{
    printf("scanning for wii remote...\n");
    wii_controller.state = STATE_WII_REMOTE_PAIRING_PENDING;
    post_bt_packet(create_hci_inquiry_packet(GAP_IAC_LIMITED_INQUIRY, 30, 0));
}

void handle_wii_remote_inquiry_result(uint8_t* packet, uint16_t size)
{
    uint8_t num_responses = packet[3];
    uint8_t* p = packet + 4;
    for (int i = 0; i < num_responses; i++)
    {
        bd_addr_t addr;
        //read_bda(p, addr);
        memcpy(addr, p, BDA_SIZE);
        uint8_t psrm = p[6];
        uint32_t cod = read_uint24(p + 9);
        uint16_t clock_offset = read_uint16_be(p + 12) & 0x7fff;

        if (found_devices != NULL)
        {
            FOUND_DEVICE* fd = found_devices;
            if (memcmp(addr, fd->addr, BDA_SIZE) == 0)
            {
                continue;
            }
        }

        printf("query addr %s psrm %u cod %06x clock_offset %04x\n", bda_to_string(addr), psrm, cod, clock_offset);

        post_bt_packet(create_hci_remote_name_request_packet(addr, psrm, true, clock_offset));

        if (found_devices == NULL)
        {
            found_devices = malloc(sizeof(FOUND_DEVICE));
            memcpy(found_devices->addr, addr, BDA_SIZE);
            found_devices->next = NULL;
        }
        else
        {
            FOUND_DEVICE* fd = found_devices;
            while (fd->next != NULL)
            {
                fd = fd->next;
            }
            fd->next = malloc(sizeof(FOUND_DEVICE));
            memcpy(fd->next->addr, addr, BDA_SIZE);
            fd->next->next = NULL;
        }
    }
}

void handle_wii_remote_inquiry_complete(uint8_t* packet, uint16_t size)
{
    if (!found_wii_remote)
    {
        find_wii_remote();
    }
}

void handle_wii_remote_remote_name_request_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    bd_addr_t addr;
    //read_bda(packet + 4, addr);
    memcpy(addr, packet + 4, BDA_SIZE);
    char* name = (char*)(packet + 10);

    printf("found device status 0x%02x %s name %s\n", status, bda_to_string(addr), name);

    if (strcmp(name, WII_REMOTE_NAME) == 0)
    {
        found_wii_remote = true;
        printf("pairing %s...\n", name);

        start_wii_remote_pairing(addr);
    }
}

void handle_wii_remote_connection_complete(HCI_CONNECTION_COMPLETE_EVENT_PACKET* packet)
{
    printf("connection complete addr %s status 0x%02x handle 0x%04x, link_type %u encrypted %u\n", bda_to_string(packet->addr), packet->status, packet->con_handle, packet->link_type, packet->encryption_enabled);
    if (packet->status == 0)
    {
        if (wii_controller.state == STATE_WII_REMOTE_PAIRING_PENDING)
        {
            wii_controller.state = STATE_WII_REMOTE_PAIRING_STARTED;
            post_bt_packet(create_hci_authentication_requested_packet(packet->con_handle));
        }
        else
        {
            //open_data_channel(packet->con_handle);
        }
    }
}

void handle_wii_remote_link_key_request(HCI_LINK_KEY_REQUEST_EVENT_PACKET* packet)
{
    //reverse_bda(packet->addr);

    printf("link key request from %s...\n", bda_to_string(packet->addr));

    if (wii_controller.state == STATE_WII_REMOTE_PAIRING_STARTED)
    {
        printf("rejecting link key request from %s...\n", bda_to_string(packet->addr));
        post_bt_packet(create_hci_link_key_request_negative_packet(packet->addr));
    }
}

void handle_wii_remote_pin_code_request(HCI_PIN_CODE_REQUEST_EVENT_PACKET* packet)
{
    //reverse_bda(packet->addr);

    printf("pin code request from %s...\n", bda_to_string(packet->addr));

    if (wii_controller.state == STATE_WII_REMOTE_PAIRING_STARTED)
    {
        uint8_t pin[6];
        //write_bda(pin, device_addr);
        memcpy(pin, device_addr, BDA_SIZE);
        printf("sending pin code %02x %02x %02x %02x %02x %02x\n",
            pin[0], pin[1], pin[2], pin[3], pin[4], pin[5]);

        post_bt_packet(create_hci_pin_code_reply_packet(packet->addr, pin, BDA_SIZE));
    }

}

void handle_wii_remote_connection_request(HCI_CONNECTION_REQUEST_EVENT_PACKET* packet)
{
    uint32_t cod = cod_to_uint32(packet->class_of_device);
    printf("connection request from %s cod %06x type %u\n", bda_to_string(packet->addr), cod, packet->link_type);

    if (packet->link_type == HCI_LINK_TYPE_ACL && cod == WII_REMOTE_COD)
    {
        printf("accepting wii remote connection...\n");
        wii_controller.state = STATE_WII_REMOTE_PAIRING_COMPLETE;
        post_bt_packet(create_hci_accept_connection_request_packet(packet->addr, HCI_ROLE_MASTER));
    }
    else
    {
        printf("rejecting unknown connection...\n");
        post_bt_packet(create_hci_reject_connection_request_packet(packet->addr, ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR));
    }
}

void handle_wii_remote_authentication_complete(HCI_AUTHENTICATION_COMPLETE_EVENT_PACKET* packet)
{
    printf("auth complete handle 0x%x status 0x%x\n", packet->con_handle, packet->status);

    if (wii_controller.state == STATE_WII_REMOTE_PAIRING_STARTED)
    {
        wii_controller.con_handle = packet->con_handle;
        wii_controller.state = packet->status == ERROR_CODE_SUCCESS ? STATE_WII_REMOTE_PAIRING_COMPLETE : 0;

        if (packet->status == ERROR_CODE_SUCCESS)
        {
            open_data_channel(packet->con_handle);
        }
    }
}

void handle_wii_remote_l2cap_connection_response(L2CAP_CONNECTION_RESPONSE_PACKET* response_packet)
{
    printf("l2cap conn response handle 0x%04x remote_cid 0x%x local_cid 0x%x result 0x%04x status 0x%x\n",
        response_packet->con_handle, response_packet->remote_cid, response_packet->local_cid, response_packet->result, response_packet->status);

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_PENDING)
    {
        post_l2ap_config_mtu_request(response_packet->con_handle, response_packet->remote_cid);
    }

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_SUCCESS)
    {
        wii_controller.con_handle = response_packet->con_handle;
        switch (response_packet->local_cid)
        {
            case SDP_LOCAL_CID:
                wii_controller.sdp_cid = response_packet->remote_cid;
                printf("set wii_controller.con_handle %04x wii_controller.control_cid=0x%x\n", wii_controller.con_handle, wii_controller.sdp_cid);
                break;
            case WII_CONTROL_LOCAL_CID:
                wii_controller.control_cid = response_packet->remote_cid;
                printf("set wii_controller.con_handle %04x wii_controller.control_cid=0x%x\n", wii_controller.con_handle, wii_controller.control_cid);
                break;
            case WII_DATA_LOCAL_CID:
                wii_controller.data_cid = response_packet->remote_cid;
                printf("set wii_controller.con_handle %04x wii_controller.data_cid=0x%x\n", wii_controller.con_handle, wii_controller.data_cid);
                break;
        }
    }
}

void handle_wii_remote_l2cap_connection_request(L2CAP_CONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap connection request handle 0x%x psm 0x%x local_cid %04x\n", packet->con_handle, packet->psm, packet->local_cid);

    wii_controller.con_handle = packet->con_handle;
    uint16_t cid;
    switch (packet->psm)
    {
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
    post_l2ap_config_mtu_request(packet->con_handle, packet->local_cid);
}

void send_led_report(uint8_t leds)
{
    uint8_t report[3];
    report[0] = HID_OUTPUT_REPORT;
    report[1] = WII_REMOTE_LED_REPORT;
    report[2] = leds;

    post_bt_packet(create_output_report_packet(wii_controller.con_handle, wii_controller.data_cid, report, sizeof(report)));
}

void handle_wii_remote_l2cap_command_reject(L2CAP_COMMAND_REJECT_PACKET* packet)
{
    printf("l2cap cmd rejected handle 0x%04x reason 0x%02x\n", packet->con_handle, packet->reason);
}

void handle_wii_remote_l2cap_disconnection_request(L2CAP_DISCONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap disconnect request 0x%04x remote_cid 0x%0x local_cid 0x%x\n", packet->con_handle, packet->remote_cid, packet->local_cid);

    post_bt_packet(create_l2cap_disconnection_response_packet(packet->con_handle, packet->identifier, packet->remote_cid, packet->local_cid));
    //post_bt_packet(create_hci_disconnect_packet(packet->con_handle, ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION));
}

void handle_wii_remote_l2cap_config_request(L2CAP_CONFIG_REQUEST_PACKET* request_packet)
{
    uint16_t options_size = request_packet->payload_size - 4;

    uint16_t cid;
    switch (request_packet->remote_cid)
    {
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

void handle_wii_remote_l2cap_config_response(L2CAP_CONFIG_RESPONSE_PACKET* packet)
{
    uint16_t options_size = packet->payload_size - 6;

    printf("l2cap config response handle 0x%04x local_cid 0x%x result 0x%x options_size %u options", packet->con_handle, packet->local_cid, packet->result, options_size);
    dump_l2cap_config_options(packet->options, options_size);
    printf("\n");
}

void handle_wii_remote_l2cap_signal_channel(uint8_t* packet, uint16_t size, uint16_t con_handle)
{
    uint8_t code = packet[9];
    switch (code)
    {
        case L2CAP_CONNECTION_REQUEST:
            handle_wii_remote_l2cap_connection_request((L2CAP_CONNECTION_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONNECTION_RESPONSE:
            handle_wii_remote_l2cap_connection_response((L2CAP_CONNECTION_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_CONFIG_REQUEST:
            handle_wii_remote_l2cap_config_request((L2CAP_CONFIG_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONFIG_RESPONSE:
            handle_wii_remote_l2cap_config_response((L2CAP_CONFIG_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_COMMAND_REJECT:
            handle_wii_remote_l2cap_command_reject((L2CAP_COMMAND_REJECT_PACKET*)packet);
            break;
        case L2CAP_DISCONNECTION_REQUEST:
            handle_wii_remote_l2cap_disconnection_request((L2CAP_DISCONNECTION_REQUEST_PACKET*)packet);
            break;
        default:
            printf("unhandled signal channel code 0x%02x\n", code);
            break;
    }
}

void handle_wii_remote_sdp_channel(L2CAP_PACKET* packet)
{
    printf("sdp data { ");
    if (packet->l2cap_size > 0)
    {
        printf("0x%02x", packet->data[0]);
    }
    for (int i = 1; i < packet->l2cap_size; i++)
    {
        printf(", 0x%02x", packet->data[i]);
    }
    printf(" }\n");
}

void dump_button(uint16_t buttons, uint16_t button, const char* name)
{
    if ((buttons & button) != 0)
    {
        printf(" %s", name);
    }
}

void handle_wii_remote_remote_data(L2CAP_PACKET* packet)
{
    static uint16_t last_buttons;
    if (packet->data[0] == HID_INPUT_REPORT)
    {
        uint16_t buttons = read_uint16_be(packet->data + 2);

        if (buttons != last_buttons)
        {
            if (buttons != 0)
            {
                printf("remote buttons: ");
                dump_button(buttons, WII_BUTTON_LEFT, "Left");
                dump_button(buttons, WII_BUTTON_RIGHT, "Right");
                dump_button(buttons, WII_BUTTON_UP, "Up");
                dump_button(buttons, WII_BUTTON_DOWN, "Down");
                dump_button(buttons, WII_BUTTON_A, "A");
                dump_button(buttons, WII_BUTTON_B, "B");
                dump_button(buttons, WII_BUTTON_PLUS, "Plus");
                dump_button(buttons, WII_BUTTON_HOME, "Home");
                dump_button(buttons, WII_BUTTON_MINUS, "Minus");
                dump_button(buttons, WII_BUTTON_ONE, "One");
                dump_button(buttons, WII_BUTTON_TWO, "Two");
                printf("\n");
            }
            last_buttons = buttons;
        }

        if (!wii_remote_connection_complete)
        {
            wii_remote_connected();
            wii_remote_connection_complete = true;
        }
    }
}

int wii_remote_packet_handler(uint8_t* packet, uint16_t size)
{
    dump_packet("recv", packet, size);

    bool handled = true;

    switch (packet[0])
    {
        case HCI_EVENT_PACKET:
            switch (packet[1])
            {
                case HCI_EVENT_INQUIRY_RESULT:
                    handle_wii_remote_inquiry_result(packet, size);
                    break;
                case HCI_EVENT_INQUIRY_COMPLETE:
                    handle_wii_remote_inquiry_complete(packet, size);
                    break;
                case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                    handle_wii_remote_remote_name_request_complete(packet, size);
                    break;
                case HCI_EVENT_CONNECTION_COMPLETE:
                    handle_wii_remote_connection_complete((HCI_CONNECTION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_LINK_KEY_REQUEST:
                    handle_wii_remote_link_key_request((HCI_LINK_KEY_REQUEST_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_PIN_CODE_REQUEST:
                    handle_wii_remote_pin_code_request((HCI_PIN_CODE_REQUEST_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_AUTHENTICATION_COMPLETE:
                    handle_wii_remote_authentication_complete((HCI_AUTHENTICATION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_CONNECTION_REQUEST:
                    handle_wii_remote_connection_request((HCI_CONNECTION_REQUEST_EVENT_PACKET*)packet);
                    break;
                default:
                    handled = false;
                    break;
            }
            break;
        case HCI_ACL_DATA_PACKET:
        {
            uint16_t con_handle = read_uint16(packet + 1) & HCI_CON_HANDLE_MASK;
            uint16_t channel = read_uint16(packet + 7);

            switch (channel)
            {
                case L2CAP_SIGNAL_CHANNEL:
                    handle_wii_remote_l2cap_signal_channel(packet, size, con_handle);
                    break;
                case WII_DATA_LOCAL_CID:
                    handle_wii_remote_remote_data((L2CAP_PACKET*)packet);
                    break;
                case SDP_LOCAL_CID:
                    handle_wii_remote_sdp_channel((L2CAP_PACKET*)packet);
                    break;
                default:
                    printf("unhandled l2cap channel %04x con_handle %04x\n", channel, con_handle);
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

void wii_remote_test()
{
    post_bt_packet(create_hci_write_scan_eanble_packet(HCI_PAGE_SCAN_ENABLE));
    find_wii_remote();
}

void start_wii_remote_pairing(const bd_addr_t addr)
{
    memcpy(wii_remote_addr, addr, BDA_SIZE);
    post_bt_packet(create_hci_create_connection_packet(addr, 0x334, 1, false, 0, 0));
}

void wii_remote_connected()
{    
    send_led_report(WII_REMOTE_LED_4);
    post_bt_packet(create_l2cap_connection_request_packet(wii_controller.con_handle, SDP_PSM, SDP_LOCAL_CID));
}

void post_sdp_packet(uint8_t* data, uint16_t data_size)
{
    post_bt_packet(create_l2cap_packet(wii_controller.con_handle, wii_controller.sdp_cid, data, data_size));
}

void sdp_query()
{
    uint8_t tx0[] = { 0x02, 0x00, 0x00, 0x00, 0x08, 0x35, 0x03, 0x19, 0x11, 0x24, 0x00, 0x15, 0x00 };

    post_sdp_packet(tx0, sizeof(tx0));
}

#endif