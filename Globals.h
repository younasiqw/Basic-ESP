#pragma once

#include <Windows.h>
#include <string>
#include "Memory.h"
#include "Overlay.h"

//Global variables to be used throughout the project
namespace Global {

	//Handle to the game
	extern HANDLE GameHandle;

	//Base address of the game
	extern LPVOID BaseAddress;

	//Local player name, used for finding local entity by comparing names
	extern std::string LocalName;

	//Instance of memory class
	extern Mem Memory;

	//Instance of Overlay class
	extern Overlay Over;
}