/**
  **************************************************************************************************************************************
  * @file    run_timer.c
  * @author  AW		adrian.wojcik@put.poznan.pl
  * @version 1.0
  * @date    19-May-2022
  * @brief   Precise sample time in C++20 CLI app for Raspberry Pi
  *
  **************************************************************************************************************************************
  */

/* Includes ---------------------------------------------------------------------------------------------------------------------------*/
#include <chrono>
#include <stdio.h>
#include "inc/mc_type.h"

using namespace std::chrono;

/* Main function ----------------------------------------------------------------------------------------------------------------------*/

void print_error_name(int error_code)
{
	if((error_code & MC_FOC_DURATION) != 0)
		printf("MC_FOC_DURATION");
	if((error_code & MC_OVER_VOLT) != 0)
		printf("MC_OVER_VOLT");
	if((error_code & MC_UNDER_VOLT) != 0)
		printf("MC_UNDER_VOLT");
	if((error_code & MC_OVER_TEMP) != 0)
		printf("MC_OVER_TEMP");
	if((error_code & MC_START_UP) != 0)
		printf("MC_START_UP");
	if((error_code & MC_SPEED_FDBK) != 0)
		printf("MC_SPEED_FDBK");
	if((error_code & MC_BREAK_IN) != 0)
		printf("MC_BREAK_IN");
	if((error_code & MC_SW_ERROR) != 0)
		printf("MC_SW_ERROR");
}

/**
  * @brief  The application entry point.
  * @retval 0 
  */
int main(void)
{
	// TIMESTAMP SETUP
	long long int iter_period_us = 500'000;
	auto program_start = high_resolution_clock::now();
	
	print_error_name(MC_FOC_DURATION | MC_OVER_VOLT);
	
	for(int i = 0; i < 5; i++)
	{
		// TIMESTAMP SETUP
		auto iter_start = high_resolution_clock::now();
		long long int timestamp_us = duration_cast<microseconds>(iter_start - program_start).count();
		
		// USER CODE:
		double u = 0.0;
		for(long long int j = 0; j < 100000; j++)
			u = u*u + 0.2*u;
		printf("TIMESTAMP: %lld\n", timestamp_us);
		
		/* ACTIVE WAITING BEGINS ******************************************************************************************************/
		long long int iter_elapsed_us = 0;
		do { 
			auto iter_elapsed = high_resolution_clock::now();	
			iter_elapsed_us = duration_cast<microseconds>(iter_elapsed - iter_start).count();
		} while(iter_elapsed_us < iter_period_us);
		/* ACTIVE WAITING ENDS ********************************************************************************************************/
	}
	
	return 0;
}