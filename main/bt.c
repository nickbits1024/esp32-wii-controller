#include "wii_controller.h"

// void reverse_bda(bd_addr_t bda)
// {
//     uint8_t temp;
//     temp = bda[0];
//     bda[0] = bda[5];
//     bda[5] = temp;
//     temp = bda[1];
//     bda[1] = bda[4];
//     bda[4] = temp;
//     temp = bda[2];
//     bda[2] = bda[3];
//     bda[3] = temp;
// }

// void read_bda(const uint8_t* p, bd_addr_t bda)
// {
//     // bda[0] = p[5];
//     // bda[1] = p[4];
//     // bda[2] = p[3];
//     // bda[3] = p[2];
//     // bda[4] = p[1];
//     // bda[5] = p[0];
// }

// void write_bda(uint8_t* p, const bd_addr_t bda)
// {
//     p[0] = bda[5];
//     p[1] = bda[4];
//     p[2] = bda[3];
//     p[3] = bda[2];
//     p[4] = bda[1];
//     p[5] = bda[0];
// }

uint16_t read_uint16(uint8_t* p)
{
    return *((uint16_t*)p);
}

uint16_t read_uint16_be(uint8_t* p)
{
    return (uint16_t)p[0] << 8 | p[1];
}

uint32_t read_uint24(const uint8_t* p)
{
    return ((uint32_t) p[0]) | (((uint32_t)p[1]) << 8) | (((uint32_t)p[2]) << 16);
}

void write_uint16_be(uint8_t* p, uint16_t value)
{
    p[0] = value >> 8;
    p[1] = value & 0xff;
}

const char* bda_to_string(const bd_addr_t bda)
{
    static char addr[2][18];

    int core = xPortGetCoreID();

    char* p = &addr[core][0];

    //snprintf(p, 18, "%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
	snprintf(p, 18, "%02x:%02x:%02x:%02x:%02x:%02x", bda[5], bda[4], bda[3], bda[2], bda[1], bda[0]);

    return p;
}

BT_PACKET_ENVELOPE* create_hci_cmd_packet(uint16_t op_code, uint8_t params_size)
{
    uint16_t packet_size = sizeof(HCI_COMMAND_PACKET) + params_size;
    uint16_t size = sizeof(BT_PACKET_ENVELOPE) + packet_size;
    BT_PACKET_ENVELOPE* env = (BT_PACKET_ENVELOPE*)malloc(size);
    memset(env, 0, size);
    env->size = packet_size;

    HCI_COMMAND_PACKET* packet = (HCI_COMMAND_PACKET*)env->packet;

    packet->type = HCI_COMMAND_DATA_PACKET;
    packet->op_code = op_code;
    packet->params_size = params_size;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_inquiry_packet(uint32_t lap, uint8_t duration, uint8_t num_responses)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_INQUIRY, PARAMS_SIZE(HCI_INQUIRY_PACKET));
    HCI_INQUIRY_PACKET* packet = (HCI_INQUIRY_PACKET*)env->packet;

    memcpy(&packet->lap, &lap, 3);
    packet->duration = duration;
    packet->num_responses = num_responses;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_remote_name_request_packet(const bd_addr_t addr, uint8_t psrm, bool clock_offset_valid, uint16_t clock_offset)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_REMOTE_NAME_REQUEST, PARAMS_SIZE(HCI_REMOTE_NAME_REQUEST_PACKET));
    HCI_REMOTE_NAME_REQUEST_PACKET* packet = (HCI_REMOTE_NAME_REQUEST_PACKET*)env->packet;

    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BD_ADDR_LEN);
    packet->psrm = psrm;
    packet->reserved = 0;
    write_uint16_be((uint8_t*)&packet->clock_offset, (clock_offset_valid ? 0x8000 : 0) | clock_offset);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_create_connection_packet(const bd_addr_t addr, uint16_t packet_type, uint8_t psrm, bool clock_offset_valid, uint16_t clock_offset, uint8_t allow_role_switch)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_CREATE_CONNECTION, PARAMS_SIZE(HCI_CREATE_CONNECTION_PACKET));
    HCI_CREATE_CONNECTION_PACKET* packet = (HCI_CREATE_CONNECTION_PACKET*)env->packet;

    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BD_ADDR_LEN);
    packet->packet_type = packet_type;
    packet->psrm = psrm;
    packet->reserved = 0x00,
    write_uint16_be((uint8_t*)&packet->clock_offset, (clock_offset_valid ? 0x8000 : 0) | clock_offset);
    packet->allow_role_switch = allow_role_switch;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_authentication_requested_packet(uint16_t con_handle)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_AUTHENTICATION_REQUESTED, PARAMS_SIZE(HCI_AUTHENTICATION_REQUESTED_PACKET));
    HCI_AUTHENTICATION_REQUESTED_PACKET* packet = (HCI_AUTHENTICATION_REQUESTED_PACKET*)env->packet;
    packet->con_handle = con_handle;
    return env;
}

BT_PACKET_ENVELOPE* create_hci_link_key_request_reply_packet(const bd_addr_t addr, const uint8_t* link_key)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_LINK_KEY_REQUEST_REPLY, PARAMS_SIZE(HCI_LINK_KEY_REQUEST_REPLY_PACKET));
    HCI_LINK_KEY_REQUEST_REPLY_PACKET* packet = (HCI_LINK_KEY_REQUEST_REPLY_PACKET*)env->packet;

    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BD_ADDR_LEN);
    memcpy(packet->link_key, link_key, HCI_LINK_KEY_SIZE);    

    return env;
}

BT_PACKET_ENVELOPE* create_hci_link_key_request_negative_packet(const bd_addr_t addr)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY, PARAMS_SIZE(HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET));
    HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET* packet = (HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET*)env->packet;

    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BD_ADDR_LEN);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_pin_code_reply_packet(const bd_addr_t addr, const uint8_t* pin_code, uint8_t pin_code_size)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_PIN_CODE_REQUEST_REPLY, PARAMS_SIZE(HCI_PIN_CODE_REQUEST_REPLY_PACKET));
    HCI_PIN_CODE_REQUEST_REPLY_PACKET* packet = (HCI_PIN_CODE_REQUEST_REPLY_PACKET*)env->packet;

    if (pin_code_size > HCI_MAX_PIN_CODE_SIZE)
    {
        return NULL;
    }
    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BD_ADDR_LEN);
    packet->pin_code_size = pin_code_size;
    memcpy(packet->pin_code, pin_code, pin_code_size);

    return env;
}

BT_PACKET_ENVELOPE* create_acl_packet(uint16_t packet_size, uint16_t con_handle, uint16_t channel)
{
    uint16_t size = sizeof(BT_PACKET_ENVELOPE) + packet_size;
    BT_PACKET_ENVELOPE* env = (BT_PACKET_ENVELOPE*)malloc(size);
    memset(env, 0, size);
    env->size = packet_size;

    L2CAP_PACKET* packet = (L2CAP_PACKET*)env->packet;

    packet->type = HCI_ACL_DATA_PACKET;
    packet->con_handle = con_handle;
    packet->packet_boundary_flag = L2CAP_PB_FIRST_FLUSH;
    packet->broadcast_flag = L2CAP_BROADCAST_NONE;
    packet->hci_acl_size = packet_size - sizeof(HCI_ACL_PACKET);
    packet->l2cap_size = packet_size - sizeof(L2CAP_PACKET);
    packet->channel = channel;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_connection_request(uint16_t con_handle, uint16_t psm, uint16_t local_cid)
{
    uint16_t size = sizeof(L2CAP_CONNECTION_REQUEST_PACKET);
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_PACKET);

    BT_PACKET_ENVELOPE* env = create_acl_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONNECTION_REQUEST_PACKET* packet = (L2CAP_CONNECTION_REQUEST_PACKET*)env->packet;

    packet->code = L2CAP_CONNECTION_REQUEST;
    packet->identifier = 0xdd;
    packet->payload_size = payload_size;
    packet->psm = psm;
    packet->local_cid = local_cid;

    // printf("sizeof(L2CAP_CONNECTION_REQUEST_PACKET) = %u\n", sizeof(L2CAP_CONNECTION_REQUEST_PACKET));
    // printf("sizeof(L2CAP_SIGNAL_PACKET) = %u\n", sizeof(L2CAP_SIGNAL_PACKET));
    // printf("sizeof(L2CAP_PACKET) = %u\n", sizeof(L2CAP_PACKET));
    // printf("sizeof(HCI_ACL_PACKET) = %u\n", sizeof(HCI_ACL_PACKET));
    // printf("sizeof(BT_PACKET_ENVELOPE) = %u\n", sizeof(BT_PACKET_ENVELOPE));

    //printf("create_l2cap_connection_request size %u hci_acl_size %u l2cap2_size %u payload_size %u\n", env->size, packet->hci_acl_size, packet->l2cap_size, packet->payload_size);

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_config_request(uint16_t con_handle, uint16_t remote_cid, uint16_t flags, uint16_t options_size)
{
    uint16_t size = sizeof(L2CAP_CONFIG_REQUEST_PACKET) + options_size;
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_PACKET);
    BT_PACKET_ENVELOPE* env = create_acl_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONFIG_REQUEST_PACKET* packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

    packet->code = L2CAP_CONFIG_REQUEST;
    packet->identifier = 0xdd;
    packet->payload_size = payload_size;
    packet->remote_cid = remote_cid;
    packet->flags = flags;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_config_response(uint16_t con_handle, uint16_t local_cid, uint8_t identifier, uint16_t flags, uint16_t options_size)
{
    uint16_t size = sizeof(L2CAP_CONFIG_RESPONSE_PACKET) + options_size;
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_PACKET);
    BT_PACKET_ENVELOPE* env = create_acl_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONFIG_RESPONSE_PACKET* packet = (L2CAP_CONFIG_RESPONSE_PACKET*)env->packet;

    packet->code = L2CAP_CONFIG_RESPONSE;
    packet->identifier = identifier;
    packet->payload_size = payload_size;
    packet->local_cid = local_cid;
    packet->flags = flags;

    return env;
}

BT_PACKET_ENVELOPE* create_output_report_packet(uint16_t con_handle, uint16_t channel, uint8_t* report, uint16_t report_size)
{
    uint16_t size = sizeof(L2CAP_PACKET) + report_size;
    BT_PACKET_ENVELOPE* env = create_acl_packet(size, con_handle, channel);
    L2CAP_PACKET* packet = (L2CAP_PACKET*)env->packet;
    memcpy(packet->data, report, report_size);

    return env;
}
