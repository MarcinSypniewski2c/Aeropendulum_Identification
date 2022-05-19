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

/* Main function ----------------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval 0 
  */
int main(void)
{
	// TIMESTAMP SETUP
	long long int iter_period_us = 500'000;
	auto program_start = std::chrono::high_resolution_clock::now();
	
	for(int i = 0; i < 5; i++)
	{
		// TIMESTAMP SETUP
		auto iter_start = std::chrono::high_resolution_clock::now();
		long long int timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(iter_start - program_start).count();
		
		// USER CODE:
		double u = 0.0;
		for(long long int j = 0; j < 100000; j++)
			u = u*u + 0.2*u;
		printf("TIMESTAMP: %lld\n", timestamp_us);
		
		/* ACTIVE WAITING BEGINS ******************************************************************************************************/
		long long int iter_elapsed_us = 0;
		do { 
			auto iter_elapsed = std::chrono::high_resolution_clock::now();	
			iter_elapsed_us =  std::chrono::duration_cast<std::chrono::microseconds>(iter_elapsed - iter_start).count();
		} while(iter_elapsed_us < iter_period_us);
		/* ACTIVE WAITING ENDS ********************************************************************************************************/
	}
	
	return 0;
}