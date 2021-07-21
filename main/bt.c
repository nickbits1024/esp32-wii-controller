#include "wii_controller.h"

uint8_t l2cap_identifier = 1;

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
    return ((uint32_t)p[0]) | (((uint32_t)p[1]) << 8) | (((uint32_t)p[2]) << 16);
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

uint32_t uint24_bytes_to_uint32(const uint8_t* cod)
{
    uint32_t uint32 = 0;

    memcpy(&uint32, cod, 3);

    return uint32;
}

BT_PACKET_ENVELOPE* create_packet_envelope(uint16_t packet_size)
{
    // leave this alone!!!
    size_t size = sizeof(BT_PACKET_ENVELOPE) + packet_size;

    BT_PACKET_ENVELOPE* env = (BT_PACKET_ENVELOPE*)malloc(size);
    memset(env, 0, size);
    //env->io_direction = io_direction;
    env->size = packet_size;      
    return env;
}

BT_PACKET_ENVELOPE* create_hci_cmd_packet(uint16_t op_code, uint8_t params_size)
{
    BT_PACKET_ENVELOPE* env = create_packet_envelope(sizeof(HCI_COMMAND_PACKET) + params_size);

    HCI_COMMAND_PACKET* packet = (HCI_COMMAND_PACKET*)env->packet;

    packet->type = HCI_COMMAND_PACKET_TYPE;
    packet->op_code = op_code;
    packet->params_size = params_size;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_set_controller_to_host_flow_control_packet(uint8_t flow_control_enable)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_SET_CONTROLLER_TO_HOST_FLOW_CONTROL, PARAMS_SIZE(HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL_PACKET));
    HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL_PACKET* packet = (HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL_PACKET*)env->packet;

    packet->flow_control_enable = flow_control_enable;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_host_buffer_size_packet(uint16_t host_acl_data_packet_length, uint8_t host_synchronous_data_packet_length, uint16_t host_total_num_acl_data_packets, uint16_t host_total_num_synchronous_data_packets)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_HOST_BUFFER_SIZE, PARAMS_SIZE(HCI_HOST_BUFFER_SIZE_PACKET));
    HCI_HOST_BUFFER_SIZE_PACKET* packet = (HCI_HOST_BUFFER_SIZE_PACKET*)env->packet;

    packet->host_acl_data_packet_length = host_acl_data_packet_length;
    packet->host_synchronous_data_packet_length = host_synchronous_data_packet_length;
    packet->host_total_num_acl_data_packets = host_total_num_acl_data_packets;
    packet->host_total_num_synchronous_data_packets = host_total_num_synchronous_data_packets;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_host_number_of_completed_packets_packet(uint8_t number_of_handles, uint16_t* connection_handles, uint16_t* host_num_of_completed_packets)
{
    uint16_t data_size = number_of_handles * 4;
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_HOST_NUMBER_OF_COMPLETED_PACKETS, PARAMS_SIZE(HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_PACKET) + data_size);
    HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_PACKET* packet = (HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_PACKET*)env->packet;

    packet->number_of_handles = number_of_handles;
    uint16_t* p = (uint16_t*)packet->data;
    for (int i = 0; i < number_of_handles; i++)
    {
        *p = connection_handles[i];
        p++;
    }
    for (int i = 0; i < number_of_handles; i++)
    {
        *p = host_num_of_completed_packets[i];
        p++;
    }

    return env;

}

BT_PACKET_ENVELOPE* create_hci_reset_packet()
{
    return create_hci_cmd_packet(HCI_OPCODE_RESET, 0);
}

BT_PACKET_ENVELOPE* create_hci_inquiry_cancel_packet()
{
    return create_hci_cmd_packet(HCI_OPCODE_INQUIRY_CANCEL, 0);
}

BT_PACKET_ENVELOPE* create_hci_read_buffer_size_packet()
{
    return create_hci_cmd_packet(HCI_OPCODE_READ_BUFFER_SIZE, 0);
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
    memcpy(packet->addr, addr, BDA_SIZE);
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
    memcpy(packet->addr, addr, BDA_SIZE);
    packet->packet_type = packet_type;
    packet->psrm = psrm;
    packet->reserved = 0;
    write_uint16_be((uint8_t*)&packet->clock_offset, (clock_offset_valid ? 0x8000 : 0) | clock_offset);
    packet->allow_role_switch = allow_role_switch;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_switch_role_packet(const bd_addr_t addr, uint8_t role)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_SWITCH_ROLE_COMMAND, PARAMS_SIZE(HCI_SWITCH_ROLE_PACKET));
    HCI_SWITCH_ROLE_PACKET* packet = (HCI_SWITCH_ROLE_PACKET*)env->packet;

    memcpy(packet->addr, addr, BDA_SIZE);
    packet->role = role;

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
    memcpy(packet->addr, addr, BDA_SIZE);
    memcpy(packet->link_key, link_key, HCI_LINK_KEY_SIZE);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_link_key_request_negative_packet(const bd_addr_t addr)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY, PARAMS_SIZE(HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET));
    HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET* packet = (HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET*)env->packet;

    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BDA_SIZE);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_write_pin_type_packet(uint8_t pin_type)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_PIN_TYPE, PARAMS_SIZE(HCI_WRITE_PIN_TYPE_PACKET));
    HCI_WRITE_PIN_TYPE_PACKET* packet = (HCI_WRITE_PIN_TYPE_PACKET*)env->packet;

    packet->pin_type  = pin_type;

    return env;
}


BT_PACKET_ENVELOPE* create_hci_pin_code_request_reply_packet(const bd_addr_t addr, const uint8_t* pin_code, uint8_t pin_code_size)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_PIN_CODE_REQUEST_REPLY, PARAMS_SIZE(HCI_PIN_CODE_REQUEST_REPLY_PACKET));
    HCI_PIN_CODE_REQUEST_REPLY_PACKET* packet = (HCI_PIN_CODE_REQUEST_REPLY_PACKET*)env->packet;

    if (pin_code_size > HCI_MAX_PIN_CODE_SIZE)
    {
        return NULL;
    }
    //write_bda(packet->addr, addr);
    memcpy(packet->addr, addr, BDA_SIZE);
    packet->pin_code_size = pin_code_size;
    memcpy(packet->pin_code, pin_code, pin_code_size);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_pin_code_request_negative_reply_packet(const bd_addr_t addr)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_PIN_CODE_REQUEST_NEGATIVE_REPLY, PARAMS_SIZE(HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY_PACKET));
    HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY_PACKET* packet = (HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY_PACKET*)env->packet;

    memcpy(packet->addr, addr, BDA_SIZE);

    return env;
}


BT_PACKET_ENVELOPE* create_hci_write_authentication_enable_packet(uint8_t enable)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_AUTHENTICATION_ENABLE, PARAMS_SIZE(HCI_WRITE_AUTHENTICATION_ENABLE_PACKET));
    HCI_WRITE_AUTHENTICATION_ENABLE_PACKET* packet = (HCI_WRITE_AUTHENTICATION_ENABLE_PACKET*)env->packet;

    packet->authentication_enable = enable;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_write_encryption_mode_packet(uint8_t encryption_mode)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_ENCRYPTION_MODE, PARAMS_SIZE(HCI_WRITE_ENCRYPTION_MODE_COMPLETE_PACKET));
    HCI_WRITE_ENCRYPTION_MODE_PACKET* packet = (HCI_WRITE_ENCRYPTION_MODE_PACKET*)env->packet;

    packet->encryption_mode = encryption_mode;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_set_connection_encryption_packet(uint16_t con_handle, uint8_t encryption_enable)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_SET_CONNECTION_ENCRYPTION, PARAMS_SIZE(HCI_SET_CONNECTION_ENCRYPTION_PACKET));
    HCI_SET_CONNECTION_ENCRYPTION_PACKET* packet = (HCI_SET_CONNECTION_ENCRYPTION_PACKET*)env->packet;

    packet->con_handle = con_handle;
    packet->encryption_enable = encryption_enable;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_exit_park_state_packet(uint16_t con_handle)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_EXIT_PARK_STATE, PARAMS_SIZE(HCI_EXIT_PARK_STATE_PACKET));
    HCI_EXIT_PARK_STATE_PACKET* packet = (HCI_EXIT_PARK_STATE_PACKET*)env->packet;

    packet->con_handle = con_handle;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_read_remote_supported_features_packet(uint16_t con_handle)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_READ_REMOTE_SUPPORTED_FEATURES_COMMAND, PARAMS_SIZE(HCI_READ_REMOTE_SUPPORTED_FEATURES_PACKET));
    HCI_READ_REMOTE_SUPPORTED_FEATURES_PACKET* packet = (HCI_READ_REMOTE_SUPPORTED_FEATURES_PACKET*)env->packet;

    packet->con_handle = con_handle;

    return env;
}


BT_PACKET_ENVELOPE* create_hci_set_connection_encryption(uint16_t con_handle, uint8_t encryption_enable)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_SET_CONNECTION_ENCRYPTION, PARAMS_SIZE(HCI_SET_CONNECTION_ENCRYPTION_PACKET));
    HCI_SET_CONNECTION_ENCRYPTION_PACKET* packet = (HCI_SET_CONNECTION_ENCRYPTION_PACKET*)env->packet;

    packet->con_handle = con_handle;
    packet->encryption_enable = encryption_enable;

    return env;
}


BT_PACKET_ENVELOPE* create_hci_write_scan_enable_packet(uint8_t scan_enable)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_SCAN_ENABLE, PARAMS_SIZE(HCI_WRITE_SCAN_ENABLE_PACKET));
    HCI_WRITE_SCAN_ENABLE_PACKET* packet = (HCI_WRITE_SCAN_ENABLE_PACKET*)env->packet;

    packet->scan_enable = scan_enable;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_accept_connection_request_packet(bd_addr_t addr, uint8_t role)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_ACCEPT_CONNECTION_REQUEST, PARAMS_SIZE(HCI_ACCEPT_CONNECTION_REQUEST_PACKET));
    HCI_ACCEPT_CONNECTION_REQUEST_PACKET* packet = (HCI_ACCEPT_CONNECTION_REQUEST_PACKET*)env->packet;

    memcpy(packet->addr, addr, BDA_SIZE);
    packet->role = role;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_reject_connection_request_packet(bd_addr_t addr, uint8_t reason)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_REJECT_CONNECTION_REQUEST, PARAMS_SIZE(HCI_REJECT_CONNECTION_REQUEST_PACKET));
    HCI_REJECT_CONNECTION_REQUEST_PACKET* packet = (HCI_REJECT_CONNECTION_REQUEST_PACKET*)env->packet;

    memcpy(packet->addr, addr, BDA_SIZE);
    packet->reason = reason;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_disconnect_packet(uint16_t con_handle, uint8_t reason)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_DISCONNECT, PARAMS_SIZE(HCI_DISCONNECT_PACKET));
    HCI_DISCONNECT_PACKET* packet = (HCI_DISCONNECT_PACKET*)env->packet;

    packet->con_handle = con_handle;
    packet->reason = reason;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_write_class_of_device_packet(uint32_t class_of_device)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_CLASS_OF_DEVICE, PARAMS_SIZE(HCI_WRITE_CLASS_OF_DEVICE_PACKET));
    HCI_WRITE_CLASS_OF_DEVICE_PACKET* packet = (HCI_WRITE_CLASS_OF_DEVICE_PACKET*)env->packet;

    memcpy(&packet->class_of_device, &class_of_device, 3);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_write_default_link_policy_settings_packet(uint16_t default_link_policy_settings)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_DEFAULT_LINK_POLICY_SETTINGS, PARAMS_SIZE(HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_PACKET));
    HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_PACKET* packet = (HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_PACKET*)env->packet;

    packet->default_link_policy_settings = default_link_policy_settings;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_secure_connections_host_support_packet(uint16_t secure_connections_host_support)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT, PARAMS_SIZE(HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_PACKET));
    HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_PACKET* packet = (HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_PACKET*)env->packet;

    packet->secure_connections_host_support  = secure_connections_host_support;

    return env;
}

BT_PACKET_ENVELOPE* create_hci_qos_setup_packet(uint16_t con_handle, uint8_t flags, uint8_t service_type, uint32_t token_rate, uint32_t peak_bandwidth, uint32_t latency, uint32_t delay_variation)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_QOS_SETUP, PARAMS_SIZE(HCI_QOS_SETUP_PACKET));
    HCI_QOS_SETUP_PACKET* packet = (HCI_QOS_SETUP_PACKET*)env->packet;

    packet->con_handle = con_handle;
    packet->flags = flags;
    packet->service_type = service_type;
    packet->token_rate = token_rate;
    packet->peak_bandwidth = peak_bandwidth;
    packet->latency = latency;
    packet->delay_variation = delay_variation;

    return env;

}

BT_PACKET_ENVELOPE* create_hci_current_iac_lap_packet(uint32_t iac_lap)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_CURRENT_IAC_LAP, PARAMS_SIZE(HCI_WRITE_CURRENT_IAC_LAP_PACKET));
    HCI_WRITE_CURRENT_IAC_LAP_PACKET* packet = (HCI_WRITE_CURRENT_IAC_LAP_PACKET*)env->packet;

    packet->num_current_iac = 1;
    memcpy(&packet->iac_lap, &iac_lap, 3);

    return env;
}

BT_PACKET_ENVELOPE* create_hci_write_local_name(char* local_name)
{
    BT_PACKET_ENVELOPE* env = create_hci_cmd_packet(HCI_OPCODE_WRITE_LOCAL_NAME, PARAMS_SIZE(HCI_WRITE_LOCAL_NAME_PACKET));
    HCI_WRITE_LOCAL_NAME_PACKET* packet = (HCI_WRITE_LOCAL_NAME_PACKET*)env->packet;

    strncpy((char*)packet->local_name, local_name, HCI_MAX_LOCAL_NAME_SIZE);

    return env;
}

BT_PACKET_ENVELOPE* create_acl_packet(uint16_t con_handle, uint16_t channel, uint8_t packet_boundary_flag, uint8_t broadcast_flag, const uint8_t* data, uint16_t data_size)
{
    BT_PACKET_ENVELOPE* env = create_packet_envelope(sizeof(HCI_ACL_PACKET) + data_size);

    HCI_ACL_PACKET* packet = (HCI_ACL_PACKET*)env->packet;

    packet->type = HCI_ACL_PACKET_TYPE;
    packet->con_handle = con_handle;
    packet->packet_boundary_flag = packet_boundary_flag;
    packet->broadcast_flag = broadcast_flag;
    packet->hci_acl_size = data_size;
    memcpy(packet->data, data, data_size);

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_base_packet(uint16_t packet_size, uint16_t con_handle, uint16_t channel)
{
    BT_PACKET_ENVELOPE* env = create_packet_envelope(packet_size);

    L2CAP_PACKET* packet = (L2CAP_PACKET*)env->packet;

    packet->type = HCI_ACL_PACKET_TYPE;
    packet->con_handle = con_handle;
    packet->packet_boundary_flag = L2CAP_PB_FIRST_FLUSH;
    packet->broadcast_flag = L2CAP_BROADCAST_NONE;
    packet->hci_acl_size = packet_size - sizeof(HCI_ACL_PACKET);
    packet->l2cap_size = packet_size - sizeof(L2CAP_PACKET);
    packet->channel = channel;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_packet(uint16_t con_handle, uint16_t l2cap_size, uint16_t channel, const uint8_t* data, uint16_t data_size)
{
    uint16_t size = sizeof(L2CAP_PACKET) + data_size;

    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, channel);
    L2CAP_PACKET* packet = (L2CAP_PACKET*)env->packet;
    if (l2cap_size != L2CAP_AUTO_SIZE)
    {
        packet->l2cap_size = l2cap_size;
    }
    memcpy(packet->data, data, data_size);

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_connection_request_packet(uint16_t con_handle, uint16_t psm, uint16_t source_cid)
{
    uint16_t size = sizeof(L2CAP_CONNECTION_REQUEST_PACKET);
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);

    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONNECTION_REQUEST_PACKET* packet = (L2CAP_CONNECTION_REQUEST_PACKET*)env->packet;

    packet->code = L2CAP_CONNECTION_REQUEST;
    packet->identifier = l2cap_identifier++;
    packet->payload_size = payload_size;
    packet->psm = psm;
    packet->source_cid = source_cid;

    // printf("sizeof(L2CAP_CONNECTION_REQUEST_PACKET) = %u\n", sizeof(L2CAP_CONNECTION_REQUEST_PACKET));
    // printf("sizeof(L2CAP_SIGNAL_CHANNEL_PACKET) = %u\n", sizeof(L2CAP_SIGNAL_CHANNEL_PACKET));
    // printf("sizeof(L2CAP_PACKET) = %u\n", sizeof(L2CAP_PACKET));
    // printf("sizeof(HCI_ACL_PACKET) = %u\n", sizeof(HCI_ACL_PACKET));
    // printf("sizeof(BT_PACKET_ENVELOPE) = %u\n", sizeof(BT_PACKET_ENVELOPE));

    //printf("create_l2cap_connection_request size %u hci_acl_size %u l2cap2_size %u payload_size %u\n", env->size, packet->hci_acl_size, packet->l2cap_size, packet->payload_size);

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_connection_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t dest_cid, uint16_t source_cid, uint16_t result, uint16_t status)
{
    uint16_t size = sizeof(L2CAP_CONNECTION_RESPONSE_PACKET);
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);

    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONNECTION_RESPONSE_PACKET* packet = (L2CAP_CONNECTION_RESPONSE_PACKET*)env->packet;

    packet->code = L2CAP_CONNECTION_RESPONSE;
    packet->identifier = identifier;
    packet->payload_size = payload_size;
    packet->dest_cid = dest_cid;
    packet->source_cid = source_cid;
    packet->result = result;
    packet->status = status;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_config_request_packet(uint16_t con_handle, uint16_t dest_cid, uint16_t flags, uint16_t options_size)
{
    uint16_t size = sizeof(L2CAP_CONFIG_REQUEST_PACKET) + options_size;
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);
    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONFIG_REQUEST_PACKET* packet = (L2CAP_CONFIG_REQUEST_PACKET*)env->packet;

    packet->code = L2CAP_CONFIG_REQUEST;
    packet->identifier = l2cap_identifier++;
    packet->payload_size = payload_size;
    packet->dest_cid = dest_cid;
    packet->flags = flags;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_config_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t source_cid, uint16_t flags, uint16_t options_size)
{
    uint16_t size = sizeof(L2CAP_CONFIG_RESPONSE_PACKET) + options_size;
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);
    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_CONFIG_RESPONSE_PACKET* packet = (L2CAP_CONFIG_RESPONSE_PACKET*)env->packet;

    packet->code = L2CAP_CONFIG_RESPONSE;
    packet->identifier = identifier;
    packet->payload_size = payload_size;
    packet->source_cid = source_cid;
    packet->flags = flags;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_disconnection_request_packet(uint16_t con_handle, uint16_t dest_cid, uint16_t source_cid)
{
    uint16_t size = sizeof(L2CAP_DISCONNECTION_REQUEST_PACKET);
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);
    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_DISCONNECTION_REQUEST_PACKET* packet = (L2CAP_DISCONNECTION_REQUEST_PACKET*)env->packet;

    packet->code = L2CAP_DISCONNECTION_REQUEST;
    packet->identifier = l2cap_identifier++;
    packet->payload_size = payload_size;
    packet->source_cid = source_cid;
    packet->dest_cid = dest_cid;

    return env;
}

BT_PACKET_ENVELOPE* create_l2cap_disconnection_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t dest_cid, uint16_t source_cid)
{
    uint16_t size = sizeof(L2CAP_DISCONNECTION_RESPONSE_PACKET);
    uint16_t payload_size = size - sizeof(L2CAP_SIGNAL_CHANNEL_PACKET);
    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, L2CAP_SIGNAL_CHANNEL);
    L2CAP_DISCONNECTION_RESPONSE_PACKET* packet = (L2CAP_DISCONNECTION_RESPONSE_PACKET*)env->packet;

    packet->code = L2CAP_DISCONNECTION_RESPONSE;
    packet->identifier = identifier;
    packet->payload_size = payload_size;
    packet->source_cid = source_cid;
    packet->dest_cid = dest_cid;

    return env;
}

BT_PACKET_ENVELOPE* create_output_report_packet(uint16_t con_handle, uint16_t channel, const uint8_t* report, uint16_t report_size)
{
    uint16_t size = sizeof(L2CAP_PACKET) + report_size;
    BT_PACKET_ENVELOPE* env = create_l2cap_base_packet(size, con_handle, channel);
    L2CAP_PACKET* packet = (L2CAP_PACKET*)env->packet;
    memcpy(packet->data, report, report_size);

    return env;
}
