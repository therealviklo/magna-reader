#pragma once
#include "win.h"
#include "winerror.h"

class COMHandler
{
public:
	COMHandler()
	{
		HRESULT hr;
		if (FAILED(hr = CoInitialize(nullptr)))
			throw WinError(L"Failed to initialise COM", hr);
	}

	~COMHandler()
	{
		CoUninitialize();
	}

	COMHandler(const COMHandler&) = delete;
	COMHandler& operator=(const COMHandler&) = delete;
};