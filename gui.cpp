#include "gui.h"

MainWindow::PageWindow::PageWindow(HWND parent) :
	Window(
		defWindowClass,
		WS_CHILD,
		0,
		L"Page Window",
		parent
	),
	rt(
		*this,
		getParent<MainWindow>().d2dfac
	) {}

LRESULT MainWindow::PageWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto& mw = getParent<MainWindow>();
	switch (msg)
	{
 		case WM_PAINT:
		{
			rt.beginDraw();

			if (!mw.pics.empty())
			{
				const auto& bmp = mw.pics.at(mw.pic);
				rt.drawBitmap(bmp, mw.x, mw.y, bmp.getWidth(), bmp.getHeight());
			}

			if (rt.endDraw(mw.d2dfac))
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

void MainWindow::onResize(unsigned w, unsigned h)
{
	MoveWindow(pageWindow, 0, 0, w, h, TRUE);
}

MainWindow::MainWindow(const std::vector<std::wstring>& files) :
	Window(
		defWindowClass,
		WS_OVERLAPPEDWINDOW,
		WS_EX_OVERLAPPEDWINDOW,
		L"Magna Reader"
	),
	pageWindow(*this),
	pic(0),
	x(0.0),
	y(0.0)
{
	RECT rc{};
	if (!GetClientRect(*this, &rc))
		throw WinError("Failed to get client area");
	onResize(rc.right - rc.left, rc.bottom - rc.top);

	pics.reserve(files.size());
	for (const auto& i : files)
	{
		pics.emplace_back(i.c_str(), wicfac, pageWindow.rt);
	}
}

LRESULT MainWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
		{
			onResize(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}