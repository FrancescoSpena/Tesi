#pragma once 
#include "../src/common/beth_packets.h"
#include "../src/common/packet_status.h"
#include "../src/common/packet_handler.h"

typedef struct{
    PacketHandler ph;  //packet handler
    int serial_fd; //serial fd
}BethHost;

void BethHost_init(BethHost*, char* dev, int speed);
PacketStatus BethHost_sendPacket(BethHost*, PacketHeader*);