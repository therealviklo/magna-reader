#pragma once
#include <stdexcept>
#include "werror.h"
#include "win.h"

struct WinError : public WRE
{
	DWORD hr;

	WinError(const wchar_t* str) noexcept
		: WRE(str),
		  hr(GetLastError()) {}
	WinError(const wchar_t* str, DWORD hr) noexcept
		: WRE(str),
		  hr(hr) {}
};

struct NoResWinError : public WinError
{
	NoResWinError(const wchar_t* str) noexcept
		: WinError(str, E_FAIL) {}
};