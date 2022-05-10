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

using namespace std;

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
        in_name = "mtest.csv";
        out_name = "m2test.csv";
    }
    //Init
    UART uart;
    uart.baud = 38400;
    Frame f_start, f_set, f2, ff2, f_start2, f_set2;
    int16_t num_iter = 5;
    uint16_t speed = 3000;

    int as5600;
    AS5600_Init(&as5600);

    ofstream OutputFile;
    OutputFile.open(out_name);

    ifstream InputFile;
    string line, word;
    InputFile.open(in_name);

    vector<vector<int>> all_ins;
    vector<int> input;
    int i_word;
    //read input into vector
    while(getline(InputFile, line)){
			stringstream str(line);
            input.clear();
 
			while(getline(str, word, ',')){
                i_word = stoi(word);
                input.push_back(i_word);
            }
            all_ins.push_back(input);				
		}
    for (int j= 0; j < all_ins.size(); j++){
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
            }
            //print exception
            catch(const exception& ex){

                cout << "Error: " << ex.what() << endl;

                //FaultAck(1,uart,&f2);
                //StopMotor(1,uart,&ff2);

                //StartMotor(1,uart, &f_start2);
                //SetMotorRefSpeed(speed, 1, uart, &f_set2);

                //cout << "RESET" << endl;
            }
        }
    OutputFile << ";" << endl;
    StopMotor(1,uart,&ff2);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

    OutputFile.close();

    return 0;
}