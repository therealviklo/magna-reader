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
	  )
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

			for (const auto& i : pics)
				rt.drawBitmap(i, 0, 0, 500, 500);

			rt.endDraw(d2dfac);

			ValidateRect(*this, nullptr);
		}
		return 0;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}