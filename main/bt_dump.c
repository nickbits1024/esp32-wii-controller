#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "bt.h"
#include "bt_dump.h"

static void dump_l2cap_config_options(uint8_t* options, uint16_t options_size);

const char* bda_to_string(const bd_addr_t bda)
{
    static char addr[2][18];

#ifdef _WINDOWS_
    int core = 0;
#else
    int core = xPortGetCoreID();
#endif

    char* p = &addr[core][0];

    //snprintf(p, 18, "%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    snprintf(p, 18, "%02x:%02x:%02x:%02x:%02x:%02x", bda[5], bda[4], bda[3], bda[2], bda[1], bda[0]);

    return p;
}

uint16_t read_uint16(const uint8_t* p)
{
    return *((uint16_t*)p);
}

uint16_t read_uint16_be(const uint8_t* p)
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

uint32_t uint24_bytes_to_uint32(const uint8_t* cod)
{
    uint32_t uint32 = 0;

    memcpy(&uint32, cod, 3);

    return uint32;
}

const char* get_hci_event_name(uint16_t event_code)
{
    switch (event_code)
    {
        case HCI_EVENT_INQUIRY_COMPLETE:
            return "HCI_EVENT_INQUIRY_COMPLETE";
        case HCI_EVENT_INQUIRY_RESULT:
            return "HCI_EVENT_INQUIRY_RESULT";
        case HCI_EVENT_CONNECTION_COMPLETE:
            return "HCI_EVENT_CONNECTION_COMPLETE";
        case HCI_EVENT_CONNECTION_REQUEST:
            return "HCI_EVENT_CONNECTION_REQUEST";
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            return "HCI_EVENT_DISCONNECTION_COMPLETE";
        case HCI_EVENT_AUTHENTICATION_COMPLETE:
            return "HCI_EVENT_AUTHENTICATION_COMPLETE";
        case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
            return "HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE";
        case HCI_EVENT_ENCRYPTION_CHANGE:
            return "HCI_EVENT_ENCRYPTION_CHANGE";
        case HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE:
            return "HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE";
        case HCI_EVENT_MASTER_LINK_KEY_COMPLETE:
            return "HCI_EVENT_MASTER_LINK_KEY_COMPLETE";
        case HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE:
            return "HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE";
        case HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE:
            return "HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE";
        case HCI_EVENT_QOS_SETUP_COMPLETE:
            return "HCI_EVENT_QOS_SETUP_COMPLETE";
        case HCI_EVENT_COMMAND_COMPLETE:
            return "HCI_EVENT_COMMAND_COMPLETE";
        case HCI_EVENT_COMMAND_STATUS:
            return "HCI_EVENT_COMMAND_STATUS";
        case HCI_EVENT_HARDWARE_ERROR:
            return "HCI_EVENT_HARDWARE_ERROR";
        case HCI_EVENT_FLUSH_OCCURRED:
            return "HCI_EVENT_FLUSH_OCCURRED";
        case HCI_EVENT_ROLE_CHANGE:
            return "HCI_EVENT_ROLE_CHANGE";
        case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
            return "HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS";
        case HCI_EVENT_MODE_CHANGE:
            return "HCI_EVENT_MODE_CHANGE";
        case HCI_EVENT_RETURN_LINK_KEYS:
            return "HCI_EVENT_RETURN_LINK_KEYS";
        case HCI_EVENT_PIN_CODE_REQUEST:
            return "HCI_EVENT_PIN_CODE_REQUEST";
        case HCI_EVENT_LINK_KEY_REQUEST:
            return "HCI_EVENT_LINK_KEY_REQUEST";
        case HCI_EVENT_LINK_KEY_NOTIFICATION:
            return "HCI_EVENT_LINK_KEY_NOTIFICATION";
        case HCI_EVENT_DATA_BUFFER_OVERFLOW:
            return "HCI_EVENT_DATA_BUFFER_OVERFLOW";
        case HCI_EVENT_MAX_SLOTS_CHANGED:
            return "HCI_EVENT_MAX_SLOTS_CHANGED";
        case HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE:
            return "HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE";
        case HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED:
            return "HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED";
        case HCI_EVENT_INQUIRY_RESULT_WITH_RSSI:
            return "HCI_EVENT_INQUIRY_RESULT_WITH_RSSI";
        case HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE:
            return "HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE";
        case HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE:
            return "HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE";
        case HCI_EVENT_EXTENDED_INQUIRY_RESPONSE:
            return "HCI_EVENT_EXTENDED_INQUIRY_RESPONSE";
        case HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE:
            return "HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE";
        case HCI_EVENT_IO_CAPABILITY_REQUEST:
            return "HCI_EVENT_IO_CAPABILITY_REQUEST";
        case HCI_EVENT_IO_CAPABILITY_RESPONSE:
            return "HCI_EVENT_IO_CAPABILITY_RESPONSE";
        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            return "HCI_EVENT_USER_CONFIRMATION_REQUEST";
        case HCI_EVENT_USER_PASSKEY_REQUEST:
            return "HCI_EVENT_USER_PASSKEY_REQUEST";
        case HCI_EVENT_REMOTE_OOB_DATA_REQUEST:
            return "HCI_EVENT_REMOTE_OOB_DATA_REQUEST";
        case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:
            return "HCI_EVENT_SIMPLE_PAIRING_COMPLETE";
        case HCI_EVENT_USER_PASSKEY_NOTIFICATION:
            return "HCI_EVENT_USER_PASSKEY_NOTIFICATION";
        case HCI_EVENT_KEYPRESS_NOTIFICATION:
            return "HCI_EVENT_KEYPRESS_NOTIFICATION";
        case HCI_EVENT_VENDOR_SPECIFIC:
            return "HCI_EVENT_VENDOR_SPECIFIC";
        case HCI_EVENT_TRANSPORT_SLEEP_MODE:
            return "HCI_EVENT_TRANSPORT_SLEEP_MODE";
        case HCI_EVENT_TRANSPORT_READY:
            return "HCI_EVENT_TRANSPORT_READY";
        case HCI_EVENT_TRANSPORT_PACKET_SENT:
            return "HCI_EVENT_TRANSPORT_PACKET_SENT";
        case HCI_EVENT_SCO_CAN_SEND_NOW:
            return "HCI_EVENT_SCO_CAN_SEND_NOW";
        case L2CAP_EVENT_CHANNEL_OPENED:
            return "L2CAP_EVENT_CHANNEL_OPENED";
        case L2CAP_EVENT_CHANNEL_CLOSED:
            return "L2CAP_EVENT_CHANNEL_CLOSED";
        case L2CAP_EVENT_INCOMING_CONNECTION:
            return "L2CAP_EVENT_INCOMING_CONNECTION";
        case L2CAP_EVENT_TIMEOUT_CHECK:
            return "L2CAP_EVENT_TIMEOUT_CHECK";
        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_REQUEST:
            return "L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_REQUEST";
        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            return "L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE";
        case L2CAP_EVENT_CAN_SEND_NOW:
            return "L2CAP_EVENT_CAN_SEND_NOW";
        case L2CAP_EVENT_ERTM_BUFFER_RELEASED:
            return "L2CAP_EVENT_ERTM_BUFFER_RELEASED";
        case L2CAP_EVENT_TRIGGER_RUN:
            return "L2CAP_EVENT_TRIGGER_RUN";
        default:
            return "HCI_EVENT_XXXX";
    }
}

const char* get_hci_op_code_name(uint16_t op_code)
{
    switch (op_code)
    {
        case HCI_OPCODE_INQUIRY:
            return "HCI_INQUIRY";
        case HCI_OPCODE_INQUIRY_CANCEL:
            return "HCI_INQUIRY_CANCEL";
        case HCI_OPCODE_CREATE_CONNECTION:
            return "HCI_CREATE_CONNECTION";
        case HCI_OPCODE_DISCONNECT:
            return "HCI_DISCONNECT";
        case HCI_OPCODE_CREATE_CONNECTION_CANCEL:
            return "HCI_CREATE_CONNECTION_CANCEL";
        case HCI_OPCODE_ACCEPT_CONNECTION_REQUEST:
            return "HCI_ACCEPT_CONNECTION_REQUEST";
        case HCI_OPCODE_REJECT_CONNECTION_REQUEST:
            return "HCI_REJECT_CONNECTION_REQUEST";
        case HCI_OPCODE_LINK_KEY_REQUEST_REPLY:
            return "HCI_LINK_KEY_REQUEST_REPLY";
        case HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY:
            return "HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_PIN_CODE_REQUEST_REPLY:
            return "HCI_PIN_CODE_REQUEST_REPLY";
        case HCI_OPCODE_PIN_CODE_REQUEST_NEGATIVE_REPLY:
            return "HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_CHANGE_CONNECTION_PACKET_TYPE:
            return "HCI_CHANGE_CONNECTION_PACKET_TYPE";
        case HCI_OPCODE_AUTHENTICATION_REQUESTED:
            return "HCI_AUTHENTICATION_REQUESTED";
        case HCI_OPCODE_SET_CONNECTION_ENCRYPTION:
            return "HCI_SET_CONNECTION_ENCRYPTION";
        case HCI_OPCODE_CHANGE_CONNECTION_LINK_KEY:
            return "HCI_CHANGE_CONNECTION_LINK_KEY";
        case HCI_OPCODE_REMOTE_NAME_REQUEST:
            return "HCI_REMOTE_NAME_REQUEST";
        case HCI_OPCODE_REMOTE_NAME_REQUEST_CANCEL:
            return "HCI_REMOTE_NAME_REQUEST_CANCEL";
        case HCI_OPCODE_READ_REMOTE_SUPPORTED_FEATURES_COMMAND:
            return "HCI_READ_REMOTE_SUPPORTED_FEATURES_COMMAND";
        case HCI_OPCODE_READ_REMOTE_EXTENDED_FEATURES_COMMAND:
            return "HCI_READ_REMOTE_EXTENDED_FEATURES_COMMAND";
        case HCI_OPCODE_READ_REMOTE_VERSION_INFORMATION:
            return "HCI_READ_REMOTE_VERSION_INFORMATION";
        case HCI_OPCODE_SETUP_SYNCHRONOUS_CONNECTION:
            return "HCI_SETUP_SYNCHRONOUS_CONNECTION";
        case HCI_OPCODE_ACCEPT_SYNCHRONOUS_CONNECTION:
            return "HCI_ACCEPT_SYNCHRONOUS_CONNECTION";
        case HCI_OPCODE_IO_CAPABILITY_REQUEST_REPLY:
            return "HCI_IO_CAPABILITY_REQUEST_REPLY";
        case HCI_OPCODE_USER_CONFIRMATION_REQUEST_REPLY:
            return "HCI_USER_CONFIRMATION_REQUEST_REPLY";
        case HCI_OPCODE_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY:
            return "HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_USER_PASSKEY_REQUEST_REPLY:
            return "HCI_USER_PASSKEY_REQUEST_REPLY";
        case HCI_OPCODE_USER_PASSKEY_REQUEST_NEGATIVE_REPLY:
            return "HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_REMOTE_OOB_DATA_REQUEST_REPLY:
            return "HCI_REMOTE_OOB_DATA_REQUEST_REPLY";
        case HCI_OPCODE_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY:
            return "HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY:
            return "HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY";
        case HCI_OPCODE_ENHANCED_SETUP_SYNCHRONOUS_CONNECTION:
            return "HCI_ENHANCED_SETUP_SYNCHRONOUS_CONNECTION";
        case HCI_OPCODE_ENHANCED_ACCEPT_SYNCHRONOUS_CONNECTION:
            return "HCI_ENHANCED_ACCEPT_SYNCHRONOUS_CONNECTION";
        case HCI_OPCODE_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY:
            return "HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY";
        case HCI_OPCODE_SNIFF_MODE:
            return "HCI_SNIFF_MODE";
        case HCI_OPCODE_EXIT_SNIFF_MODE:
            return "HCI_EXIT_SNIFF_MODE";
        case HCI_OPCODE_QOS_SETUP:
            return "HCI_QOS_SETUP";
        case HCI_OPCODE_ROLE_DISCOVERY:
            return "HCI_ROLE_DISCOVERY";
        case HCI_OPCODE_SWITCH_ROLE_COMMAND:
            return "HCI_SWITCH_ROLE_COMMAND";
        case HCI_OPCODE_READ_LINK_POLICY_SETTINGS:
            return "HCI_READ_LINK_POLICY_SETTINGS";
        case HCI_OPCODE_WRITE_LINK_POLICY_SETTINGS:
            return "HCI_WRITE_LINK_POLICY_SETTINGS";
        case HCI_OPCODE_WRITE_DEFAULT_LINK_POLICY_SETTINGS:
            return "HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS";
        case HCI_OPCODE_FLOW_SPECIFICATION:
            return "HCI_FLOW_SPECIFICATION";
        case HCI_OPCODE_SNIFF_SUBRATING:
            return "HCI_SNIFF_SUBRATING";
        case HCI_OPCODE_SET_EVENT_MASK:
            return "HCI_SET_EVENT_MASK";
        case HCI_OPCODE_RESET:
            return "HCI_RESET";
        case HCI_OPCODE_FLUSH:
            return "HCI_FLUSH";
        case HCI_OPCODE_READ_PIN_TYPE:
            return "HCI_READ_PIN_TYPE";
        case HCI_OPCODE_WRITE_PIN_TYPE:
            return "HCI_WRITE_PIN_TYPE";
        case HCI_OPCODE_DELETE_STORED_LINK_KEY:
            return "HCI_DELETE_STORED_LINK_KEY";
        case HCI_OPCODE_WRITE_LOCAL_NAME:
            return "HCI_WRITE_LOCAL_NAME";
        case HCI_OPCODE_READ_LOCAL_NAME:
            return "HCI_READ_LOCAL_NAME";
        case HCI_OPCODE_READ_PAGE_TIMEOUT:
            return "HCI_READ_PAGE_TIMEOUT";
        case HCI_OPCODE_WRITE_PAGE_TIMEOUT:
            return "HCI_WRITE_PAGE_TIMEOUT";
        case HCI_OPCODE_WRITE_SCAN_ENABLE:
            return "HCI_WRITE_SCAN_ENABLE";
        case HCI_OPCODE_READ_PAGE_SCAN_ACTIVITY:
            return "HCI_READ_PAGE_SCAN_ACTIVITY";
        case HCI_OPCODE_WRITE_PAGE_SCAN_ACTIVITY:
            return "HCI_WRITE_PAGE_SCAN_ACTIVITY";
        case HCI_OPCODE_READ_INQUIRY_SCAN_ACTIVITY:
            return "HCI_READ_INQUIRY_SCAN_ACTIVITY";
        case HCI_OPCODE_WRITE_INQUIRY_SCAN_ACTIVITY:
            return "HCI_WRITE_INQUIRY_SCAN_ACTIVITY";
        case HCI_OPCODE_WRITE_AUTHENTICATION_ENABLE:
            return "HCI_WRITE_AUTHENTICATION_ENABLE";
        case HCI_OPCODE_WRITE_CLASS_OF_DEVICE:
            return "HCI_WRITE_CLASS_OF_DEVICE";
        case HCI_OPCODE_READ_NUM_BROADCAST_RETRANSMISSIONS:
            return "HCI_READ_NUM_BROADCAST_RETRANSMISSIONS";
        case HCI_OPCODE_WRITE_NUM_BROADCAST_RETRANSMISSIONS:
            return "HCI_WRITE_NUM_BROADCAST_RETRANSMISSIONS";
        case HCI_OPCODE_READ_TRANSMIT_POWER_LEVEL:
            return "HCI_READ_TRANSMIT_POWER_LEVEL";
        case HCI_OPCODE_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE:
            return "HCI_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE";
        case HCI_OPCODE_SET_CONTROLLER_TO_HOST_FLOW_CONTROL:
            return "HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL";
        case HCI_OPCODE_HOST_BUFFER_SIZE:
            return "HCI_HOST_BUFFER_SIZE";
        case HCI_OPCODE_HOST_NUMBER_OF_COMPLETED_PACKETS:
            return "HCI_HOST_NUMBER_OF_COMPLETED_PACKETS";
        case HCI_OPCODE_READ_LINK_SUPERVISION_TIMEOUT:
            return "HCI_READ_LINK_SUPERVISION_TIMEOUT";
        case HCI_OPCODE_WRITE_LINK_SUPERVISION_TIMEOUT:
            return "HCI_WRITE_LINK_SUPERVISION_TIMEOUT";
        case HCI_OPCODE_WRITE_CURRENT_IAC_LAP:
            return "HCI_WRITE_CURRENT_IAC_LAP";
        case HCI_OPCODE_READ_INQUIRY_SCAN_TYPE:
            return "HCI_READ_INQUIRY_SCAN_TYPE";
        case HCI_OPCODE_WRITE_INQUIRY_SCAN_TYPE:
            return "HCI_WRITE_INQUIRY_SCAN_TYPE";
        case HCI_OPCODE_READ_INQUIRY_MODE:
            return "HCI_READ_INQUIRY_MODE";
        case HCI_OPCODE_WRITE_INQUIRY_MODE:
            return "HCI_WRITE_INQUIRY_MODE";
        case HCI_OPCODE_READ_PAGE_SCAN_TYPE:
            return "HCI_READ_PAGE_SCAN_TYPE";
        case HCI_OPCODE_WRITE_PAGE_SCAN_TYPE:
            return "HCI_WRITE_PAGE_SCAN_TYPE";
        case HCI_OPCODE_WRITE_EXTENDED_INQUIRY_RESPONSE:
            return "HCI_WRITE_EXTENDED_INQUIRY_RESPONSE";
        case HCI_OPCODE_READ_SIMPLE_PAIRING_MODE:
            return "HCI_READ_SIMPLE_PAIRING_MODE";
        case HCI_OPCODE_WRITE_SIMPLE_PAIRING_MODE:
            return "HCI_WRITE_SIMPLE_PAIRING_MODE";
        case HCI_OPCODE_READ_LOCAL_OOB_DATA:
            return "HCI_READ_LOCAL_OOB_DATA";
        case HCI_OPCODE_WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING:
            return "HCI_WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING";
        case HCI_OPCODE_READ_LE_HOST_SUPPORTED:
            return "HCI_READ_LE_HOST_SUPPORTED";
        case HCI_OPCODE_WRITE_LE_HOST_SUPPORTED:
            return "HCI_WRITE_LE_HOST_SUPPORTED";
        case HCI_OPCODE_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT:
            return "HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT";
        case HCI_OPCODE_READ_LOCAL_EXTENDED_OOB_DATA:
            return "HCI_READ_LOCAL_EXTENDED_OOB_DATA";
        case HCI_OPCODE_READ_LOOPBACK_MODE:
            return "HCI_READ_LOOPBACK_MODE";
        case HCI_OPCODE_WRITE_LOOPBACK_MODE:
            return "HCI_WRITE_LOOPBACK_MODE";
        case HCI_OPCODE_ENABLE_DEVICE_UNDER_TEST_MODE:
            return "HCI_ENABLE_DEVICE_UNDER_TEST_MODE";
        case HCI_OPCODE_WRITE_SIMPLE_PAIRING_DEBUG_MODE:
            return "HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE";
        case HCI_OPCODE_WRITE_SECURE_CONNECTIONS_TEST_MODE:
            return "HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE";
        case HCI_OPCODE_READ_LOCAL_VERSION_INFORMATION:
            return "HCI_READ_LOCAL_VERSION_INFORMATION";
        case HCI_OPCODE_READ_LOCAL_SUPPORTED_COMMANDS:
            return "HCI_READ_LOCAL_SUPPORTED_COMMANDS";
        case HCI_OPCODE_READ_LOCAL_SUPPORTED_FEATURES:
            return "HCI_READ_LOCAL_SUPPORTED_FEATURES";
        case HCI_OPCODE_READ_BUFFER_SIZE:
            return "HCI_READ_BUFFER_SIZE";
        case HCI_OPCODE_READ_BD_ADDR:
            return "HCI_READ_BD_ADDR";
        case HCI_OPCODE_READ_RSSI:
            return "HCI_READ_RSSI";
        case HCI_OPCODE_READ_ENCRYPTION_KEY_SIZE:
            return "HCI_READ_ENCRYPTION_KEY_SIZE";
        default:
            return "HCI_OPCODE_XXXX\n";
    }
}

void dump_reset_complete(HCI_RESET_COMPLETE_PACKET* packet)
{
    printf("reset complete status 0x%x\n", packet->status);
}

void dump_read_bd_addr_complete(HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET* packet)
{
    printf("read local address complete, status 0x%x addr %s\n", packet->status, bda_to_string(packet->addr));
}

void dump_read_buffer_size_complete(HCI_READ_BUFFER_SIZE_COMPLETE_PACKET* packet)
{
    printf("buffers status %x HC_ACL_Data_Packet_Length %u HC_Total_Num_ACL_Data_Packets %u\n", packet->status, packet->hc_acl_data_packet_length, packet->hc_total_num_acl_data_packets);
}

void dump_read_simple_pairing_mode_complete(HCI_READ_SIMPLE_PAIRING_MODE_COMPLETE_PACKET* packet)
{
    printf("read simple pairing mode complete, status 0x%x mode %u\n", packet->status, packet->simple_pairing_mode);
}

void dump_read_local_name_complete(HCI_READ_LOCAL_NAME_COMPLETE_PACKET* packet)
{
    char local_name[HCI_MAX_LOCAL_NAME_SIZE + 1] = { };
    memcpy(local_name, packet->local_name, HCI_MAX_LOCAL_NAME_SIZE);
    printf("read local name complete, status 0x%x name %s\n", packet->status, local_name);
}

void dump_auth_code_complete(HCI_AUTH_CODE_COMPLETE_PACKET* packet, const char* name)
{
    printf("%s addr %s status 0x%x\n", name, bda_to_string(packet->addr), packet->status);
}

void dump_write_scan_enable_complete(HCI_WRITE_SCAN_ENABLE_COMPLETE_PACKET* packet)
{
    printf("write_scan_enable complete status 0x%x\n", packet->status);
}

void dump_write_class_of_device_complete(HCI_WRITE_CLASS_OF_DEVICE_COMPLETE_PACKET* packet)
{
    printf("write_class_of_device complete status 0x%x\n", packet->status);
}

void dump_write_local_name_complete(HCI_WRITE_LOCAL_NAME_COMPLETE_PACKET* packet)
{
    printf("write_local_name complete status 0x%x\n", packet->status);
}

void dump_write_current_iac_lap_complete(HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_PACKET* packet)
{
    printf("write_current_iac_lap complete status 0x%x\n", packet->status);
}

void dump_write_authentication_enable_complete(HCI_WRITE_AUTHENTICATION_ENABLE_COMPLETE_PACKET* packet)
{
    printf("write_authentication_enable complete status 0x%x\n", packet->status);
}

void dump_write_encryption_mode_complete(HCI_WRITE_ENCRYPTION_MODE_COMPLETE_PACKET* packet)
{
    printf("write_encryption_mode complete status 0x%x\n", packet->status);
}

void dump_write_default_link_policy_settings_complete(HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_COMPLETE_PACKET* packet)
{
    printf("write_default_link_policy_settings complete status 0x%x\n", packet->status);
}

void dump_write_secure_connections_host_support_complete(HCI_WRITE_SECURE_CONNECTION_HOST_SUPPORT_COMPLETE_PACKET* packet)
{
    printf("write_secure_connections_host_support complete status 0x%x\n", packet->status);
}

void dump_write_pin_type(HCI_WRITE_PIN_TYPE_COMPLETE_PACKET* packet)
{
    printf("handle_write_pin_type complete status 0x%x\n", packet->status);
}

void dump_command(HCI_COMMAND_PACKET* packet)
{
    printf("%s\n", get_hci_op_code_name(packet->op_code));
}

void dump_command_complete(HCI_COMMAND_COMPLETE_EVENT_PACKET* packet)
{
    switch (packet->op_code)
    {
        case HCI_OPCODE_RESET:
            dump_reset_complete((HCI_RESET_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_BD_ADDR:
            dump_read_bd_addr_complete((HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_BUFFER_SIZE:
            dump_read_buffer_size_complete((HCI_READ_BUFFER_SIZE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_LOCAL_NAME:
            dump_read_local_name_complete((HCI_READ_LOCAL_NAME_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_READ_SIMPLE_PAIRING_MODE:
            dump_read_simple_pairing_mode_complete((HCI_READ_SIMPLE_PAIRING_MODE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_PIN_CODE_REQUEST_REPLY:
            dump_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_pin_code_request_reply");
            break;
        case HCI_OPCODE_PIN_CODE_REQUEST_NEGATIVE_REPLY:
            dump_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_pin_code_request_negative_reply");
            break;
        case HCI_OPCODE_LINK_KEY_REQUEST_REPLY:
            dump_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_link_key_request_reply");
            break;
        case HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY:
            dump_auth_code_complete((HCI_AUTH_CODE_COMPLETE_PACKET*)packet, "hci_link_key_request_negative_reply");
            break;
        case HCI_OPCODE_WRITE_SCAN_ENABLE:
            dump_write_scan_enable_complete((HCI_WRITE_SCAN_ENABLE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_CLASS_OF_DEVICE:
            dump_write_class_of_device_complete((HCI_WRITE_CLASS_OF_DEVICE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_LOCAL_NAME:
            dump_write_local_name_complete((HCI_WRITE_LOCAL_NAME_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_CURRENT_IAC_LAP:
            dump_write_current_iac_lap_complete((HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_AUTHENTICATION_ENABLE:
            dump_write_authentication_enable_complete((HCI_WRITE_AUTHENTICATION_ENABLE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_SET_CONNECTION_ENCRYPTION:
            printf("set_connection_encryption complete\n"); // no params
            break;
        case HCI_OPCODE_WRITE_DEFAULT_LINK_POLICY_SETTINGS:
            dump_write_default_link_policy_settings_complete((HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT:
            dump_write_secure_connections_host_support_complete((HCI_WRITE_SECURE_CONNECTION_HOST_SUPPORT_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_PIN_TYPE:
            dump_write_pin_type((HCI_WRITE_PIN_TYPE_COMPLETE_PACKET*)packet);
            break;
        case HCI_OPCODE_WRITE_ENCRYPTION_MODE:
            dump_write_encryption_mode_complete((HCI_WRITE_ENCRYPTION_MODE_COMPLETE_PACKET*)packet);
            break;
        default:
            printf("unhandled command complete 0x%04x\n", packet->op_code);
            break;
    }
}

void dump_command_status(HCI_COMMAND_STATUS_EVENT_PACKET* packet)
{
    printf("%s status 0x%x\n", get_hci_op_code_name(packet->op_code), packet->status);
}

void dump_number_of_completed_packets(const uint8_t* packet, uint16_t size)
{
    uint8_t num_handles = packet[3];
    const uint8_t* p = packet + 4;

    printf("number_of_completed_packets");
    for (int i = 0; i < num_handles; i++)
    {
        uint16_t con_handle = read_uint16(p);
        uint16_t num_completed = read_uint16(p + 2);

        printf(" handle 0x%x completed %u", con_handle, num_completed);

        p += 4;
    }
    printf("\n");
}

void dump_qos_setup_complete(HCI_QOS_SETUP_COMPLETE_EVENT_PACKET* packet)
{
    printf("qos setup con_handle 0x%x status 0x%x "
        "service_type 0x%x token_rate %u peak_bandwidth %u "
        "latency %u delay_variation %u\n",
        packet->con_handle, packet->status,
        packet->service_type, packet->token_rate, packet->peak_bandwidth,
        packet->latency, packet->delay_variation);
}

void dump_max_slots_changed(HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET* packet)
{
    printf("max slots changed handle 0x%x lmp_max_slots %u\n", packet->con_handle, packet->lmp_max_slots);
}

void dump_role_change(HCI_ROLE_CHANGE_EVENT_PACKET* packet)
{
    printf("role change for %s status 0x%x new_role %u\n", bda_to_string(packet->addr), packet->status, packet->new_role);
}

void dump_link_key_notification(HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET* packet)
{
    printf("new link key for %s type %u", bda_to_string(packet->addr), packet->key_type);
    for (int i = 0; i < HCI_LINK_KEY_SIZE; i++)
    {
        printf(" %02x", packet->link_key[i]);
    }
    printf("\n");
}

void dump_connection_complete(HCI_CONNECTION_COMPLETE_EVENT_PACKET* packet)
{
    printf("connection complete addr %s status 0x%02x con_handle 0x%x, link_type %u encrypted %u\n", bda_to_string(packet->addr), packet->status, packet->con_handle, packet->link_type, packet->encryption_enabled);
}

void dump_disconnection_complete(HCI_DISCONNECTION_COMPLETE_EVENT_PACKET* packet)
{
    printf("disconnected handle 0x%04x status 0x%x reason 0x%x\n", packet->con_handle, packet->status, packet->reason);
}

void dump_encryption_change(HCI_ENCRYPTION_CHANGE_EVENT_PACKET* packet)
{
    printf("encryption changed status 0x%x handle 0x%x encryption_enabled %u\n", packet->status, packet->con_handle, packet->encryption_enabled);
}

void dump_mode_change(HCI_MODE_CHANGE_EVENT_PACKET* packet)
{
    printf("mode changed handle 0x%x status 0x%x current_mode 0x%x interval 0x%x\n", packet->con_handle, packet->status, packet->current_mode, packet->interval);
}

void dump_l2cap_connection_request(L2CAP_CONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap connection request con_handle 0x%x id 0x%x psm 0x%x source_cid 0x%x\n", packet->con_handle, packet->identifier, packet->psm, packet->source_cid);

}

void dump_l2cap_connection_response(L2CAP_CONNECTION_RESPONSE_PACKET* packet)
{
    printf("l2cap conn response con_handle 0x%x id 0x%x dest_cid 0x%x source_cid 0x%x result 0x%x status 0x%x\n",
        packet->con_handle, packet->identifier, packet->dest_cid, packet->source_cid, packet->result, packet->status);
}

void dump_l2cap_command_reject(L2CAP_COMMAND_REJECT_PACKET* packet)
{
    printf("l2cap cmd rejected con_handle 0x%x id 0x%x reason 0x%02x\n", packet->con_handle, packet->identifier, packet->reason);
}

void dump_l2cap_config_request(L2CAP_CONFIG_REQUEST_PACKET* request_packet)
{
    uint16_t options_size = request_packet->payload_size - 4;

    printf("l2cap config request con_handle 0x%x id 0x%x dest_cid 0x%x options_size %u options", request_packet->con_handle, request_packet->identifier, request_packet->dest_cid, options_size);
    dump_l2cap_config_options(request_packet->options, options_size);
    printf("\n");
}

void dump_l2cap_config_response(L2CAP_CONFIG_RESPONSE_PACKET* packet)
{
    uint16_t options_size = packet->payload_size - 6;

    printf("l2cap config response con_handle 0x%x id 0x%x source_cid 0x%x result 0x%x options_size %u options", packet->con_handle, packet->identifier, packet->source_cid, packet->result, options_size);
    dump_l2cap_config_options(packet->options, options_size);
    printf("\n");
}

void dump_l2cap_disconnection_request(L2CAP_DISCONNECTION_REQUEST_PACKET* packet)
{
    printf("l2cap disconnect request con_handle 0x%x id 0x%x dest_cid 0x%0x source_cid 0x%x\n", packet->con_handle, packet->identifier, packet->dest_cid, packet->source_cid);
}

void dump_l2cap_disconnection_response(L2CAP_DISCONNECTION_RESPONSE_PACKET* packet)
{
    printf("l2cap disconnect response con_handle 0x%x id 0x%x dest_cid 0x%0x source_cid 0x%x\n", packet->con_handle, packet->identifier, packet->dest_cid, packet->source_cid);
}

void dump_l2cap_signal_channel(L2CAP_SIGNAL_CHANNEL_PACKET* packet)
{
    switch (packet->code)
    {
        case L2CAP_CONNECTION_REQUEST:
            dump_l2cap_connection_request((L2CAP_CONNECTION_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONNECTION_RESPONSE:
            dump_l2cap_connection_response((L2CAP_CONNECTION_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_CONFIG_REQUEST:
            dump_l2cap_config_request((L2CAP_CONFIG_REQUEST_PACKET*)packet);
            break;
        case L2CAP_CONFIG_RESPONSE:
            dump_l2cap_config_response((L2CAP_CONFIG_RESPONSE_PACKET*)packet);
            break;
        case L2CAP_COMMAND_REJECT:
            dump_l2cap_command_reject((L2CAP_COMMAND_REJECT_PACKET*)packet);
            break;
        case L2CAP_DISCONNECTION_REQUEST:
            dump_l2cap_disconnection_request((L2CAP_DISCONNECTION_REQUEST_PACKET*)packet);
            break;
        case L2CAP_DISCONNECTION_RESPONSE:
            dump_l2cap_disconnection_response((L2CAP_DISCONNECTION_RESPONSE_PACKET*)packet);
            break;
        default:
            printf("unhandled signal channel code 0x%x\n", packet->code);
            break;
    }
}

static void dump_l2cap_config_options(uint8_t* options, uint16_t options_size)
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
                printf(" mtu %u", mtu_option->mtu);
                break;
            }
            case L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION_TYPE:
            {
                L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION* fto_option = (L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION*)option;
                printf(" flush_timeout %u", fto_option->flush_timeout);
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

void dump_packet(uint8_t io_direction, const uint8_t* packet, uint16_t size)
{
    //#ifdef WII_REMOTE_HOST
        //static int skip;
        //uint8_t* hid4 = packet + size - 4;
        //uint8_t* hid19 = packet + size - 19;
        // if (
        //     ((size >= 4 && memcmp(hid4, (uint8_t*)"\xa1\x30\x00\x00", 4) == 0) ||
        //     (size >= 19 && memcmp(hid19, (uint8_t*)"\xa1\x33\x20\x60\x80\x81\x9c\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 19) == 0)))
        // {
        //     skip++;
        //     return;
        // }

    //#endif

    HCI_ACL_PACKET* acl_packet = (HCI_ACL_PACKET*)packet;
    HCI_EVENT_PACKET* event_packet = (HCI_EVENT_PACKET*)packet;
    L2CAP_PACKET* l2cap_packet = (L2CAP_PACKET*)packet;

    // if (acl_packet->type == HCI_EVENT_PACKET_TYPE &&
    //     event_packet->event_code == HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS)
    // {
    //     return;
    // }

    static uint16_t last_channel[2];

    for (int i = 0; i < size; i++)
    {
        if (i % DUMP_WIDTH == 0 && i == 0)
        {
            printf("%s: (%02x)", IO_DIRECTION_TAG(io_direction), size);
        }
        else
        {
            printf("          ");
        }
        int j = 0;
        for (; j < DUMP_WIDTH && i < size; i++, j++)
        {
            printf(" %02x", packet[i]);
        }
        for (; j < DUMP_WIDTH; j++)
        {
            printf("   ");
        }
        if (i + 1 < size)
        {
            printf("\n");
        }
    }
    printf("  ");

    switch (acl_packet->type)
    {
        case HCI_COMMAND_PACKET_TYPE:
            dump_command((HCI_COMMAND_PACKET*)packet);
            break;
        case HCI_EVENT_PACKET_TYPE:
            switch (event_packet->event_code)
            {
                case HCI_EVENT_COMMAND_COMPLETE:
                    dump_command_complete((HCI_COMMAND_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_COMMAND_STATUS:
                    dump_command_status((HCI_COMMAND_STATUS_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_MAX_SLOTS_CHANGED:
                    dump_max_slots_changed((HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
                    dump_number_of_completed_packets(packet, size);
                    break;
                case HCI_EVENT_QOS_SETUP_COMPLETE:
                    dump_qos_setup_complete((HCI_QOS_SETUP_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_HARDWARE_ERROR:
                    printf("hardware error\n");
                    break;
                case HCI_EVENT_ROLE_CHANGE:
                    dump_role_change((HCI_ROLE_CHANGE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_LINK_KEY_NOTIFICATION:
                    dump_link_key_notification((HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_CONNECTION_COMPLETE:
                    dump_connection_complete((HCI_CONNECTION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    dump_disconnection_complete((HCI_DISCONNECTION_COMPLETE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_MODE_CHANGE:
                    dump_mode_change((HCI_MODE_CHANGE_EVENT_PACKET*)packet);
                    break;
                case HCI_EVENT_ENCRYPTION_CHANGE:
                    dump_encryption_change((HCI_ENCRYPTION_CHANGE_EVENT_PACKET*)packet);
                    break;
                default:
                    printf("%s\n", get_hci_event_name(event_packet->event_code));
                    break;
            }
            break;
        case HCI_ACL_PACKET_TYPE:
        {
            if (acl_packet->broadcast_flag != 0)
            {
                printf("~~~~~~~~~~~~~~~~~~~~~~broadcast_flag %0x~~~~~~~~~~~~~~~~\n", acl_packet->broadcast_flag);
            }

            uint16_t local_channel = l2cap_packet->channel;
            if (acl_packet->packet_boundary_flag == L2CAP_PB_FRAGMENT || acl_packet->packet_boundary_flag == 0)
            {
                local_channel = last_channel[io_direction - 1];
            }

            // if (io_direction == OUTPUT_PACKET)
            // {
            //     if (local_channel == wii_controller.sdp_cid)
            //     {
            //         local_channel = SDP_LOCAL_CID;
            //     }
            //     else if (local_channel == wii_controller.data_cid)
            //     {
            //         local_channel = WII_DATA_LOCAL_CID;
            //     }
            //     else if (local_channel == wii_controller.control_cid)
            //     {
            //         local_channel = WII_CONTROL_LOCAL_CID;
            //     }
            // }

            if (acl_packet->packet_boundary_flag == L2CAP_PB_FIRST_FLUSH || acl_packet->packet_boundary_flag == 0)
            {
                last_channel[io_direction - 1] = l2cap_packet->channel;
                switch (local_channel)
                {
                    case L2CAP_SIGNAL_CHANNEL:
                        dump_l2cap_signal_channel((L2CAP_SIGNAL_CHANNEL_PACKET*)packet);
                        break;
                    // case WII_CONTROL_LOCAL_CID:
                    //     printf("wii control\n");
                    //     break;
                    // case WII_DATA_LOCAL_CID:
                    //     printf("wii data\n");
                    //     break;
                    // case SDP_LOCAL_CID:
                    //     printf("sdp\n");
                    //     break;
                    default:
                        printf("l2cap channel 0x%x con_handle 0x%x\n", l2cap_packet->channel, l2cap_packet->con_handle);
                        break;
                }
            }
            else if (acl_packet->packet_boundary_flag == L2CAP_PB_FRAGMENT)
            {
                switch (local_channel)
                {
                    // case SDP_LOCAL_CID:
                    //     printf("sdp\n");
                    //     break;
                    default:
                        printf("acl fragment channel 0x%x con_handle 0x%x\n", last_channel[io_direction - 1], l2cap_packet->con_handle);
                        break;
                }
            }
            else
            {
                printf("bad packet_boundary_flag 0x%x\n", acl_packet->packet_boundary_flag);
            }


            break;
        }
        default:
            printf("unhandled packet type 0x%02x\n", acl_packet->type);
            break;
    }
}