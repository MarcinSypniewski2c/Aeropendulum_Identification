/**
 * Compile: g++ -o run_test Run_test.cpp inc/uartSteval.cpp inc/AS5600.cpp -lwiringPi
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

using namespace std;

void checkForErrors(UART u)
{
    //delay(5000);
    Frame f, f2, f3;
    GetRegistry(STEVAL_REGISTERS::FLAGS, STEVAL_REGISTERS_LEN::GET, 1, u, &f);
    long fd;
    switch ((int)f.data)
    {
    case 2:
        fd = start_log(LOG_PATH);
        printf("[ERROR] Over voltage\n");
        FaultAck(1, u, &f2);
        StartMotor(1, u, &f3);
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
    string in_name, out_name;
    // set input and output file names
    if (argc > 2){
        in_name = argv[2];
    }
    if (argc > 5){
        out_name = argv[4];
    }
    if (argc == 1){
        in_name = "signals_aero_test.csv";
        out_name = "m2test.csv";
    }
    //Init
    UART uart;
    uart.baud = 38400;
    Frame f_start, f_set, f2, ff2, f_start2, f_set2;
    int num_iter = 5;
    int speed = 3000;

    int as5600;
    AS5600_Init(&as5600);

    ofstream OutputFile;
    OutputFile.open(out_name);

    ifstream InputFile;
    string line, word;
    InputFile.open(in_name);

    vector<int> input;
    int i_word;
    //read single line input into vector
    getline(InputFile, line);
    stringstream strl(line);
    input.clear();
 
        while(getline(strl, word, ',')){
            i_word = stoi(word);
            input.push_back(i_word);
        }

    for (int j= 0; j < 1; j++){
    StartMotor(1,uart, &f_start);
        //for every val in input vector
        for (int i = 0; i < input.size(); i++){
            try{
                speed = input[i];

                SetMotorRefSpeed(speed, 1, uart, &f_set);

                float get_angle = convertRawAngleToDegrees(getRawAngle());
                string get_angle_s = to_string(get_angle);

                OutputFile << get_angle_s << ", ";

                cout << "Measured angle: " << get_angle << endl;

                checkForErrors(uart);
            }
            //print exception
            catch(const exception& ex){
                cout << "Error: " << ex.what() << endl;
            }
        }
    OutputFile << ";" << endl;
    StopMotor(1,uart,&ff2);
    this_thread::sleep_for(chrono::milliseconds(3000));
    }

    OutputFile.close();

    return 0;
}