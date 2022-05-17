/**
 * Compile: g++ -o run_test Run_test.cpp uartSteval.cpp AS5600.cpp -lwiringPi
 * To run file with default file names: ./run_test
 * To run with user defined file names: ./run_test -i in_name.csv -o out_name.csv
 */

#include <ctype.h>
#include <stdio.h>
#include <wiringPi.h>
#include <iostream>
#include "uartSteval.h"
#include "AS5600.h"
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include "tools.h"

#include <wiringPi.h>
#include <wiringSerial.h>

using namespace std;

void checkForErrors(UART u)
{
    Frame f, f2, f3;
    GetRegistry(STEVAL_REGISTERS::FLAGS, STEVAL_REGISTERS_LEN::GET, 1, u, &f);
    long fd;
    switch ((int)f.data)
    {
    case 2:
        fd = start_log(LOG_PATH);
        printf("[ERROR] Over voltage\n");
        stop_log(fd);
        return;
    case 32:
        fd = start_log(LOG_PATH);
        printf("[ERROR] SpeedFeedback\n");
        stop_log(fd);
        FaultAck(1, u, &f2);
        StartMotor(1, u, &f3);
        return;
    default:
        return;
    }
}

int main(int argc, char* argv[])
{
    UART uart;
    uart.baud = 38400;
    Frame f1,f2,f3;
	uart.handler = serialOpen("/dev/ttyACM0",uart.baud);
	checkForErrors(uart);
	FaultAck(1, uart, &f2);
    SetMotorRefSpeed(1500, 1, uart, &f3);
	StartMotor(1, uart, &f1);
	serialClose(uart.handler);
	
    return 0;
}