#pragma once
#include <vector>
#include "window.h"
#include "d2d.h"

class MainWindow : public Window
{
private:
	D2DFactory d2dfac;
	WICFactory wicfac;
	RenderTarget rt;
	std::vector<Bitmap> pics;
public:
	MainWindow(const std::vector<std::wstring>& files);
	
	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};