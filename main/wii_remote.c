#include "wii_controller.h"

//void handle_XXX(uint8_t* packet, uint16_t size)

typedef struct _FOUND_DEVICE
{
    bd_addr_t addr;
    struct _FOUND_DEVICE* next;
} FOUND_DEVICE;

bool found_wii_remote;
FOUND_DEVICE* found_devices;
bd_addr_t wii_remote_addr;

bool wii_remote_connected;
uint16_t wii_remote_con_handle;
uint16_t wii_remote_data_cid;

void start_wii_remote_pairing(const bd_addr_t addr);
void send_led_report(uint8_t leds);

void find_wii_remote()
{
    printf("scanning for wii remote...\n");
    post_bt_packet(create_hci_inquiry_packet(GAP_IAC_LIMITED_INQUIRY, 5, 0));
}

void handle_inquiry_result(uint8_t* packet, uint16_t size)
{
    uint8_t num_responses = packet[3];
    uint8_t* p = packet + 4;
    for (int i = 0; i < num_responses; i++)
    {
        bd_addr_t addr;
        read_bda(p, addr);
        uint8_t psrm = p[6];
        uint32_t cod = read_uint24(p + 9);
        uint16_t clock_offset = read_uint16_be(p + 12) & 0x7fff;

        if (found_devices != NULL)
        {
            FOUND_DEVICE* fd = found_devices;
            if (memcmp(addr, fd->addr, BD_ADDR_LEN) == 0)
            {
                continue;
            }
        }

        printf("query addr %s psrm %u cod %06x clock_offset %04x\n", bda_to_string(addr), psrm, cod, clock_offset);

        post_bt_packet(create_hci_remote_name_request_packet(addr, psrm, true, clock_offset));

        if (found_devices == NULL)
        {
            found_devices = malloc(sizeof(FOUND_DEVICE));
            memcpy(found_devices->addr, addr, BD_ADDR_LEN);
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
            memcpy(fd->next->addr, addr, BD_ADDR_LEN);
            fd->next->next = NULL;
        }
    }
}

void handle_inquiry_complete(uint8_t* packet, uint16_t size)
{
    if (!found_wii_remote)
    {
        find_wii_remote();
    }
}

void handle_remote_name_request_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    bd_addr_t addr;
    read_bda(packet + 4, addr);
    char* name = (char*)(packet + 10);

    printf("found device status 0x%02x %s name %s\n", status, bda_to_string(addr), name);

    if (strcmp(name, WII_REMOTE_NAME) == 0)
    {
        found_wii_remote = true;
        printf("pairing %s...\n", name);

        start_wii_remote_pairing(addr);
    }
}

void handle_connection_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    uint16_t con_handle = read_uint16(packet + 4);
    bd_addr_t addr;
    read_bda(packet + 6, addr);
    uint8_t link_type = packet[12];
    uint8_t encryption_enabled = packet[13];

    printf("connection complete addr %s status 0x%02x handle 0x%04x, link_type %u encrypted %u\n", bda_to_string(addr), status, con_handle, link_type, encryption_enabled);
    if (status == 0)
    {
        post_bt_packet(create_l2cap_connection_request(con_handle, WII_DATA_PSM, WII_DATA_LOCAL_CID));
    }
}

void handle_disconnection_complete(uint8_t* packet, uint16_t size)
{
    uint8_t status = packet[3];
    uint16_t con_handle = read_uint16(packet + 4);
    uint8_t reason = packet[6];

    printf("disconnected handle 0x%04x status 0x%x02 reason 0x%02x\n", con_handle, status, reason);
}

void handle_l2cap_connection_response(L2CAP_CONNECTION_RESPONSE_PACKET* response_packet)
{
    printf("l2cap conn response handle 0x%04x remote_cid 0x%x local_cid 0x%x result 0x%04x status 0x%x\n",
        response_packet->con_handle, response_packet->remote_cid, response_packet->local_cid, response_packet->result, response_packet->status);

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_PENDING)
    {
        uint16_t options_size = sizeof(L2CAP_CONFIG_MTU_OPTION);
        BT_PACKET_ENVELOPE* env = create_l2cap_config_request(response_packet->con_handle, response_packet->remote_cid, 0, options_size);
        L2CAP_CONFIG_REQUEST_PACKET* config_packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

        L2CAP_CONFIG_MTU_OPTION* mtu_option = (L2CAP_CONFIG_MTU_OPTION*)config_packet->options;
        mtu_option->type = L2CAP_CONFIG_MTU_OPTION_TYPE;
        mtu_option->size = sizeof(L2CAP_CONFIG_MTU_OPTION) - sizeof(L2CAP_CONFIG_OPTION);
        mtu_option->mtu = WII_MTU;

        post_bt_packet(env);
    }

    if (response_packet->status == ERROR_CODE_SUCCESS && response_packet->result == L2CAP_CONNECTION_SUCCESS)
    {
        wii_remote_con_handle = response_packet->con_handle;
        wii_remote_data_cid = response_packet->remote_cid;
        printf("set wii_remote_con_handle %04x wii_remote_data_cid=0x%x\n", wii_remote_con_handle, wii_remote_data_cid);
    }
}

void send_led_report(uint8_t leds)
{
    uint8_t report[3];
    report[0] = HID_OUTPUT_REPORT;
    report[1] = WII_REMOTE_LED_REPORT;
    report[2] = leds;

    post_bt_packet(create_output_report_packet(wii_remote_con_handle, wii_remote_data_cid, report, sizeof(report)));
}

void handle_l2cap_command_reject(L2CAP_COMMAND_REJECT_PACKET* packet)
{
    printf("l2cap cmd rejected handle 0x%04x reason 0x%02x\n", packet->con_handle, packet->reason);
}

void handle_l2cap_disconnection_request(L2CAP_DISCONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap disconnect request 0x%04x remote_cid 0x%0x local_cid 0x%x\n", packet->con_handle, packet->remote_cid, packet->local_cid);
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
                printf(" mtu %u\n", mtu_option->mtu);

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

void handle_l2cap_config_request(L2CAP_CONFIG_REQUEST_PACKET* request_packet)
{
    uint16_t options_size = request_packet->payload_size - 4;
    printf("l2cap config request 0x%04x remote_cid 0x%0x options_size %u options", request_packet->con_handle, request_packet->remote_cid, options_size);
    dump_l2cap_config_options(request_packet->options, options_size);
    printf("\n");

    BT_PACKET_ENVELOPE* env = create_l2cap_config_response(request_packet->con_handle, wii_remote_data_cid, request_packet->identifier, request_packet->flags, options_size);

    L2CAP_CONFIG_RESPONSE_PACKET* response_packet = (L2CAP_CONFIG_RESPONSE_PACKET*)env->packet;

    memcpy(response_packet->options, request_packet->options, options_size);

    post_bt_packet(env);
}

void handle_l2cap_config_response(L2CAP_CONFIG_RESPONSE_PACKET* packet)
{
    uint16_t options_size = packet->payload_size - 6;

    printf("l2cap config response handle 0x%04x local_cid 0x%x result 0x%x options_size %u options", packet->con_handle, packet->local_cid, packet->result, options_size);
    dump_l2cap_config_options(packet->options, options_size);
    printf("\n");
}

void handle_l2cap_signal_channel(uint8_t* packet, uint16_t size, uint16_t con_handle)
{
    uint8_t code = packet[9];
    switch (code)
    {
        case L2CAP_CONNECTION_RESPONSE:
            handle_l2cap_connection_response((L2CAP_CONNECTION_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_CONFIG_REQUEST:
            handle_l2cap_config_request((L2CAP_CONFIG_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONFIG_RESPONSE:
            handle_l2cap_config_response((L2CAP_CONFIG_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_COMMAND_REJECT:
            handle_l2cap_command_reject((L2CAP_COMMAND_REJECT_PACKET*)packet);
            break;
        case L2CAP_DISCONNECTION_REQUEST:
            handle_l2cap_disconnection_request((L2CAP_DISCONNECTION_REQUEST_PACKET*)packet);
            break;
        default:
            printf("unhandled signal channel code 0x%02x\n", code);
            break;
    }
}

void dump_button(uint16_t buttons, uint16_t button, const char* name)
{
    if ((buttons & button) != 0)
    {
        printf(" %s", name);
    }
}

void handle_remote_data(L2CAP_PACKET* packet)
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

        if (!wii_remote_connected)
        {
            send_led_report(WII_REMOTE_LED_4);
            wii_remote_connected = true;
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
                    handle_inquiry_result(packet, size);
                    break;
                case HCI_EVENT_INQUIRY_COMPLETE:
                    handle_inquiry_complete(packet, size);
                    break;
                case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                    handle_remote_name_request_complete(packet, size);
                    break;
                case HCI_EVENT_CONNECTION_COMPLETE:
                    handle_connection_complete(packet, size);
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    handle_disconnection_complete(packet, size);
                    break;
                default:
                    handled = false;
                    break;
            }
            break;
        case HCI_ACL_DATA_PACKET:
        {
            uint16_t con_handle = read_uint16(packet + 1) & HCI_CONN_HANDLE_MASK;
            uint16_t channel = read_uint16(packet + 7);

            switch (channel)
            {
                case L2CAP_SIGNAL_CHANNEL:
                    handle_l2cap_signal_channel(packet, size, con_handle);
                    break;
                default:
                    if (channel == WII_DATA_LOCAL_CID)
                    {
                        handle_remote_data((L2CAP_PACKET*)packet);
                    }
                    else
                    {
                        printf("unhandled l2cap channel %04x con_handle %04x\n", channel, con_handle);
                    }
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
    find_wii_remote();
}

void start_wii_remote_pairing(const bd_addr_t addr)
{
    memcpy(wii_remote_addr, addr, BD_ADDR_LEN);
    post_bt_packet(create_hci_create_connection_packet(addr, 0x18, 1, false, 0, 0));
}