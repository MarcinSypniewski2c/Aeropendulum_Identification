default: app

app:  Run_test.cpp src/uartSteval.cpp src/AS5600.cpp src/tools.cpp
	g++ -o run_test Run_test.cpp src/uartSteval.cpp src/AS5600.cpp src/tools.cpp -lwiringPi -lpthread -I inc
	
timer: run_timer.cpp
	g++ -o run_timer run_timer.cpp -Wall -pedantic -std=c++17
	
clean: 
	rm -f motor

