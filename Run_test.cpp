/**
 * Compile: g++ -o run_test Run_test.cpp src/uartSteval.cpp src/AS5600.cpp src/tools.cpp -lwiringPi -lpthread -I inc
 * To run file with default file names: ./run_test
 * To run with user defined file names: ./run_test -i in_name.csv -o out_name.csv
 */

#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#include <wiringPi.h>
#include <wiringSerial.h>


#include "uartSteval.h"
#include "AS5600.h"
#include "tools.h"

#include <future>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <thread>
#include <chrono>

using namespace std;

void checkForErrors(UART u)
{
    Frame errf;
    GetRegistry(STEVAL_REGISTERS::FLAGS, STEVAL_REGISTERS_LEN::GET, 1, u, &errf);
    long fd;
    switch ((int)errf.data)
    {
    case 2:
        printf("[ERROR] Over voltage\n");
        FaultAck(1, u, &errf);
        StopMotor(1,u,&errf);
        StartMotor(1, u, &errf);
        return;
    case 32:
        printf("[ERROR] SpeedFeedback\n");
        FaultAck(1, u, &errf);
        StopMotor(1,u,&errf);
        StartMotor(1, u, &errf);
        return;
    default:
        return;
    }
}


int main(int argc, char* argv[])
{
    //Init vars
    Frame f;
    int speed;

    clock_t start_time = 0;
    clock_t end_time = 0;

    chrono::milliseconds timeout(10);
    future<int> async_getchar = async(getchar);

    // encoder
    int as5600;
    AS5600_Init(&as5600);

    // set input and output file names
    string in_name, out_name;
    if (argc > 2){
        in_name = argv[2];
    }
    if (argc > 5){
        out_name = argv[4];
    }
    if (argc == 1){
        in_name = "MATLAB/speed_ref.csv";
        out_name = "ref_out.csv";
    }

    // open output & input file
    ofstream OutputFile;
    OutputFile.open(out_name);

    ifstream InputFile;
    InputFile.open(in_name);

    //read single line input into vector
    vector<int> input;
    string line, word;
    int i_word;

    getline(InputFile, line);
    stringstream strl(line);
    input.clear();
 
    while(getline(strl, word, ',')){
        i_word = stoi(word);
        input.push_back(i_word);
    }

    // connect
    UART uart;
    uart.baud = 38400;
    uart.handler = serialOpen("/dev/ttyACM0",uart.baud);

    checkForErrors(uart);
    StartMotor(1,uart, &f);
    //for every val in input vector
    for (int i = 0; i < input.size(); i++){
        try{
            checkForErrors(uart);
            speed = input[i];

            SetMotorRefSpeed(speed, 1, uart, &f);

            float get_angle = convertRawAngleToDegrees(getRawAngle());
            end_time = clock();
            string get_angle_s = to_string(get_angle);

            OutputFile << get_angle_s << ", ";

            cout << "Measured angle: " << get_angle << endl;
            cout << "Timestamp: " << (end_time - start_time) << endl;

            if(async_getchar.wait_for(timeout) == future_status::ready)
            {
                async_getchar.get();
                cout << "Exiting by user" << endl;
                break;
            }

            start_time = clock();

        }
        catch(const exception& ex){
            cout << "Error: " << ex.what() << endl;
        }
    }

    //this_thread::sleep_for(chrono::milliseconds(1000));
    SetMotorRefSpeed(0, 1, uart, &f);
    StopMotor(1,uart,&f);

    serialClose(uart.handler);

    OutputFile.close();

    cout << "Exiting by end" << endl;

    return 0;
}