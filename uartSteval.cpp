/**
 * @file uartSteval.cpp
 *
 *  Created on: Nov 27, 2020
 *      Author: Jakub Walkowski
 */

#include <ctype.h>
#include <stdio.h>
#include <wiringPi.h>
#include <iostream>
#include <unistd.h>
#include <wiringSerial.h>
#include "uartSteval.h"
#include <stdarg.h>

#define DEBUG 0 //0

#if DEBUG
void printf_debug(const char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsprintf(buffer,format, args);
  puts(buffer);
  va_end(args);
}
#else
void printf_debug(const char * format, ...)
{

}
#endif

UART_STATUS receive(Frame *cmd,int connection)
{
    int j=0;
    int* array = new int[10];
    printf_debug("Recive: ");
    while (serialDataAvail(connection))
    {
        array[j] = serialGetchar(connection);
        printf_debug("%02x ", array[j]);
        j++;
    }
    printf_debug("\n");
    if(array[0]!=0xff)
    {
        *cmd = Frame(array,j);
        printf_debug("Odebrana wartość: %d \n",cmd->data);
    }        
    else
        printf_debug("ERROR\n");
    return UART_OK;
}
UART_STATUS sendData(int con, int data,int l, bool cmd_show=true)
{
    for(int i = 0;i<l;i++){
      serialPutchar(con,(data >> i*8) & 0xFF);
      if(cmd_show)
        printf_debug("%02X ", (data >> i*8) & 0xFF); 
    }
    if(cmd_show)
        printf_debug("\n");
    return UART_OK;
}
UART_STATUS send(Frame cmd, UART uart,Frame* f, bool cmd_show)
{
    int connection = serialOpen("/dev/ttyACM0",uart.baud);
    if(connection==-1)
    {
        return UART_CONNECTION_ERROR;
    }

    serialPutchar(connection,(cmd.motorId<<5)+(int)cmd.frameCode);
    serialPutchar(connection,(int)cmd.payload);
    serialPutchar(connection,(int)cmd.reg);

    if (cmd_show)
    {
        printf_debug("Send: ");
        //std::cout << std::hex << (cmd.motorId<<5)+(int)cmd.frameCode<<" ";
        //std::cout << std::hex << (int)cmd.payload<<" ";
        //std::cout << std::hex << (int)cmd.reg<<" ";
    }
    
    if(cmd.data!=NO_DATA)
    {
        UART_STATUS resD = sendData(connection,cmd.data,(int)cmd.payload-1);
        if(resD != UART_OK)
            return resD;
    }

    serialPutchar(connection,cmd.CRC);
    
    if (cmd_show)
    {
        //std::cout << std::hex << cmd.CRC<<" ";
        printf_debug("\n");
    }

    Frame recCommand= Frame();
    //delay(3);
    receive(&recCommand,connection);
    *f = recCommand;
    serialClose(connection);
    return UART_OK;
}
UART_STATUS StartMotor(int motorId, UART uart,Frame* f)
{
    Frame  cmd = Frame(1,FRAME_CODES::EXE,STEVAL_REGISTERS::START_MOTOR,(int)STEVAL_REGISTERS_LEN::EXE_CMD, NO_DATA);
    return send(cmd, uart,f);
}
UART_STATUS StopMotor(int motorId, UART uart,Frame* f)
{
    Frame  cmd = Frame(1,FRAME_CODES::EXE,STEVAL_REGISTERS::STOP_MOTOR,(int)STEVAL_REGISTERS_LEN::EXE_CMD, NO_DATA);
    return send(cmd,uart,f);
}
UART_STATUS SetMotorRefSpeed(int ref, int motorId, UART uart,Frame* f)
{
    Frame  cmd = Frame(1,FRAME_CODES::SET,STEVAL_REGISTERS::RAMP_FIN_SPEED,(int)STEVAL_REGISTERS_LEN::RAMP_FIN_SPEED, ref);
    return send(cmd,uart,f);
}
UART_STATUS SetRegistry(STEVAL_REGISTERS reg, STEVAL_REGISTERS_LEN regL, int motorId, UART uart, int data,Frame* f)
{
    Frame cmd = Frame(1,FRAME_CODES::GET,reg,(int)regL,data);
    return send(cmd,uart,f);
}
UART_STATUS GetRegistry(STEVAL_REGISTERS reg, STEVAL_REGISTERS_LEN regL, int motorId, UART uart,Frame* f)
{
    Frame cmd = Frame(1,FRAME_CODES::GET,reg,(int)regL,NO_DATA);
    printf_debug("data:%d:%d",cmd.data, f->data);
    return send(cmd,uart,f);
}

UART_STATUS FaultAck(int motorId, UART uart, Frame *f)
{
    Frame cmd = Frame(1, FRAME_CODES::EXE, STEVAL_REGISTERS::FAULT_ACT, (int)STEVAL_REGISTERS_LEN::EXE_CMD, NO_DATA);
    return send(cmd, uart, f);
}