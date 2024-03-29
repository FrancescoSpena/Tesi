#include "../arch/include/comm_uart.h"
#include "../common/packet_handler.h"
#include <stdio.h>
#include "../common/print_packets.h"
#include "../common/beth_packets.h"
#include "../common/packet_operations.h"
#include <string.h>
#include <util/delay.h>
#include "../arch/include/uart.h"
#include "../common/beth_comm.h"
#include <stdlib.h>
#include "../beth_firmware/beth_globals.h"


PacketHandler handler;

void echoRoutine(struct Uart* uart){
    uint8_t c;
    while(1){
        if(Uart_available(uart)){
            c = Uart_read(uart);
            Uart_write(uart,c);
        }
    } 
}

void statusRoutine(struct Uart* uart){
    PacketStatus status = UnknownType;
    uint8_t c;
    while(1){
        if(Uart_available(uart)){
            while(status != ChecksumSuccess){
                c = Uart_read(uart);
                status = PacketHandler_readByte(&handler,c);
                Uart_write(uart,c);
            }
            status = UnknownType;
        }
        _delay_ms(500);
    }
}

//TODO function to send packet to arduino

int main(void){
    printf_init();
    struct Uart* uart = Uart_init(19200);
    PacketHandler_init(&handler);
    PacketHandler_addOperation(&handler,&diff_drive_control_op);
    while(1){
        statusRoutine(uart);
        BethComm_sendPacket(&handler,&drive_control.h,uart);
        _delay_ms(1000);
        BethComm_sendPacket(&handler,&drive_status.h,uart);
        _delay_ms(1000);
        BethComm_sendPacket(&handler,&drive_params.h,uart);
        _delay_ms(1000);
    }
}