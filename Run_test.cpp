/** 19.05.2022
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
#include "mc_type.h"

#include <future>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <thread>
#include <chrono>

using namespace std;

void print_error_name(int error_code, long long int time_error)
{
    error_code = error_code >> 0;
	if((error_code & MC_FOC_DURATION) != 0)
		printf("TIMESTAMP: %d, MC_FOC_DURATION\n", time_error);
	if((error_code & MC_OVER_VOLT) != 0)
		printf("TIMESTAMP: %lld, MC_OVER_VOLT\n", time_error);
	if((error_code & MC_UNDER_VOLT) != 0)
		printf("TIMESTAMP: %lld, MC_UNDER_VOLT\n", time_error);
	if((error_code & MC_OVER_TEMP) != 0)
		printf("TIMESTAMP: %lld, MC_OVER_TEMP\n", time_error);
	if((error_code & MC_START_UP) != 0)
		printf("TIMESTAMP: %lld, MC_START_UP\n", time_error);
	if((error_code & MC_SPEED_FDBK) != 0)
		printf("TIMESTAMP: %lld, MC_SPEED_FDBK\n", time_error);
	if((error_code & MC_BREAK_IN) != 0)
		printf("TIMESTAMP: %lld, MC_BREAK_IN\n", time_error);
	if((error_code & MC_SW_ERROR) != 0)
		printf("TIMESTAMP: %lld, MC_SW_ERROR\n", time_error);
}

void checkForErrors(UART u, long long int time_error)
{
    Frame errf;
    GetRegistry(STEVAL_REGISTERS::FLAGS, STEVAL_REGISTERS_LEN::GET, 1, u, &errf);
    long fd;
    if ((int)errf.data != 0){

        //print_error_name((int)errf.data, time_error);
    }
    /*
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
    }*/
}

void ramp_stop(UART u, int curr_speed)
{
    int temp_speed = curr_speed;
    Frame fstp;

    if (temp_speed > 0){

        while(temp_speed > 0){
            this_thread::sleep_for(chrono::milliseconds(250));
            SetMotorRefSpeed(temp_speed, 1, u, &fstp);
            temp_speed = temp_speed - 125;
            if (temp_speed < 0){
                temp_speed = 0;
            }
        }
    }
    else{
        while(temp_speed < 0){
            this_thread::sleep_for(chrono::milliseconds(250));
            SetMotorRefSpeed(temp_speed, 1, u, &fstp);
            temp_speed = temp_speed + 125;
            if (temp_speed > 0){
                temp_speed = 0;
            }
        }
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    StopMotor(1,u,&fstp);
    this_thread::sleep_for(chrono::milliseconds(100));
}


int main(int argc, char* argv[])
{
    //Init vars
    Frame f;
    int speed;
    int last_speed;

    chrono::milliseconds timeout(10);
    //future<int> async_getchar = async(getchar);

    // encoder
    int as5600;
    AS5600_Init(&as5600);

    // set input and output file names
    string in_name, out_name, time_name;
    if (argc > 2){
        in_name = argv[2];
    }
    if (argc > 5){
        out_name = argv[4];
    }
    if (argc == 1){
        in_name = "MATLAB/speed_ref.csv";
        out_name = "ref_out.csv";
        time_name = "time_out.csv";
    }

    // open output & input file
    ofstream OutputFile;
    OutputFile.open(out_name);

    ofstream TimeFile;
    TimeFile.open(time_name);

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
    cout << input.size() << endl;
    // connect
    UART uart;
    uart.baud = 38400;
    uart.handler = serialOpen("/dev/ttyACM0",uart.baud);

    // TIMESTAMP SETUP
	long long int iter_period_us = 50'000;
	auto program_start = std::chrono::high_resolution_clock::now();
    
    FaultAck(1, uart, &f);
    checkForErrors(uart, 0);
    
    StartMotor(1,uart, &f);

    //for every val in input vector
    for (int i = 0; i < input.size(); i++){
        try{
            // TIMESTAMP SETUP
            auto iter_start = std::chrono::high_resolution_clock::now();
            long long int timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(iter_start - program_start).count();

            FaultAck(1, uart, &f);
            StartMotor(1,uart, &f);
            checkForErrors(uart, timestamp_us);
            speed = input[i];
            
            SetMotorRefSpeed(speed, 1, uart, &f);

            float get_angle = convertRawAngleToDegrees(getRawAngle());
            string get_angle_s = to_string(get_angle);

            OutputFile << get_angle_s << ", ";
            TimeFile << timestamp_us << ", ";

            //cout << "Measured angle: " << get_angle << endl;
            //printf("[%d/%d] TIMESTAMP: %lld\n", i, input.size(), timestamp_us);

/*
            if(async_getchar.wait_for(timeout) == future_status::ready)
            {
                async_getchar.get();
                cout << "Exiting by user" << endl;
                last_speed = speed;
                break;
            }
*/
            /* ACTIVE WAITING BEGINS ******************************************************************************************************/
            long long int iter_elapsed_us = 0;
            do { 
                auto iter_elapsed = std::chrono::high_resolution_clock::now();	
                iter_elapsed_us =  std::chrono::duration_cast<std::chrono::microseconds>(iter_elapsed - iter_start).count();
            } while(iter_elapsed_us < iter_period_us);
            /* ACTIVE WAITING ENDS ********************************************************************************************************/

        }
        catch(const exception& ex){
            cout << "Error: " << ex.what() << endl;
        }
    }
    // stop motor, close connection, close file
    //FaultAck(1, uart, &f);
    StopMotor(1, uart, &f);
    delay(100);
    StopMotor(1, uart, &f);
    delay(100);
    StopMotor(1, uart, &f);
    delay(100);
    serialClose(uart.handler);
    OutputFile.close();
    TimeFile.close();
    printf("end of loop\n");
    return 0;
}