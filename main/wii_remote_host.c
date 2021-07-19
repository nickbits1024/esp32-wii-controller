#include "wii_controller.h"

//void handle_wii_remote_XXX(uint8_t* packet, uint16_t size)

#ifdef WII_REMOTE_HOST

bool found_wii_remote;
bd_addr_t wii_remote_addr;
SemaphoreHandle_t hid_sem;
uint8_t hid_wait;

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
    static DISCOVERED_DEVICE* discovered_devices;

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

        if (discovered_devices != NULL)
        {
            DISCOVERED_DEVICE* fd = discovered_devices;
            if (memcmp(addr, fd->addr, BDA_SIZE) == 0)
            {
                continue;
            }
        }

        printf("query addr %s psrm %u cod %06x clock_offset %04x\n", bda_to_string(addr), psrm, cod, clock_offset);

        post_bt_packet(create_hci_remote_name_request_packet(addr, psrm, true, clock_offset));

        if (discovered_devices == NULL)
        {
            discovered_devices = malloc(sizeof(DISCOVERED_DEVICE));
            memcpy(discovered_devices->addr, addr, BDA_SIZE);
            discovered_devices->next = NULL;
        }
        else
        {
            DISCOVERED_DEVICE* fd = discovered_devices;
            while (fd->next != NULL)
            {
                fd = fd->next;
            }
            fd->next = malloc(sizeof(DISCOVERED_DEVICE));
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
    printf("connection complete addr %s status 0x%02x con_handle 0x%x, link_type %u encrypted %u\n", bda_to_string(packet->addr), packet->status, packet->con_handle, packet->link_type, packet->encryption_enabled);
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

        post_bt_packet(create_hci_pin_code_request_reply_packet(packet->addr, pin, BDA_SIZE));
    }

}

void handle_wii_remote_connection_request(HCI_CONNECTION_REQUEST_EVENT_PACKET* packet)
{
    uint32_t cod = uint24_bytes_to_uint32(packet->class_of_device);
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
    printf("auth complete con_handle 0x%x status 0x%x\n", packet->con_handle, packet->status);

    if (wii_controller.state == STATE_WII_REMOTE_PAIRING_STARTED)
    {
        wii_controller.con_handle = packet->con_handle;
        wii_controller.state = packet->status == ERROR_CODE_SUCCESS ? STATE_WII_REMOTE_PAIRING_COMPLETE : 0;

        if (packet->status == ERROR_CODE_SUCCESS)
        {
            post_bt_packet(create_l2cap_connection_request_packet(wii_controller.con_handle, SDP_PSM, SDP_LOCAL_CID));

            //open_control_channel();
            //open_data_channel();
        }
    }
}

void handle_wii_remote_l2cap_connection_request(L2CAP_CONNECTION_REQUEST_PACKET* packet)
{
    //printf("l2cap connection request con_handle 0x%x psm 0x%x source_cid 0x%x\n", packet->con_handle, packet->psm, packet->source_cid);

    wii_controller.con_handle = packet->con_handle;
    uint16_t cid;
    switch (packet->psm)
    {
        case SDP_PSM:
            wii_controller.sdp_cid = packet->source_cid;
            cid = SDP_LOCAL_CID;
            printf("set wii_controller.con_handle 0x%x wii_controller.sdp_cid=0x%x\n", wii_controller.con_handle, wii_controller.sdp_cid);
            break;
        case WII_CONTROL_PSM:
            wii_controller.control_cid = packet->source_cid;
            cid = WII_CONTROL_LOCAL_CID;
            printf("set wii_controller.con_handle 0x%x wii_controller.control_cid=0x%x\n", wii_controller.con_handle, wii_controller.control_cid);
            break;
        case WII_DATA_PSM:
            wii_controller.data_cid = packet->source_cid;
            cid = WII_DATA_LOCAL_CID;
            printf("set wii_controller.con_handle 0x%x wii_controller.data_cid=0x%x\n", wii_controller.con_handle, wii_controller.data_cid);
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

    post_bt_packet(create_l2cap_connection_response_packet(packet->con_handle, packet->identifier, cid, packet->source_cid, 0, ERROR_CODE_SUCCESS));
    //post_l2ap_config_mtu_request(packet->con_handle, packet->source_cid, WII_MTU);
}

void handle_wii_remote_l2cap_connection_response(L2CAP_CONNECTION_RESPONSE_PACKET* response_packet)
{
    //printf("l2cap conn response con_handle 0x%x id 0x%x dest_cid 0x%x source_cid 0x%x result 0x%x status 0x%x\n",
    //    response_packet->con_handle, packet->identifier, response_packet->dest_cid, response_packet->source_cid, response_packet->result, response_packet->status);

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_RESULT_SUCCESS)
    {
        wii_controller.con_handle = response_packet->con_handle;
        switch (response_packet->source_cid)
        {
            case SDP_LOCAL_CID:
                wii_controller.sdp_cid = response_packet->dest_cid;
                printf("set wii_controller.con_handle 0x%x wii_controller.sdp_cid=0x%x\n", wii_controller.con_handle, wii_controller.sdp_cid);
                post_l2ap_config_mtu_flush_timeout_request(response_packet->con_handle, response_packet->dest_cid, WII_SDP_MTU, WII_SDP_FLUSH_TIMEOUT);
                break;
            case WII_CONTROL_LOCAL_CID:
                wii_controller.control_cid = response_packet->dest_cid;
                printf("set wii_controller.con_handle 0x%x wii_controller.control_cid=0x%x\n", wii_controller.con_handle, wii_controller.control_cid);
                post_l2ap_config_mtu_flush_timeout_request(response_packet->con_handle, response_packet->dest_cid, WII_MTU, WII_SDP_FLUSH_TIMEOUT);
                open_data_channel();
                break;
            case WII_DATA_LOCAL_CID:
                wii_controller.data_cid = response_packet->dest_cid;
                printf("set wii_controller.con_handle 0x%x wii_controller.data_cid=0x%x\n", wii_controller.con_handle, wii_controller.data_cid);
                post_l2ap_config_mtu_flush_timeout_request(response_packet->con_handle, response_packet->dest_cid, WII_MTU, WII_SDP_FLUSH_TIMEOUT);
                break;
        }
    }
}

void send_led_report(uint8_t led_flags)
{
    uint8_t report[3];
    report[0] = HID_OUTPUT_REPORT;
    report[1] = WII_LED_REPORT;
    report[2] = led_flags;

    post_bt_packet(create_output_report_packet(wii_controller.con_handle, wii_controller.data_cid, report, sizeof(report)));
}

void handle_wii_remote_l2cap_command_reject(L2CAP_COMMAND_REJECT_PACKET* packet)
{
    //printf("l2cap cmd rejected con_handle 0x%x id 0x%x reason 0x%02x\n", packet->con_handle, packet->identifier, ->reason);
}

void handle_wii_remote_l2cap_disconnection_request(L2CAP_DISCONNECTION_REQUEST_PACKET* packet)
{
    //printf("l2cap disconnect request con_handle 0x%x id 0x%x dest_cid 0x%x source_cid 0x%x\n", packet->con_handle, packet->identifier, packet->dest_cid, packet->source_cid);

    post_bt_packet(create_l2cap_disconnection_response_packet(packet->con_handle, packet->identifier, packet->dest_cid, packet->source_cid));
    //post_bt_packet(create_hci_disconnect_packet(packet->con_handle, ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION));
}

void handle_wii_remote_l2cap_disconnection_response(L2CAP_DISCONNECTION_RESPONSE_PACKET* packet)
{
    switch (packet->source_cid)
    {
    case SDP_LOCAL_CID:
        open_control_channel();        
        break;
    }
}

void handle_wii_remote_l2cap_config_request(L2CAP_CONFIG_REQUEST_PACKET* request_packet)
{
    //uint16_t options_size = request_packet->payload_size - 4;

    uint16_t cid;
    switch (request_packet->dest_cid)
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
        printf("l2cap config request no matching cid for 0x%x\n", request_packet->dest_cid);
        return;
    }

    // printf("l2cap config request con_handle 0x%x dest_cid 0x%x source_cid 0x%x options_size %u options", request_packet->con_handle, request_packet->dest_cid, cid, options_size);
    // dump_l2cap_config_options(request_packet->options, options_size);
    // printf("\n");

    BT_PACKET_ENVELOPE* env = create_l2cap_config_response_packet(request_packet->con_handle, request_packet->identifier, cid, request_packet->flags, 0);

    //L2CAP_CONFIG_RESPONSE_PACKET* response_packet = (L2CAP_CONFIG_RESPONSE_PACKET*)env->packet;

    //memcpy(response_packet->options, request_packet->options, options_size);

    post_bt_packet(env);

    switch (request_packet->dest_cid)
    {
        case SDP_LOCAL_CID:
            wii_remote_sdp_query();
            post_bt_packet(create_l2cap_disconnection_request_packet(request_packet->con_handle, wii_controller.sdp_cid, SDP_LOCAL_CID));
            break;
    }
}

void handle_wii_remote_l2cap_config_response(L2CAP_CONFIG_RESPONSE_PACKET* packet)
{
    // uint16_t options_size = packet->payload_size - 6;

    // printf("l2cap config response con_handle 0x%x source_cid 0x%x result 0x%x options_size %u options", packet->con_handle, packet->source_cid, packet->result, options_size);
    // dump_l2cap_config_options(packet->options, options_size);
    // printf("\n");

}

void handle_wii_remote_l2cap_signal_channel(L2CAP_SIGNAL_CHANNEL_PACKET* packet)
{
    switch (packet->code)
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
        case L2CAP_DISCONNECTION_REQUEST:
            handle_wii_remote_l2cap_disconnection_request((L2CAP_DISCONNECTION_REQUEST_PACKET*)packet);
            break;
        case L2CAP_DISCONNECTION_RESPONSE:
            handle_wii_remote_l2cap_disconnection_response((L2CAP_DISCONNECTION_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_COMMAND_REJECT:
            handle_wii_remote_l2cap_command_reject((L2CAP_COMMAND_REJECT_PACKET*)packet);
            break;
        default:
            printf("unhandled signal channel code 0x%02x\n", packet->code);
            break;
    }
}

int sdp_response_index = -1;
int sdp_response_fragment_index;

void handle_wii_remote_sdp_channel(L2CAP_PACKET* packet)
{
    sdp_response_index++;
    sdp_response_fragment_index = 0;
    printf("sdp response %d.%d \"", sdp_response_index, sdp_response_fragment_index);
    uint16_t data_size = packet->hci_acl_size - (sizeof(L2CAP_PACKET) - sizeof(HCI_ACL_PACKET));
    for (int i = 0; i < data_size; i++)
    {
        printf("\\x%02x", packet->data[i]);
    }
    printf("\", %u\n", data_size);
}

void handle_wii_remote_sdp_channel_fragment(HCI_ACL_PACKET* packet)
{
    sdp_response_fragment_index++;

    printf("sdp response %d.%d \"", sdp_response_index, sdp_response_fragment_index);
    for (int i = 0; i < packet->hci_acl_size; i++)
    {
        printf("\\x%02x", packet->data[i]);
    }
    printf("\", %u\n", packet->hci_acl_size);
}

void dump_button(uint16_t buttons, uint16_t button, const char* name)
{
    if ((buttons & button) != 0)
    {
        printf(" %s", name);
    }
}

void handle_wii_remote_remote_data(HID_REPORT_PACKET* packet, uint16_t size)
{
    static uint16_t last_buttons;
    switch (packet->report_type)
    {
        case HID_INPUT_REPORT:
        {
            if ((wii_remote_connection_complete && size == 4 && memcmp(packet, "\xa1\x30\x00\x00", 4) == 0) ||
                (size == 19 && memcmp(packet, (uint8_t*)"\xa1\x33\x40\x00\x7f\x81\x9c\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 19) == 0))
            {
                break;
            }
            printf("recv input hid report \"");
            uint8_t* p = (uint8_t*)packet;
            for (int i = 0; i < size; i++)
            {
                printf("\\x%02x", p[i]);
            }
            printf("\", %u\n", size);
            uint16_t buttons = read_uint16_be(packet->data);

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
            if (hid_wait != 0 && packet->report_id == hid_wait)
            {
                xSemaphoreGive(hid_sem);
            }
            break;
        }
        default:
            printf("unhandled hid report 0x%x\n", packet->report_type);
            break;
    }
}

void wii_remote_packet_handler(uint8_t* packet, uint16_t size)
{
    //dump_packet("recv", packet, size);

    HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)packet;
    HCI_EVENT_PACKET* event_packet = (HCI_EVENT_PACKET*)packet;

    switch (acl_packet->type)
    {
        case HCI_EVENT_PACKET_TYPE:
            switch (event_packet->event_code)
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
                    break;
            }
            break;
        case HCI_ACL_PACKET_TYPE:
        {
            HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)packet;
            L2CAP_PACKET* l2cap_packet = (L2CAP_PACKET*)packet;

            static uint16_t last_channel = INVALID_HANDLE_VALUE;

            if (acl_packet->packet_boundary_flag == L2CAP_PB_FIRST_FLUSH)
            {
                last_channel = l2cap_packet->channel;

                switch (l2cap_packet->channel)
                {
                    case L2CAP_SIGNAL_CHANNEL:
                        handle_wii_remote_l2cap_signal_channel((L2CAP_SIGNAL_CHANNEL_PACKET*)packet);
                        break;
                    case WII_DATA_LOCAL_CID:
                        handle_wii_remote_remote_data((HID_REPORT_PACKET*)l2cap_packet->data, l2cap_packet->l2cap_size);
                        break;
                    case SDP_LOCAL_CID:
                        handle_wii_remote_sdp_channel((L2CAP_PACKET*)packet);
                        break;
                    default:
                        printf("unhandled l2cap channel 0x%x con_handle 0x%x\n", l2cap_packet->channel, l2cap_packet->con_handle);
                        break;
                }
            }
            else if (acl_packet->packet_boundary_flag == L2CAP_PB_FRAGMENT)
            {
                switch (last_channel)
                {
                    case SDP_LOCAL_CID:
                        handle_wii_remote_sdp_channel_fragment((HCI_ACL_PACKET*)packet);
                        break;
                    default:
                        printf("unhandled acl fragment in channel 0x%x\n", last_channel);
                        break;
                }
            }
            // else
            // {
            //     printf("bad packet_boundary_flag 0x%x\n", acl_packet->packet_boundary_flag);
            // }
            break;
        }
        default:
            break;
    }
}

void wii_remote_host()
{
    post_bt_packet(create_hci_write_scan_enable_packet(HCI_PAGE_SCAN_ENABLE));
    post_bt_packet(create_hci_write_pin_type_packet(HCI_FIXED_PIN_TYPE));
    post_bt_packet(create_hci_write_class_of_device_packet(WII_COD));
    find_wii_remote();
}

void start_wii_remote_pairing(const bd_addr_t addr)
{
    memcpy(wii_remote_addr, addr, BDA_SIZE);
    post_bt_packet(create_hci_create_connection_packet(addr, 0x08, 1, false, 0, 0));
}

void post_wii_remote_hid_report_packet(uint8_t return_report_id, char* report, uint16_t report_size)
{
    hid_wait = return_report_id;
    post_hid_report_packet((uint8_t*)report, report_size);
    xSemaphoreTake(hid_sem, portMAX_DELAY);
}

void wii_remote_connected_task(void* p)
{
    //send_led_report(WII_REMOTE_LED_4);
    
    post_wii_remote_hid_report_packet(0x21, "\xa2\x17\x00\x00\x17\x70\x00\x01", 8);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x12\x06\x30", 4);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x1a\x02", 3);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x11\x12", 3);
    post_wii_remote_hid_report_packet(0x21, "\xa2\x17\x00\x00\x00\x2a\x00\x38", 8);
    post_wii_remote_hid_report_packet(0x21, "\xa2\x17\x00\x00\x00\x62\x00\x38", 8);
    post_wii_remote_hid_report_packet(0x21, "\xa2\x17\x00\x00\x00\x00\x00\x2a", 8);
    post_wii_remote_hid_report_packet(0x20, "\xa2\x15\x00", 3);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x13\x06", 3);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x1a\x06", 3);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x30\x01\x01\x00\x15\x00\x14\xbc\xc4\x00\x00\x00\x07\x00\x00\x00\x02\x81", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x00\x09\x02\x00\x00\x71\x01\x00\xaa\x00\x64\x13\xcc\x90\x00\xaa\x84\x81", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x30\x01\x01\x00\x15\x00\x59\x0d\x78\x81\x14\x2c\xbc\x81\x5a\x97\x28\x81", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x1a\x02\x63\x03\xe0\x00\x11\xc2\x6c\x00\x00\x00\x06\x81\x14\x3b\x04\x81", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x33\x01\x03\x00\x00\x81\x14\x3b\x0c\x90\x00\x68\x5c\x90\x00\x68\x80\x81", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x16\x04\xb0\x00\x30\x01\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 23);
    post_wii_remote_hid_report_packet(0x22, "\xa2\x12\x06\x33", 4);

    vTaskDelete(NULL);
}

void wii_remote_connected()
{
    hid_sem = xSemaphoreCreateBinary();

    xTaskCreate(wii_remote_connected_task, "wii_remote_connected", 8000, NULL, 1, NULL);
 }

void wii_remote_sdp_query()
{
    //post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)"\x02\x00\x00\x00\x08\x35\x03\x19\x11\x24\x00\x15\x00", 13);
    //post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)"\x04\x00\x01\x00\x0e\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x00", 19);
    //post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)"\x04\x00\x02\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x00\x76", 21);
    //post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)"\x04\x00\x03\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x00\xec", 21);
    //post_sdp_packet(AUTO_L2CAP_SIZE, (uint8_t*)"\x04\x00\x04\x00\x10\x00\x01\x00\x00\x00\xf0\x35\x05\x0a\x00\x00\xff\xff\x02\x01\x62", 21);
}

#endif