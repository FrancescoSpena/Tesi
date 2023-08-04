#include "comm_uart.h"
#include "packet_handler.h"
#include <stdio.h>
#include "print_packets.h"
#include "beth_packets.h"
#include "packet_operations.h"
#include <string.h>
#include <util/delay.h>
#include "uart.h"
#include "beth_comm.h"
#include <stdlib.h>
#include "print_packets.h"
#include "beth_globals.h"


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
                //Uart_write(uart,c);
            }
            _delay_ms(500);
            status = UnknownType;
            printf("ds:%d\t ms:%d\n",
                    motor1_status.desired_speed,motor1_status.measured_speed);
        }
        _delay_ms(500);
    }
}

int main(void){
    printf_init();
    struct Uart* uart = Uart_init(19200);
    PacketHandler_init(&handler);
    PacketHandler_addOperation(&handler,&motor_status_op);


    statusRoutine(uart);
}