#pragma once
#include "window.h"

class MainWindow : public Window
{
public:
	MainWindow()
		: Window(
			defWindowClass,
			WS_OVERLAPPEDWINDOW,
			WS_EX_OVERLAPPEDWINDOW,
			L"Magna Reader"
		  ) {}
};