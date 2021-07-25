#ifndef BTDUMP_H
#define BTDUMP_H

#define DUMP_WIDTH 36

uint32_t read_uint24(const uint8_t* p);
uint16_t read_uint16(const uint8_t* p);
uint16_t read_uint16_be(const uint8_t* p);
const char* bda_to_string(const bd_addr_t bda);
uint32_t uint24_bytes_to_uint32(const uint8_t* cod);

void dump_packet(uint8_t io_direction, const uint8_t* packet, uint16_t size);
const char* get_hci_event_name(uint16_t event_code);
const char* get_hci_op_code_name(uint16_t op_code);

#endif