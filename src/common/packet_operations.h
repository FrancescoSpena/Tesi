#pragma once
#include <stdint.h>
#include "beth_packets.h"

typedef void (*receiveFn_t)(PacketHeader*,void*);

typedef struct PacketOperation{
    uint8_t id;
    uint8_t size; 
    receiveFn_t on_receive_fn;
    void* args;
}PacketOperation;