#ifndef WII_BT_H
#define WII_BT_H


void dump_packet(const char* prefix, uint8_t* packet, uint16_t size);
int wii_bt_packet_handler(uint8_t* packet, uint16_t size, bool handled);
void post_bt_packet(void* packet);


#endif