#include "gui.h"

MainWindow::MainWindow(const std::vector<std::wstring>& files)
	: Window(
		defWindowClass,
		WS_OVERLAPPEDWINDOW,
		WS_EX_OVERLAPPEDWINDOW,
		L"Magna Reader"
	  ),
	  rt(
		*this,
		d2dfac
	  ),
	  pic(0)
{
	pics.reserve(files.size());
	for (const auto& i : files)
	{
		pics.emplace_back(i.c_str(), wicfac, rt);
	}
}

LRESULT MainWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
 	   case WM_PAINT:
		{
			rt.beginDraw();

			if (!pics.empty())
			{
				const auto& bmp = pics.at(pic);
				rt.drawBitmap(bmp, x, y, bmp.getWidth(), bmp.getHeight());
			}

			if (rt.endDraw(d2dfac))
				ValidateRect(*this, nullptr);
		}
		return 0;
		case WM_SIZE:
		{
			rt.resize(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}