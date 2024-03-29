/**
 * packet_handler.c
*/

#include "packet_handler.h"
#include <stdio.h>
#include "buffer.h"

PacketStatus _rxAA(PacketHandler* h, uint8_t c);
PacketStatus _rx55(PacketHandler* h, uint8_t c);
PacketStatus _rxId(PacketHandler* h, uint8_t c);
PacketStatus _rxSize(PacketHandler* h, uint8_t c);
PacketStatus _rxPayload(PacketHandler* h, uint8_t c);
PacketStatus _rxCs(PacketHandler* h, uint8_t c); // checksum

void PacketHandler_init(PacketHandler* h){
    h->tx_start=0;
    h->tx_end=0;
    h->tx_size=0;
    h->receive_fn=_rxAA;
    return;
}

void PacketHandler_addOperation(PacketHandler* h,PacketOperation* o){
    h->packet_ops[o->id]=*o;
    h->size_op++;
}

//compute checksum
uint8_t computeCS(PacketHeader* _p){
    uint8_t checksum = 0; 
    uint8_t* p = (uint8_t*)_p;
    for(int i = 0; i < _p->size; ++i){
        checksum^=p[i];
    }
    return checksum;
}

PacketStatus PacketHandler_sendPacket(PacketHandler* h, PacketHeader* _p){
    uint8_t cs = computeCS(_p);
    _p->checksum = cs;

    uint8_t* p = (uint8_t*)_p;
    uint8_t* p_end = p + _p->size;

    h->tx_end = h->tx_buffer; //reset buffer 
    h->tx_start = h->tx_buffer;
    //insert 0xAA-0x55 before the packet
    *h->tx_end=0xAA;
    h->tx_end++;
    *h->tx_end=0x55;
    h->tx_end++;

    h->tx_size=2;
    //copy the packet inside the tx_buffer
    while(p < p_end){
        *h->tx_end = *p;
        ++h->tx_end;
        ++h->tx_size;
        ++p;
    }
    return Success;
}

PacketStatus PacketHandler_readByte(PacketHandler* h,uint8_t c){
    return (*h->receive_fn)(h,c);
}

uint8_t PacketHandler_txSize(PacketHandler* h){
    return h->tx_size;
}

uint8_t PacketHandler_popByte(PacketHandler* h){
    if(h->tx_size == 0)
        return 0;
    uint8_t c = *(h->tx_start);
    ++(h->tx_start);
    --(h->tx_size);
    return c;
}

PacketStatus _rxAA(PacketHandler* h, uint8_t c){
    if(c == 0xAA){
        h->receive_fn=_rx55;
        return Success;
    }
    return UnknownType;
}

PacketStatus _rx55(PacketHandler* h, uint8_t c){
    if(c==0x55){
        h->receive_fn=_rxId;
        return Success;
    }
    h->receive_fn=_rxAA;
    return UnknownType;
}

PacketStatus _rxId(PacketHandler* h, uint8_t c){
    if(c >= MAX_PACKET_TYPE){
        h->receive_fn=_rxAA;
        return UnknownType;
    }
    if(h->size_op != 0)h->current_op = &h->packet_ops[c];
    h->id_packet=c;
    h->receive_fn=_rxSize;
    return Success;
}

PacketStatus _rxSize(PacketHandler* h, uint8_t c){
    if(h->size_op != 0 && h->current_op->size!=c) {
        h->receive_fn=_rxAA;
        h->current_op=0;
        return WrongSize;
    }
    //inserting id and size 
    h->rx_end = h->rx_buffer+c;
    h->rx_start = h->rx_buffer;
    h->rx_buffer[0]=h->id_packet;
    h->rx_buffer[1] = c;
    h->rx_start += 2; 
    //computing checksum 
    h->rx_checksum = h->rx_buffer[0];
    h->rx_checksum^=c;
    h->receive_fn=_rxPayload;
    return Success;
}

PacketStatus _rxPayload(PacketHandler* h, uint8_t c){
    h->rx_checksum^= c; 
    (*h->rx_start) = c; 
    h->rx_start++;
    if(h->rx_end == h->rx_start){
        h->rx_start=0;
        h->rx_end=0;
        h->receive_fn=_rxCs;
        return (*h->receive_fn)(h,h->rx_checksum);
    }
    return Success;
}

PacketStatus _rxCs(PacketHandler* h, uint8_t c){
    h->current_packet=(PacketHeader*)h->rx_buffer;
    uint8_t recv_cs=h->rx_checksum; //received checksum 
    recv_cs^=h->current_packet->checksum;
    h->current_packet->checksum = 0;
    uint8_t computed_cs = computeCS(h->current_packet);
    h->current_packet->checksum = recv_cs;

    if(recv_cs != computed_cs){
        h->receive_fn=_rxAA;
        return ChecksumError;
    }
    if(h->size_op != 0){
        receiveFn_t recvFn=h->current_op->on_receive_fn;
        (*recvFn)(h->current_packet, h->current_op->args);
    }
    h->receive_fn=_rxAA;
    return ChecksumSuccess;
}
