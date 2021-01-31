#pragma once
#include <vector>
#include "window.h"
#include "bitmap.h"

class MainWindow : public Window
{
private:
	std::vector<Bitmap> pics;
public:
	MainWindow(const std::vector<std::wstring>& files)
		: Window(
			defWindowClass,
			WS_OVERLAPPEDWINDOW,
			WS_EX_OVERLAPPEDWINDOW,
			L"Magna Reader"
		  )
	{
		pics.reserve(files.size());
		for (const auto& i : files)
		{
			pics.emplace_back(i.c_str());
		}
	}
	
	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};