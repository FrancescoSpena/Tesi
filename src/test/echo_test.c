#include "comm_uart.h"
#include "packet_handler.h"
#include <stdio.h>
#include "print_packets.h"
#include "beth_packets.h"
#include "packet_operations.h"
#include <string.h>
#include <util/delay.h>

/*Digitare Enter viene letto come un carattere*/

int main(void){
    struct Uart* uart = Uart_init(19200);

    while(1){
        if(Uart_available(uart) >= 1){
            uint8_t c = Uart_read(uart);
            Uart_write(uart,c);
        }
    }
}