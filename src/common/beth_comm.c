#include "beth_comm.h"
#include "../arch/include/comm_uart.h"
#include "packet_handler.h"
#include <string.h>
#include <util/delay.h>
#include <stdio.h>
#include "../beth_firmware/beth_globals.h"

PacketHandler handler;
struct Uart* uart;

void BethComm_init(void){
    PacketHandler_init(&handler);
    PacketHandler_addOperation(&handler,&diff_drive_control_op);
    uart = Uart_init(19200);
    return;
}

typedef void (*ReceiveFn)(PacketHeader*h);

typedef struct BethComm{
    ReceiveFn receive_fn;
}BethComm;

void MotorControlPacket_receiveFn(PacketHeader* h){
    uint8_t add = h->dest_addr;
    MotorControlPacket* p = (MotorControlPacket*)h;
    //Motor 1
    if(add == 0){
        motor1_control.h = p->h;
        motor1_control.mode = p->mode;
        motor1_control.speed = p->speed;
    }else{
        //Motor 2 
        motor2_control.h = p->h;
        motor2_control.mode = p->mode;
        motor2_control.speed = p->speed;
    }
    return;
}

//dest_addr = 0 --> motor 1 
//dest_addr = 1 --> motor 2
void MotorStatusPacket_receiveFn(PacketHeader* h){
    uint8_t add = h->dest_addr;
    MotorStatusPacket* p = (MotorStatusPacket*)h;
    //Motor 1
    if(add == 0){
        motor1_status.desired_speed = p->desired_speed;
        motor1_status.encoder_ticks = p->encoder_ticks;
        motor1_status.h = p->h;
        motor1_status.measured_speed = p->measured_speed;
    }else{
        //Motor 2
        motor2_status.desired_speed = p->desired_speed;
        motor2_status.encoder_ticks = p->encoder_ticks;
        motor2_status.h = p->h;
        motor2_status.measured_speed = p->measured_speed;
    }
    return;
}

void MotorParamsPacket_receiveFn(PacketHeader* h){
    uint8_t add = h->dest_addr;
    MotorParamsPacket* p = (MotorParamsPacket*)h;
    //Motor 1
    if(add == 0){
        motor1_params.dir_a_pin=p->dir_a_pin;
        motor1_params.dir_b_pin=p->dir_b_pin;
        motor1_params.dt=p->dt;
        motor1_params.h=p->h;
        motor1_params.idt=p->idt;
        motor1_params.kd=p->kd;
        motor1_params.ki=p->ki;
        motor1_params.kp=p->kp;
        motor1_params.max_i=p->max_i;
        motor1_params.max_out=p->max_out;
        motor1_params.pwm_pin=p->pwm_pin;
        motor1_params.sum_i=p->sum_i;
    }else{
        //Motor 2
        motor2_params.dir_a_pin=p->dir_a_pin;
        motor2_params.dir_b_pin=p->dir_b_pin;
        motor2_params.dt=p->dt;
        motor2_params.h=p->h;
        motor2_params.idt=p->idt;
        motor2_params.kd=p->kd;
        motor2_params.ki=p->ki;
        motor2_params.kp=p->kp;
        motor2_params.max_i=p->max_i;
        motor2_params.max_out=p->max_out;
        motor2_params.pwm_pin=p->pwm_pin;
        motor2_params.sum_i=p->sum_i;
    }
    return;
}

void SystemStatusPacket_receiveFn(PacketHeader* h){
    SystemStatusPacket* p = (SystemStatusPacket*)h;
    system_status.h=p->h;
    system_status.idle_cycles=p->idle_cycles;
    system_status.rx_errors=p->rx_errors;
    system_status.rx_packets=p->rx_packets;
    system_status.tx_packets=p->tx_packets;
    return;
}

void DifferentialDriveStatusPacket_receiveFn(PacketHeader* h){
    DifferentialDriveStatusPacket* p = (DifferentialDriveStatusPacket*)h;
    drive_status.h=p->h;
    drive_status.odom_theta=p->odom_theta;
    drive_status.odom_x=p->odom_x;
    drive_status.odom_y=p->odom_y;
    return;
}

void DifferentialDriveParamPacket_receiveFn(PacketHeader* h){
    DifferentialDriveParamPacket* p = (DifferentialDriveParamPacket*)h;
    drive_params.distance=p->distance;
    drive_params.h=p->h;
    drive_params.radius_wheel=p->radius_wheel;
    return;
}

void DifferentialDriveControlPacket_receiveFn(PacketHeader* h){
    DifferentialDriveControlPacket* p = (DifferentialDriveControlPacket*)h;
    drive_control.h=p->h;
    drive_control.rotational_velocity=p->rotational_velocity;
    drive_control.translational_velocity=p->translational_velocity;
    return;
}


static BethComm received_packet_ops[MAX_PACKET_TYPE] = {
    {   //0 motor_control
        .receive_fn=MotorControlPacket_receiveFn,
    },
    {  //1 motor_status
        .receive_fn=MotorStatusPacket_receiveFn,
    },
    {  //2 motor_params
        .receive_fn=MotorParamsPacket_receiveFn,
    },
    {  //3 system status
        .receive_fn=SystemStatusPacket_receiveFn,
    },
    { //4 differential status 
        .receive_fn=DifferentialDriveStatusPacket_receiveFn,
    },
    { //5 differential params
        .receive_fn=DifferentialDriveParamPacket_receiveFn,
    },
    { //6 differential control
        .receive_fn=DifferentialDriveControlPacket_receiveFn,
    }
};

PacketStatus BethComm_sendPacket_internal(PacketHandler* handler, PacketHeader* _h, struct Uart* uart){
    PacketStatus ret = PacketHandler_sendPacket(handler,_h);
    uint8_t bytes = PacketHandler_txSize(handler);
    uint8_t c;
    for(int i=0; i < bytes; i++){
        c = PacketHandler_popByte(handler);
        Uart_write(uart,c);
    }
    return ret;
}

PacketStatus BethComm_sendPacket(PacketHeader* _h){
    return BethComm_sendPacket_internal(&handler,_h,uart);
}

void BethComm_receiveFn(PacketHeader* _h, void* _null){
    ReceiveFn fn = received_packet_ops[_h->id].receive_fn;
    (*fn)(_h);
    return;
}

PacketStatus BethComm_handle(void){
    while(Uart_available(uart)){
        uint8_t c = Uart_read(uart);
        if(PacketHandler_readByte(&handler,c) < 0){
            system_status.rx_errors++;
        }
        system_status.rx_packets++;
    }
    return Success;
}