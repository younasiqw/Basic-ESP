// R6ExternalSource.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Globals.h"
#include "Utility.h"
#include "Memory.h"
#include "Overlay.h"
#include <iostream>

//Defining our globals
namespace Global {
	HANDLE GameHandle = 0x0;

	LPVOID BaseAddress = 0;

	std::string LocalName = "MPGH";

	Mem Memory = Mem();

	Overlay Over = Overlay();
}

int main()
{

	//You will need to set the actual handle to the game here from your bypass
	if (Global::GameHandle == INVALID_HANDLE_VALUE || Global::GameHandle == NULL || Global::GameHandle == (HANDLE)0x0) {
		std::cout << "Invalid handle to R6" << std::endl;
		system("pause");
		return 1;
	}

	//Get/Set the base address
	Global::Memory.SetBaseAddress();
	std::cout << "Base address: " << std::hex << Global::Memory.GetBaseAddress() << std::dec << std::endl;

	//Set up the overlay window
	Global::Over.SetupWindow();
	std::cout << "Overlay window set" << std::endl;

	std::cout << "Local name: " << Global::LocalName << std::endl;

	//Start the main loop
	Global::Over.Loop();

	system("pause");
	return EXIT_SUCCESS;

    return 0;
}

