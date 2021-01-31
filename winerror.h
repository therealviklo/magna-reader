#pragma once
#include <stdexcept>
#include "win.h"

struct WinError : public std::runtime_error
{
	DWORD hr;

	WinError(const char* str) noexcept
		: std::runtime_error(str),
		  hr(GetLastError()) {}
	WinError(const char* str, DWORD hr) noexcept
		: std::runtime_error(str),
		  hr(hr) {}
};

struct NoResWinError : public WinError
{
	NoResWinError(const char* str) noexcept
		: WinError(str, E_FAIL) {}
};