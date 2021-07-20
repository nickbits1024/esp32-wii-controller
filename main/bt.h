#ifndef BT_H
#define BT_H

#define HCI_COMMAND_PACKET_TYPE     0x01
#define HCI_ACL_PACKET_TYPE         0x02
#define HCI_SCO_PACKET_TYPE         0x03
#define HCI_EVENT_PACKET_TYPE       0x04

#define HCI_VARIABLE_PIN_TYPE   0x00
#define HCI_FIXED_PIN_TYPE      0x01

#define OGF_LINK_CONTROL                0x01
#define OGF_LINK_POLICY                 0x02
#define OGF_CONTROLLER_BASEBAND         0x03
#define OGF_INFORMATIONAL_PARAMETERS    0x04
#define OGF_STATUS_PARAMETERS           0x05
#define OGF_TESTING                     0x06
#define OGF_LE_CONTROLLER               0x08
#define OGF_VENDOR                      0x3f

#define HID_INPUT_REPORT                0xa1
#define HID_OUTPUT_REPORT               0xa2

#define HCI_OPCODE(ogf, ocf) ((ocf) | ((ogf) << 10))

#define HCI_OPCODE_INQUIRY HCI_OPCODE(OGF_LINK_CONTROL, 0x01)
#define HCI_OPCODE_INQUIRY_CANCEL HCI_OPCODE(OGF_LINK_CONTROL, 0x02)
#define HCI_OPCODE_CREATE_CONNECTION HCI_OPCODE(OGF_LINK_CONTROL, 0x05)
#define HCI_OPCODE_DISCONNECT HCI_OPCODE(OGF_LINK_CONTROL, 0x06)
#define HCI_OPCODE_CREATE_CONNECTION_CANCEL HCI_OPCODE(OGF_LINK_CONTROL, 0x08)
#define HCI_OPCODE_ACCEPT_CONNECTION_REQUEST HCI_OPCODE(OGF_LINK_CONTROL, 0x09)
#define HCI_OPCODE_REJECT_CONNECTION_REQUEST HCI_OPCODE(OGF_LINK_CONTROL, 0x0a)
#define HCI_OPCODE_LINK_KEY_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x0b)
#define HCI_OPCODE_LINK_KEY_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x0c)
#define HCI_OPCODE_PIN_CODE_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x0d)
#define HCI_OPCODE_PIN_CODE_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x0e)
#define HCI_OPCODE_CHANGE_CONNECTION_PACKET_TYPE HCI_OPCODE(OGF_LINK_CONTROL, 0x0f)
#define HCI_OPCODE_AUTHENTICATION_REQUESTED HCI_OPCODE(OGF_LINK_CONTROL, 0x11)
#define HCI_OPCODE_SET_CONNECTION_ENCRYPTION HCI_OPCODE(OGF_LINK_CONTROL, 0x13)
#define HCI_OPCODE_CHANGE_CONNECTION_LINK_KEY HCI_OPCODE(OGF_LINK_CONTROL, 0x15)
#define HCI_OPCODE_REMOTE_NAME_REQUEST HCI_OPCODE(OGF_LINK_CONTROL, 0x19)
#define HCI_OPCODE_REMOTE_NAME_REQUEST_CANCEL HCI_OPCODE(OGF_LINK_CONTROL, 0x1A)
#define HCI_OPCODE_READ_REMOTE_SUPPORTED_FEATURES_COMMAND HCI_OPCODE(OGF_LINK_CONTROL, 0x1B)
#define HCI_OPCODE_READ_REMOTE_EXTENDED_FEATURES_COMMAND HCI_OPCODE(OGF_LINK_CONTROL, 0x1C)
#define HCI_OPCODE_READ_REMOTE_VERSION_INFORMATION HCI_OPCODE(OGF_LINK_CONTROL, 0x1D)
#define HCI_OPCODE_SETUP_SYNCHRONOUS_CONNECTION HCI_OPCODE(OGF_LINK_CONTROL, 0x0028)
#define HCI_OPCODE_ACCEPT_SYNCHRONOUS_CONNECTION HCI_OPCODE(OGF_LINK_CONTROL, 0x0029)
#define HCI_OPCODE_IO_CAPABILITY_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x2b)
#define HCI_OPCODE_USER_CONFIRMATION_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x2c)
#define HCI_OPCODE_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x2d)
#define HCI_OPCODE_USER_PASSKEY_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x2e)
#define HCI_OPCODE_USER_PASSKEY_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x2f)
#define HCI_OPCODE_REMOTE_OOB_DATA_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x30)
#define HCI_OPCODE_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x33)
#define HCI_OPCODE_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x34)
#define HCI_OPCODE_ENHANCED_SETUP_SYNCHRONOUS_CONNECTION HCI_OPCODE(OGF_LINK_CONTROL, 0x3d)
#define HCI_OPCODE_ENHANCED_ACCEPT_SYNCHRONOUS_CONNECTION HCI_OPCODE(OGF_LINK_CONTROL, 0x3e)
#define HCI_OPCODE_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY HCI_OPCODE(OGF_LINK_CONTROL, 0x45)
#define HCI_OPCODE_SNIFF_MODE HCI_OPCODE(OGF_LINK_POLICY,   0x03)
#define HCI_OPCODE_EXIT_SNIFF_MODE HCI_OPCODE(OGF_LINK_POLICY,   0x04)
#define HCI_OPCODE_QOS_SETUP HCI_OPCODE(OGF_LINK_POLICY,   0x07)
#define HCI_OPCODE_ROLE_DISCOVERY HCI_OPCODE(OGF_LINK_POLICY,   0x09)
#define HCI_OPCODE_SWITCH_ROLE_COMMAND HCI_OPCODE(OGF_LINK_POLICY,   0x0b)
#define HCI_OPCODE_READ_LINK_POLICY_SETTINGS HCI_OPCODE(OGF_LINK_POLICY,   0x0c)
#define HCI_OPCODE_WRITE_LINK_POLICY_SETTINGS HCI_OPCODE(OGF_LINK_POLICY,   0x0d)
#define HCI_OPCODE_WRITE_DEFAULT_LINK_POLICY_SETTINGS HCI_OPCODE(OGF_LINK_POLICY,   0x0F)
#define HCI_OPCODE_FLOW_SPECIFICATION HCI_OPCODE(OGF_LINK_POLICY,   0x10)
#define HCI_OPCODE_SNIFF_SUBRATING HCI_OPCODE(OGF_LINK_POLICY,   0x11)
#define HCI_OPCODE_SET_EVENT_MASK HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x01)
#define HCI_OPCODE_RESET HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x03)
#define HCI_OPCODE_FLUSH HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x08)
#define HCI_OPCODE_READ_PIN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x09)
#define HCI_OPCODE_WRITE_PIN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x0A)
#define HCI_OPCODE_DELETE_STORED_LINK_KEY HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x12)
#define HCI_OPCODE_WRITE_LOCAL_NAME HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x13)
#define HCI_OPCODE_READ_LOCAL_NAME HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x14)
#define HCI_OPCODE_READ_PAGE_TIMEOUT HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x17)
#define HCI_OPCODE_WRITE_PAGE_TIMEOUT HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x18)
#define HCI_OPCODE_WRITE_SCAN_ENABLE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x1A)
#define HCI_OPCODE_READ_PAGE_SCAN_ACTIVITY HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x1B)
#define HCI_OPCODE_WRITE_PAGE_SCAN_ACTIVITY HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x1C)
#define HCI_OPCODE_READ_INQUIRY_SCAN_ACTIVITY HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x1D)
#define HCI_OPCODE_WRITE_INQUIRY_SCAN_ACTIVITY HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x1E)
#define HCI_OPCODE_WRITE_AUTHENTICATION_ENABLE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x20)
#define HCI_OPCODE_WRITE_ENCRYPTION_MODE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x22)
#define HCI_OPCODE_WRITE_CLASS_OF_DEVICE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x24)
#define HCI_OPCODE_READ_NUM_BROADCAST_RETRANSMISSIONS HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x29)
#define HCI_OPCODE_WRITE_NUM_BROADCAST_RETRANSMISSIONS HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x2a)
#define HCI_OPCODE_READ_TRANSMIT_POWER_LEVEL HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x2D)
#define HCI_OPCODE_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x2f)
#define HCI_OPCODE_SET_CONTROLLER_TO_HOST_FLOW_CONTROL HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x31)
#define HCI_OPCODE_HOST_BUFFER_SIZE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x33)
#define HCI_OPCODE_HOST_NUMBER_OF_COMPLETED_PACKETS HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x35)
#define HCI_OPCODE_READ_LINK_SUPERVISION_TIMEOUT HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x36)
#define HCI_OPCODE_WRITE_LINK_SUPERVISION_TIMEOUT HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x37)
#define HCI_OPCODE_WRITE_CURRENT_IAC_LAP HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x3A)
#define HCI_OPCODE_READ_INQUIRY_SCAN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x42)
#define HCI_OPCODE_WRITE_INQUIRY_SCAN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x43)
#define HCI_OPCODE_READ_INQUIRY_MODE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x44)
#define HCI_OPCODE_WRITE_INQUIRY_MODE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x45)
#define HCI_OPCODE_READ_PAGE_SCAN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x46)
#define HCI_OPCODE_WRITE_PAGE_SCAN_TYPE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x47)
#define HCI_OPCODE_WRITE_EXTENDED_INQUIRY_RESPONSE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x52)
#define HCI_OPCODE_READ_SIMPLE_PAIRING_MODE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x55)
#define HCI_OPCODE_WRITE_SIMPLE_PAIRING_MODE HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x56)
#define HCI_OPCODE_READ_LOCAL_OOB_DATA HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x57)
#define HCI_OPCODE_WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x5B)
#define HCI_OPCODE_READ_LE_HOST_SUPPORTED HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x6c)
#define HCI_OPCODE_WRITE_LE_HOST_SUPPORTED HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x6d)
#define HCI_OPCODE_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x7a)
#define HCI_OPCODE_READ_LOCAL_EXTENDED_OOB_DATA HCI_OPCODE(OGF_CONTROLLER_BASEBAND,  0x7d)
#define HCI_OPCODE_READ_LOOPBACK_MODE HCI_OPCODE(OGF_TESTING, 0x01)
#define HCI_OPCODE_WRITE_LOOPBACK_MODE HCI_OPCODE(OGF_TESTING, 0x02)
#define HCI_OPCODE_ENABLE_DEVICE_UNDER_TEST_MODE HCI_OPCODE(OGF_TESTING, 0x03)
#define HCI_OPCODE_WRITE_SIMPLE_PAIRING_DEBUG_MODE HCI_OPCODE(OGF_TESTING, 0x04)
#define HCI_OPCODE_WRITE_SECURE_CONNECTIONS_TEST_MODE HCI_OPCODE(OGF_TESTING, 0x0a)
#define HCI_OPCODE_READ_LOCAL_VERSION_INFORMATION HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x01)
#define HCI_OPCODE_READ_LOCAL_SUPPORTED_COMMANDS HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x02)
#define HCI_OPCODE_READ_LOCAL_SUPPORTED_FEATURES HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x03)
#define HCI_OPCODE_READ_BUFFER_SIZE HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x05)
#define HCI_OPCODE_READ_BD_ADDR HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x09)
#define HCI_OPCODE_READ_RSSI HCI_OPCODE(OGF_STATUS_PARAMETERS, 0x05)
#define HCI_OPCODE_READ_ENCRYPTION_KEY_SIZE HCI_OPCODE(OGF_STATUS_PARAMETERS, 0x08)

#define HCI_EVENT_INQUIRY_COMPLETE                         0x01
#define HCI_EVENT_INQUIRY_RESULT                           0x02
#define HCI_EVENT_CONNECTION_COMPLETE                      0x03
#define HCI_EVENT_CONNECTION_REQUEST                       0x04
#define HCI_EVENT_DISCONNECTION_COMPLETE                   0x05
#define HCI_EVENT_AUTHENTICATION_COMPLETE                  0x06
#define HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE             0x07
#define HCI_EVENT_ENCRYPTION_CHANGE                        0x08
#define HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE      0x09
#define HCI_EVENT_MASTER_LINK_KEY_COMPLETE                 0x0A
#define HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE  0x0B
#define HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE 0x0C
#define HCI_EVENT_QOS_SETUP_COMPLETE                       0x0D
#define HCI_EVENT_COMMAND_COMPLETE                         0x0E
#define HCI_EVENT_COMMAND_STATUS                           0x0F
#define HCI_EVENT_HARDWARE_ERROR                           0x10
#define HCI_EVENT_FLUSH_OCCURRED                           0x11
#define HCI_EVENT_ROLE_CHANGE                              0x12
#define HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS              0x13
#define HCI_EVENT_MODE_CHANGE                              0x14
#define HCI_EVENT_RETURN_LINK_KEYS                         0x15
#define HCI_EVENT_PIN_CODE_REQUEST                         0x16
#define HCI_EVENT_LINK_KEY_REQUEST                         0x17
#define HCI_EVENT_LINK_KEY_NOTIFICATION                    0x18
#define HCI_EVENT_DATA_BUFFER_OVERFLOW                     0x1A
#define HCI_EVENT_MAX_SLOTS_CHANGED                        0x1B
#define HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE               0x1C
#define HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED           0x1D
#define HCI_EVENT_INQUIRY_RESULT_WITH_RSSI                 0x22
#define HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE   0x23
#define HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE          0x2C
#define HCI_EVENT_EXTENDED_INQUIRY_RESPONSE                0x2F
#define HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE          0x30
#define HCI_EVENT_IO_CAPABILITY_REQUEST                    0x31
#define HCI_EVENT_IO_CAPABILITY_RESPONSE                   0x32
#define HCI_EVENT_USER_CONFIRMATION_REQUEST                0x33
#define HCI_EVENT_USER_PASSKEY_REQUEST                     0x34
#define HCI_EVENT_REMOTE_OOB_DATA_REQUEST                  0x35
#define HCI_EVENT_SIMPLE_PAIRING_COMPLETE                  0x36
#define HCI_EVENT_USER_PASSKEY_NOTIFICATION                0x3B
#define HCI_EVENT_KEYPRESS_NOTIFICATION                    0x3C
#define HCI_EVENT_VENDOR_SPECIFIC                          0xFF
#define HCI_EVENT_TRANSPORT_SLEEP_MODE                     0x69
#define HCI_EVENT_TRANSPORT_READY                          0x6D
#define HCI_EVENT_TRANSPORT_PACKET_SENT                    0x6E
#define HCI_EVENT_SCO_CAN_SEND_NOW                         0x6F
#define L2CAP_EVENT_CHANNEL_OPENED                         0x70
#define L2CAP_EVENT_CHANNEL_CLOSED                         0x71
#define L2CAP_EVENT_INCOMING_CONNECTION                    0x72
#define L2CAP_EVENT_TIMEOUT_CHECK                          0x73
#define L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_REQUEST    0x76
#define L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE   0x77
#define L2CAP_EVENT_CAN_SEND_NOW                           0x78
#define L2CAP_EVENT_ERTM_BUFFER_RELEASED                   0x7e
#define L2CAP_EVENT_TRIGGER_RUN                            0x7f

/* ENUM_START: BLUETOOTH_ERROR_CODE */
#define ERROR_CODE_SUCCESS                                 0x00 
#define ERROR_CODE_UNKNOWN_HCI_COMMAND                     0x01
#define ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER           0x02
#define ERROR_CODE_HARDWARE_FAILURE                        0x03
#define ERROR_CODE_PAGE_TIMEOUT                            0x04
#define ERROR_CODE_AUTHENTICATION_FAILURE                  0x05
#define ERROR_CODE_PIN_OR_KEY_MISSING                      0x06
#define ERROR_CODE_MEMORY_CAPACITY_EXCEEDED                0x07
#define ERROR_CODE_CONNECTION_TIMEOUT                      0x08
#define ERROR_CODE_CONNECTION_LIMIT_EXCEEDED               0x09
#define ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED  0x0A
#define ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS           0x0B
#define ERROR_CODE_COMMAND_DISALLOWED                      0x0C
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES 0x0D
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_SECURITY_REASONS  0x0E 
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR 0x0F 
#define ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED      0x10
#define ERROR_CODE_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE  0x11 
#define ERROR_CODE_INVALID_HCI_COMMAND_PARAMETERS          0x12 
#define ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION       0x13
#define ERROR_CODE_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES 0x14 
#define ERROR_CODE_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF     0x15
#define ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST     0x16
#define ERROR_CODE_REPEATED_ATTEMPTS                       0x17
#define ERROR_CODE_PAIRING_NOT_ALLOWED                     0x18
#define ERROR_CODE_UNKNOWN_LMP_PDU                         0x19                  
#define ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE_UNSUPPORTED_LMP_FEATURE 0x1A
#define ERROR_CODE_SCO_OFFSET_REJECTED                     0x1B
#define ERROR_CODE_SCO_INTERVAL_REJECTED                   0x1C
#define ERROR_CODE_SCO_AIR_MODE_REJECTED                   0x1D
#define ERROR_CODE_INVALID_LMP_PARAMETERS_INVALID_LL_PARAMETERS 0x1E
#define ERROR_CODE_UNSPECIFIED_ERROR                       0x1F
#define ERROR_CODE_UNSUPPORTED_LMP_PARAMETER_VALUE_UNSUPPORTED_LL_PARAMETER_VALUE 0x20
#define ERROR_CODE_ROLE_CHANGE_NOT_ALLOWED                 0x21
#define ERROR_CODE_LMP_RESPONSE_TIMEOUT_LL_RESPONSE_TIMEOUT 0x22
#define ERROR_CODE_LMP_ERROR_TRANSACTION_COLLISION         0x23
#define ERROR_CODE_LMP_PDU_NOT_ALLOWED                     0x24
#define ERROR_CODE_ENCRYPTION_MODE_NOT_ACCEPTABLE          0x25
#define ERROR_CODE_LINK_KEY_CANNOT_BE_CHANGED              0x26
#define ERROR_CODE_REQUESTED_QOS_NOT_SUPPORTED             0x27
#define ERROR_CODE_INSTANT_PASSED                          0x28
#define ERROR_CODE_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED     0x29
#define ERROR_CODE_DIFFERENT_TRANSACTION_COLLISION         0x2A
#define ERROR_CODE_RESERVED                                0x2B
#define ERROR_CODE_QOS_UNACCEPTABLE_PARAMETER              0x2C
#define ERROR_CODE_QOS_REJECTED                            0x2D
#define ERROR_CODE_CHANNEL_CLASSIFICATION_NOT_SUPPORTED    0x2E
#define ERROR_CODE_INSUFFICIENT_SECURITY                   0x2F
#define ERROR_CODE_PARAMETER_OUT_OF_MANDATORY_RANGE        0x30
// #define ERROR_CODE_RESERVED
#define ERROR_CODE_ROLE_SWITCH_PENDING                     0x32
// #define ERROR_CODE_RESERVED
#define ERROR_CODE_RESERVED_SLOT_VIOLATION                 0x34
#define ERROR_CODE_ROLE_SWITCH_FAILED                      0x35
#define ERROR_CODE_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE     0x36
#define ERROR_CODE_SECURE_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST 0x37
#define ERROR_CODE_HOST_BUSY_PAIRING                       0x38
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND 0x39
#define ERROR_CODE_CONTROLLER_BUSY                         0x3A
#define ERROR_CODE_UNACCEPTABLE_CONNECTION_PARAMETERS      0x3B
#define ERROR_CODE_DIRECTED_ADVERTISING_TIMEOUT            0x3C
#define ERROR_CODE_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE 0x3D
#define ERROR_CODE_CONNECTION_FAILED_TO_BE_ESTABLISHED     0x3E
#define ERROR_CODE_MAC_CONNECTION_FAILED                   0x3F
#define ERROR_CODE_COARSE_CLOCK_ADJUSTMENT_REJECTED_BUT_WILL_TRY_TO_ADJUST_USING_CLOCK_DRAGGING 0x40

// BTstack defined ERRORS, mapped into BLuetooth status code range

#define BTSTACK_CONNECTION_TO_BTDAEMON_FAILED              0x50
#define BTSTACK_ACTIVATION_FAILED_SYSTEM_BLUETOOTH         0x51
#define BTSTACK_ACTIVATION_POWERON_FAILED                  0x52
#define BTSTACK_ACTIVATION_FAILED_UNKNOWN                  0x53
#define BTSTACK_NOT_ACTIVATED                              0x54
#define BTSTACK_BUSY                                       0x55
#define BTSTACK_MEMORY_ALLOC_FAILED                        0x56
#define BTSTACK_ACL_BUFFERS_FULL                           0x57

// l2cap errors - enumeration by the command that created them
#define L2CAP_COMMAND_REJECT_REASON_COMMAND_NOT_UNDERSTOOD 0x60
#define L2CAP_COMMAND_REJECT_REASON_SIGNALING_MTU_EXCEEDED 0x61
#define L2CAP_COMMAND_REJECT_REASON_INVALID_CID_IN_REQUEST 0x62

#define L2CAP_CONNECTION_RESPONSE_RESULT_SUCCESSFUL        0x63
#define L2CAP_CONNECTION_RESPONSE_RESULT_PENDING           0x64
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_PSM       0x65
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_SECURITY  0x66
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_RESOURCES 0x67
#define L2CAP_CONNECTION_RESPONSE_RESULT_ERTM_NOT_SUPPORTED 0x68

// should be L2CAP_CONNECTION_RTX_TIMEOUT
#define L2CAP_CONNECTION_RESPONSE_RESULT_RTX_TIMEOUT       0x69
#define L2CAP_CONNECTION_BASEBAND_DISCONNECT               0x6A
#define L2CAP_SERVICE_ALREADY_REGISTERED                   0x6B
#define L2CAP_DATA_LEN_EXCEEDS_REMOTE_MTU                  0x6C
#define L2CAP_SERVICE_DOES_NOT_EXIST                       0x6D
#define L2CAP_LOCAL_CID_DOES_NOT_EXIST                     0x6E
    
#define RFCOMM_MULTIPLEXER_STOPPED                         0x70
#define RFCOMM_CHANNEL_ALREADY_REGISTERED                  0x71
#define RFCOMM_NO_OUTGOING_CREDITS                         0x72
#define RFCOMM_AGGREGATE_FLOW_OFF                          0x73
#define RFCOMM_DATA_LEN_EXCEEDS_MTU                        0x74

#define HFP_REMOTE_REJECTS_AUDIO_CONNECTION                0x7F

#define SDP_HANDLE_ALREADY_REGISTERED                      0x80
#define SDP_QUERY_INCOMPLETE                               0x81
#define SDP_SERVICE_NOT_FOUND                              0x82
#define SDP_HANDLE_INVALID                                 0x83
#define SDP_QUERY_BUSY                                     0x84

#define ATT_HANDLE_VALUE_INDICATION_IN_PROGRESS            0x90 
#define ATT_HANDLE_VALUE_INDICATION_TIMEOUT                0x91
#define ATT_HANDLE_VALUE_INDICATION_DISCONNECT             0x92

#define GATT_CLIENT_NOT_CONNECTED                          0x93
#define GATT_CLIENT_BUSY                                   0x94
#define GATT_CLIENT_IN_WRONG_STATE                         0x95
#define GATT_CLIENT_DIFFERENT_CONTEXT_FOR_ADDRESS_ALREADY_EXISTS 0x96
#define GATT_CLIENT_VALUE_TOO_LONG                         0x97
#define GATT_CLIENT_CHARACTERISTIC_NOTIFICATION_NOT_SUPPORTED 0x98
#define GATT_CLIENT_CHARACTERISTIC_INDICATION_NOT_SUPPORTED   0x99 

#define BNEP_SERVICE_ALREADY_REGISTERED                    0xA0
#define BNEP_CHANNEL_NOT_CONNECTED                         0xA1
#define BNEP_DATA_LEN_EXCEEDS_MTU                          0xA2

// OBEX ERRORS
#define OBEX_UNKNOWN_ERROR                                 0xB0
#define OBEX_CONNECT_FAILED                                0xB1
#define OBEX_DISCONNECTED                                  0xB2
#define OBEX_NOT_FOUND                                     0xB3
#define OBEX_NOT_ACCEPTABLE                                0xB4

#define MESH_ERROR_APPKEY_INDEX_INVALID                     0xD0
/* ENUM_END */

#define GAP_IAC_GENERAL_INQUIRY                             0x9E8B33L
#define GAP_IAC_LIMITED_INQUIRY                             0x9E8B00L

#define BDA_SIZE                                            6
#define HCI_LINK_KEY_SIZE                                   16
#define HCI_MAX_PIN_CODE_SIZE                               16
#define HCI_CON_HANDLE_MASK                                 0x0fff
#define HCI_INQUIRY_SCAN_ENABLE                             0x1
#define HCI_PAGE_SCAN_ENABLE                                0x2
#define HCI_LINK_TYPE_SCO                                   0
#define HCI_LINK_TYPE_ACL                                   1
#define HCI_ROLE_MASTER                                     0
#define HCI_ROLE_SLAVE                                      1

#define HCI_LINK_POLICY_ENABLE_ROLE_SWITCH                  0x1
#define HCI_LINK_POLICY_ENABLE_HOLD_MODE                    0x2
#define HCI_LINK_POLICY_ENABLE_SNIFF_MODE                   0x4

#define HCI_MAX_LOCAL_NAME_SIZE         248

#define L2CAP_SIGNAL_CHANNEL            0x0001

#define L2CAP_COMMAND_REJECT            0x01
#define L2CAP_CONNECTION_REQUEST        0x02
#define L2CAP_CONNECTION_RESPONSE       0x03
#define L2CAP_CONFIG_REQUEST            0x04
#define L2CAP_CONFIG_RESPONSE           0x05
#define L2CAP_DISCONNECTION_REQUEST     0x06
#define L2CAP_DISCONNECTION_RESPONSE    0x07

#define L2CAP_CONNECTION_RESULT_SUCCESS     0x00
#define L2CAP_CONNECTION_RESULT_PENDING     0x01

#define L2CAP_AUTO_SIZE                 0

#define L2CAP_PB_FIRST_FLUSH            ((uint16_t)2)
#define L2CAP_PB_FRAGMENT               ((uint16_t)1)
#define L2CAP_BROADCAST_NONE            ((uint16_t)0)

#define HCI_QOS_NO_TRAFFIC              0
#define HCI_QOS_BEST_EFFORT             1
#define HCI_QOS_GUARANTEED              2

#define HCI_FLOW_CONTROL_OFF            0x0
#define HCI_FLOW_CONTROL_ACL            0x1
#define HCI_FLOW_CONTROL_SCO            0x2
#define HCI_FLOW_CONTROL_ACL_SCO        0x3

#define INVALID_HANDLE_VALUE            0xffff

typedef uint8_t bd_addr_t[BDA_SIZE];

extern bd_addr_t device_addr;

#define OUTPUT_PACKET 1
#define INPUT_PACKET 2

typedef struct
{
    uint8_t io_direction;
    uint16_t size;
    uint8_t packet[];
}
__attribute__((packed)) BT_PACKET_ENVELOPE;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint8_t data[];
}
__attribute__((packed)) HCI_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t params[];
}
__attribute__((packed)) HCI_COMMAND_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t con_handle;
    uint8_t flags;
    uint8_t service_type;
    uint32_t token_rate;
    uint32_t peak_bandwidth;
    uint32_t latency;
    uint32_t delay_variation;
}
__attribute__((packed)) HCI_QOS_SETUP_PACKET;


typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t lap[3];
    uint8_t duration;
    uint8_t num_responses;
}
__attribute__((packed)) HCI_INQUIRY_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t flow_control_enable;
}
__attribute__((packed)) HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t number_of_handles;
    uint8_t data[];
}
__attribute__((packed)) HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t host_acl_data_packet_length;
    uint8_t host_synchronous_data_packet_length;
    uint16_t host_total_num_acl_data_packets;
    uint16_t host_total_num_synchronous_data_packets;
}
__attribute__((packed)) HCI_HOST_BUFFER_SIZE_PACKET;

// typedef struct
// {
//     uint8_t type;
//     uint16_t op_code;
//     uint8_t params_size;
//     uint8_t flow_control_enable;
// }
// __attribute__((packed)) HCI_HOST_NUMBER_OF_COMPLETED_PACKETS_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t pin_type;
}
__attribute__((packed)) HCI_WRITE_PIN_TYPE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t pin_code_size;
    uint8_t pin_code[HCI_MAX_PIN_CODE_SIZE];
}
__attribute__((packed)) HCI_PIN_CODE_REQUEST_REPLY_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t psrm;
    uint8_t reserved;
    uint16_t clock_offset;
}
__attribute__((packed)) HCI_REMOTE_NAME_REQUEST_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint16_t packet_type;
    uint8_t psrm;
    uint8_t reserved;
    uint16_t clock_offset;
    uint8_t allow_role_switch;
}
__attribute__((packed)) HCI_CREATE_CONNECTION_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t role;
}
__attribute__((packed)) HCI_SWITCH_ROLE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t con_handle;
    uint8_t reason;
}
__attribute__((packed)) HCI_DISCONNECT_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t class_of_device[3];
}
__attribute__((packed)) HCI_WRITE_CLASS_OF_DEVICE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t local_name[HCI_MAX_LOCAL_NAME_SIZE];
}
__attribute__((packed)) HCI_WRITE_LOCAL_NAME_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t con_handle;
}
__attribute__((packed)) HCI_AUTHENTICATION_REQUESTED_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t encryption_mode;
}
__attribute__((packed)) HCI_WRITE_ENCRYPTION_MODE_PACKET;


typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_LINK_KEY_REQUEST_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t link_key[HCI_LINK_KEY_SIZE];
}
__attribute__((packed)) HCI_LINK_KEY_REQUEST_REPLY_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t scan_enable;
}
__attribute__((packed)) HCI_WRITE_SCAN_ENABLE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t authentication_enable;
}
__attribute__((packed)) HCI_WRITE_AUTHENTICATION_ENABLE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t con_handle;
    uint8_t encryption_enable;
}
__attribute__((packed)) HCI_SET_CONNECTION_ENCRYPTION_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint16_t default_link_policy_settings;
}
__attribute__((packed)) HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t secure_connections_host_support;
}
__attribute__((packed)) HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    uint8_t num_current_iac;
    uint8_t iac_lap[3];
}
__attribute__((packed)) HCI_WRITE_CURRENT_IAC_LAP_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t role;
}
__attribute__((packed)) HCI_ACCEPT_CONNECTION_REQUEST_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t op_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t reason;
}
__attribute__((packed)) HCI_REJECT_CONNECTION_REQUEST_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_RESET_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_HOST_BUFFER_SIZE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
    uint16_t hc_acl_data_packet_length;
    uint8_t hc_synchronous_data_packet_length;
    uint16_t hc_total_num_acl_data_packets;
    uint16_t hc_total_num_synchronous_data_packets;
}
__attribute__((packed)) HCI_READ_BUFFER_SIZE_COMPLETE_PACKET;


typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
    uint8_t local_name[HCI_MAX_LOCAL_NAME_SIZE];
}
__attribute__((packed)) HCI_READ_LOCAL_NAME_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_PIN_TYPE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_AUTH_READ_BD_ADDR_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_AUTHENTICATION_ENABLE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_ENCRYPTION_MODE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_SECURE_CONNECTION_HOST_SUPPORT_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_AUTH_CODE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_LOCAL_NAME_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_CLASS_OF_DEVICE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
}
__attribute__((packed)) HCI_WRITE_SCAN_ENABLE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t num_hci_command_packets;
    uint16_t op_code;
    uint8_t status;
    uint8_t simple_pairing_mode;
}
__attribute__((packed)) HCI_READ_SIMPLE_PAIRING_MODE_COMPLETE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
}
__attribute__((packed)) HCI_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
    uint8_t reserved;
    uint8_t service_type;
    uint32_t token_rate;
    uint32_t peak_bandwidth;
    uint32_t latency;
    uint32_t delay_variation;
}
__attribute__((packed)) HCI_QOS_SETUP_COMPLETE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
    uint8_t current_mode;
    uint16_t interval;
}
__attribute__((packed)) HCI_MODE_CHANGE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
    uint8_t encryption_enabled;
}
__attribute__((packed)) HCI_ENCRYPTION_CHANGE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
    uint8_t reason;
}
__attribute__((packed)) HCI_DISCONNECTION_COMPLETE_EVENT_PACKET;

// event packets
typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_LINK_KEY_REQUEST_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    bd_addr_t addr;
    uint8_t new_role;
}
__attribute__((packed)) HCI_ROLE_CHANGE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
    bd_addr_t addr;
    uint8_t link_type;
    uint8_t encryption_enabled;
}
__attribute__((packed)) HCI_CONNECTION_COMPLETE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint16_t con_handle;
    uint8_t lmp_max_slots;
}
__attribute__((packed)) HCI_EVENT_MAX_SLOTS_CHANGED_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t link_key[HCI_LINK_KEY_SIZE];
    uint8_t key_type;
}
__attribute__((packed)) HCI_LINK_KEY_NOTIFICATION_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    bd_addr_t addr;
    uint8_t class_of_device[3];
    uint8_t link_type;
}
__attribute__((packed)) HCI_CONNECTION_REQUEST_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    uint8_t status;
    uint16_t con_handle;
}
__attribute__((packed)) HCI_AUTHENTICATION_COMPLETE_EVENT_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t event_code;
    uint8_t params_size;
    bd_addr_t addr;
}
__attribute__((packed)) HCI_PIN_CODE_REQUEST_EVENT_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint8_t data[];
}
__attribute__((packed)) HCI_ACL_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;
    uint8_t data[];
} 
__attribute__((packed)) L2CAP_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;
    uint8_t pdu;
    uint16_t tx_id;
    uint16_t params_size;
    uint8_t data[];
} 
__attribute__((packed)) L2CAP_SDP_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;
    uint8_t payload[];
}
__attribute__((packed)) L2CAP_SIGNAL_CHANNEL_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t psm;
    uint16_t source_cid;
} 
__attribute__((packed)) L2CAP_CONNECTION_REQUEST_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t dest_cid;
    uint16_t source_cid;
    uint16_t result;
    uint16_t status;
}
__attribute__((packed)) L2CAP_CONNECTION_RESPONSE_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t dest_cid;
    uint16_t flags;

    uint8_t options[];
} 
__attribute__((packed)) L2CAP_CONFIG_REQUEST_PACKET;

typedef struct
{
    //uint16_t size;
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t source_cid;
    uint16_t flags;
    
    uint16_t result;

    uint8_t options[];
} 
__attribute__((packed)) L2CAP_CONFIG_RESPONSE_PACKET;

typedef struct
{
    uint8_t type;
    uint8_t size;
}
__attribute__((packed)) L2CAP_CONFIG_OPTION;

typedef struct
{
    uint8_t type;
    uint8_t size;
    uint16_t mtu;
}
__attribute__((packed)) L2CAP_CONFIG_MTU_OPTION;

typedef struct
{
    uint8_t type;
    uint8_t size;
    uint16_t flush_timeout;
}
__attribute__((packed)) L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION;


#define L2CAP_CONFIG_MTU_OPTION_TYPE              1
#define L2CAP_CONFIG_FLUSH_TIMEOUT_OPTION_TYPE    2

typedef struct
{
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t dest_cid;
    uint16_t source_cid;
}
__attribute__((packed)) L2CAP_DISCONNECTION_REQUEST_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t hci_acl_size;
    uint16_t l2cap_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;

    uint16_t dest_cid;
    uint16_t source_cid;
}
__attribute__((packed)) L2CAP_DISCONNECTION_RESPONSE_PACKET;

typedef struct
{
    uint8_t type;
    uint16_t con_handle : 12;
    uint16_t packet_boundary_flag : 2;
    uint16_t broadcast_flag : 2;
    uint16_t l2cap_size;
    uint16_t data_size;
    uint16_t channel;

    uint8_t code;
    uint8_t identifier;
    uint16_t payload_size;
    
    uint16_t reason;
    uint8_t data[];
}
__attribute__((packed)) L2CAP_COMMAND_REJECT_PACKET;

typedef struct
{
    uint8_t report_type;
    uint8_t report_id;
    uint8_t data[];
} 
__attribute__((packed)) HID_REPORT_PACKET;

//void reverse_bda(bd_addr_t bda);
//void read_bda(const uint8_t* p, bd_addr_t bda);
//void write_bda(uint8_t* p, const bd_addr_t bda);
uint16_t read_uint16(uint8_t* p);
uint16_t read_uint16_be(uint8_t* p);
uint32_t read_uint24(const uint8_t* p);
const char* bda_to_string(const bd_addr_t bda);
uint32_t uint24_bytes_to_uint32(const uint8_t* cod);

void write_uint16_be(uint8_t* p, uint16_t value);

#define PARAMS_SIZE(a) (sizeof(a) - sizeof(HCI_COMMAND_PACKET))

BT_PACKET_ENVELOPE* create_packet_envelope(uint16_t packet_size);
BT_PACKET_ENVELOPE* create_hci_cmd_packet(uint16_t op_code, uint8_t params_size);
BT_PACKET_ENVELOPE* create_hci_set_controller_to_host_flow_control_packet(uint8_t flow_control_enable);
BT_PACKET_ENVELOPE* create_hci_host_number_of_completed_packets_packet(uint8_t number_of_handles, uint16_t* connection_handles, uint16_t* host_num_of_completed_packets);
BT_PACKET_ENVELOPE* create_hci_host_buffer_size_packet(uint16_t host_acl_data_packet_length, uint8_t host_synchronous_data_packet_length, uint16_t host_total_num_acl_data_packets, uint16_t host_total_num_synchronous_data_packets);
BT_PACKET_ENVELOPE* create_hci_read_buffer_size_packet();
BT_PACKET_ENVELOPE* create_hci_reset_packet();
BT_PACKET_ENVELOPE* create_hci_inquiry_cancel_packet();
BT_PACKET_ENVELOPE* create_hci_inquiry_packet(uint32_t lap, uint8_t duration, uint8_t num_responses);
BT_PACKET_ENVELOPE* create_hci_remote_name_request_packet(const bd_addr_t addr, uint8_t psrm, bool clock_offset_valid, uint16_t clock_offset);
BT_PACKET_ENVELOPE* create_hci_create_connection_packet(const bd_addr_t addr, uint16_t packet_type, uint8_t psrm, bool clock_offset_valid, uint16_t clock_offset, uint8_t allow_role_switch);
BT_PACKET_ENVELOPE* create_hci_switch_role_packet(const bd_addr_t addr, uint8_t role);
BT_PACKET_ENVELOPE* create_hci_authentication_requested_packet(uint16_t con_handle);
BT_PACKET_ENVELOPE* create_hci_link_key_request_reply_packet(const bd_addr_t addr, const uint8_t* link_key);
BT_PACKET_ENVELOPE* create_hci_link_key_request_negative_packet(const bd_addr_t addr);
BT_PACKET_ENVELOPE* create_hci_write_pin_type_packet(uint8_t pin_type);
BT_PACKET_ENVELOPE* create_hci_pin_code_request_reply_packet(const bd_addr_t addr, const uint8_t* pin_code, uint8_t pin_code_size);
BT_PACKET_ENVELOPE* create_hci_pin_code_request_negative_reply_packet(const bd_addr_t addr);
BT_PACKET_ENVELOPE* create_hci_write_scan_enable_packet(uint8_t scan_enable);
BT_PACKET_ENVELOPE* create_hci_accept_connection_request_packet(bd_addr_t addr, uint8_t role);
BT_PACKET_ENVELOPE* create_hci_reject_connection_request_packet(bd_addr_t addr, uint8_t reason);
BT_PACKET_ENVELOPE* create_hci_disconnect_packet(uint16_t con_handle, uint8_t reason);
BT_PACKET_ENVELOPE* create_hci_write_class_of_device_packet(uint32_t class_of_device);
BT_PACKET_ENVELOPE* create_hci_write_authentication_enable(uint8_t enable);
BT_PACKET_ENVELOPE* create_hci_write_encryption_mode(uint8_t encryption_mode);
BT_PACKET_ENVELOPE* create_hci_set_connection_encryption(uint16_t con_handle, uint8_t encryption_enable);
BT_PACKET_ENVELOPE* create_hci_write_local_name(char* local_name);
BT_PACKET_ENVELOPE* create_hci_current_iac_lap_packet(uint32_t iac_lap);
BT_PACKET_ENVELOPE* create_hci_write_default_link_policy_settings_packet(uint16_t default_link_policy_settings);
BT_PACKET_ENVELOPE* create_hci_secure_connections_host_support_packet(uint16_t secure_connections_host_support);
BT_PACKET_ENVELOPE* create_hci_qos_setup_packet(uint16_t con_handle, uint8_t flags, uint8_t service_type, uint32_t token_rate, uint32_t peak_bandwidth, uint32_t latency, uint32_t delay_variation);

BT_PACKET_ENVELOPE* create_acl_packet(uint16_t con_handle, uint16_t channel, uint8_t packet_boundary_flag, uint8_t broadcast_flag, const uint8_t* data, uint16_t data_size);
BT_PACKET_ENVELOPE* create_l2cap_packet(uint16_t con_handle, uint16_t l2cap_size, uint16_t channel, const uint8_t* data, uint16_t data_size);
BT_PACKET_ENVELOPE* create_l2cap_connection_request_packet(uint16_t con_handle, uint16_t psm, uint16_t source_cid);
BT_PACKET_ENVELOPE* create_l2cap_connection_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t dest_cid, uint16_t source_cid, uint16_t result, uint16_t status);
BT_PACKET_ENVELOPE* create_l2cap_config_request_packet(uint16_t con_handle, uint16_t dest_cid, uint16_t flags, uint16_t options_size);
BT_PACKET_ENVELOPE* create_l2cap_config_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t source_cid, uint16_t flags, uint16_t options_size);
BT_PACKET_ENVELOPE* create_l2cap_disconnection_request_packet(uint16_t con_handle, uint16_t dest_cid, uint16_t source_cid);
BT_PACKET_ENVELOPE* create_l2cap_disconnection_response_packet(uint16_t con_handle, uint8_t identifier, uint16_t dest_cid, uint16_t source_cid);

BT_PACKET_ENVELOPE* create_output_report_packet(uint16_t con_handle, uint16_t channel, const uint8_t* report, uint16_t report_size);

#endif