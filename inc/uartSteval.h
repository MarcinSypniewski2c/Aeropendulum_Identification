/**
  ******************************************************************************
  * @file    uartSteval.h
  * @author  Jakub Walkowski
  * @author  AW					adrian.wojcik@put.poznan.pl
  * @version 2.0
  * @date    13-May-2022
  * @brief   Serial port communication protocol for STEVAL-SPIN3021
  *
  ******************************************************************************
  */

#ifndef INC_UARTSTEVAL_H_
#define INC_UARTSTEVAL_H_

/* Includes ------------------------------------------------------------------*/
#include <climits>
#include <cstdio>

#include "registers.h"

/* Defines -------------------------------------------------------------------*/
#define NO_DATA INT_MAX

void printf_debug(const char * format, ...);

/* Typedef -------------------------------------------------------------------*/
/**
 * @brief UART connection object
 * 
 */
struct UART
{
    int baud;
    char *device;
	int handler;
};

/**
 * @brief UART frame object
 * 
 */
struct Frame
{
	int motorId;                    /*< Motor ID */
    FRAME_CODES frameCode;          /*< Frame code from (@see: FRAME_CODES) */
    STEVAL_REGISTERS reg;           /*< Motor control register id (@see: STEVAL_REGISTERS) */
    int payload;                    /*< Motor control register payload length */
    int data;                       /*< Data in frame */
    int crc;                        /*< Cyclic redundancy check */
	
    /**
     * @brief Default constructor
     */
    Frame(){}
	
    /**
     * @brief Parametric constructor: create frame with frame code, register ID and data.
     * 
     * @param[in] _motorId   : Motor ID
     * @param[in] _frameCode : Frame code (@see: FRAME_CODES)
     * @param[in] _reg       : Motor control register ID
     * @param[in] _payload   : Motor control register payload length
     * @param[in] _data      : Data to send, if no data put NO_DATA
     */
    Frame(int _motorId, FRAME_CODES _frameCode, STEVAL_REGISTERS _reg, int _payload, int _data)
    {
        motorId = _motorId;
        frameCode = _frameCode;
        reg = _reg;
        payload = _payload;
        data = _data;
		
		/** Encode frame 
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/Src/motor_control_protocol.c lines 162-608
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/MCSDK_v5.4.8/MotorControl/MCSDK/UILibrary/Src/frame_communication_protocol.c lines 54-123
		  */
        if (data == NO_DATA)
            crc = ((motorId << 5) + (int)frameCode) + (int)reg + payload;
        else
        {
            crc = ((motorId << 5) + (int)frameCode) + (int)reg + payload;
            for (int i = 0; i < payload; i++)
            {
                crc += (data >> i * 8) & 0xFF;
                printf_debug("crc: %d \n", (data >> i * 8) & 0xFF);
            }
        }
        crc = ((crc>>8) & 0xff)+(crc & 0xff);
        printf_debug("crc: %d \n", crc);
    }
	
	/**
     * @brief Parametric constructor: create frame from raw data
     * 
     * @param[in] array    : frame content
     * @param[in] lenght   : frame length
     */
    Frame(int* array, int lenght)
    {
		/** Decode frame 
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/Src/motor_control_protocol.c lines 162-608
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/MCSDK_v5.4.8/MotorControl/MCSDK/UILibrary/Src/frame_communication_protocol.c lines 54-123
		  */
        motorId = array[0]>>5;
        frameCode = (FRAME_CODES)(array[0]&0x1F);
        payload = array[1];
        crc = array[lenght-1];

        if(lenght>3)
        {
            for(int i = 2+payload; i >=2; i--)
            {
                data = (data<<(8*(payload-1-i)))+array[i];
            }
        }
    }

    /**
     * @brief Get the Command object as int 
     * 
     * @return int Command as single int
     */
    int getCommad()
    {
        /** Encode frame 
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/Src/motor_control_protocol.c lines 162-608
		  * @see [STEVAL-SPIN3021 PROCJECT DIR]/MCSDK_v5.4.8/MotorControl/MCSDK/UILibrary/Src/frame_communication_protocol.c lines 54-123
		  */
        return ((((motorId << 5) + (int)frameCode << 8) + (int)reg << 8) + payload << 8) + crc;
		
		
    }
	
    /**
     * @brief Bitwise inversion of a number
     * 
     * @param[in] b : int number to reverse
     * @param[in] l : Bitwise length of number
     * @return int inverts number
     */
    int reverse(int b, int l)
    {
        int newb = 0;
        for (int i = 0; i < l; i++)
        {
            if (i == l - 1)
                newb = (newb + ((b >> i * 8) & 0xFF));
            else
                newb = (newb + ((b >> i * 8) & 0xFF)) << 8;
        }
        return newb;
    }
};

/**
 * @brief Receive UART frame.
 * 
 * @param[out] cmd        : Received frame
 * @param[in] connection : UART connection id
 * @return UART_STATUS Receiving status
 */
UART_STATUS receive(Frame *cmd, int connection);

/**
 * @brief Send data in frame.
 * 
 * @param[in] con      : UART connection id
 * @param[in] data     : Data to send
 * @param[in] l        : Control register payload length
 * @param[in] cmd_show : Optional parameter, if true show sending fame in console
 * @return UART_STATUS Sending result
 */
 UART_STATUS send(Frame cmd, UART uart, Frame* f, bool cmd_show);
 
/**
 * @brief Execute start motor command.
 * 
 * @param[in] motorId : Motor id
 * @param[in] uart    : UART object
 * @return UART_STATUS Starting motor result
 */
UART_STATUS StartMotor(int motorId, UART uart, Frame* f);
 
/**
 * @brief Execute stop motor command.
 * 
 * @param[in] motorId : Motor id
 * @param[in] uart    : UART object
 * @return UART_STATUS  Stopping motor result
 */
UART_STATUS StopMotor(int motorId, UART uart, Frame* f);

/**
 * @brief Set the motor ramp final speed registry.
 * 
 * @param[in] ref     : Value to set
 * @param[in] motorId : Motor id
 * @param[in] uart    : UART object
 * @return UART_STATUS Setting result 
 */
UART_STATUS SetMotorRefSpeed(int ref, int motorId, UART uart, Frame* f);

/**
 * @brief Set the registry value.
 * 
 * @param[in] reg     : Motor control register id
 * @param[in] regL    : Motor register payload length
 * @param[in] motorId : Motor id
 * @param[in] uart    : UART object
 * @param[in] data    : Value to set
 * @return UART_STATUS Setting result
 */
UART_STATUS SetRegistry(STEVAL_REGISTERS reg, STEVAL_REGISTERS_LEN regL, int motorId, UART uart, int data, Frame* f);

/**
 * @brief Get the registry value.
 * 
 * @param[in] reg     : Motor control register id
 * @param[in] regL    : Motor register payload length
 * @param[in] motorId : Motor id
 * @param[in] uart    : UART object
 * @return UART_STATUS Getting result
 */
UART_STATUS GetRegistry(STEVAL_REGISTERS reg, STEVAL_REGISTERS_LEN regL, int motorId, UART uart, Frame* f);

/**
 * @brief [TODO]
 * 
 * @param[in] 
 * @param[in] 
 * @param[in]
 * @param[in]
 * @return
 */
UART_STATUS FaultAck(int motorId, UART uart, Frame *f);

#endif /* INC_UARTSTEVAL_H_ */