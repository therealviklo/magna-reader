#pragma once
#include <vector>
#include <cstddef>
#include "window.h"
#include "d2d.h"

class MainWindow : public Window
{
private:
	D2DFactory d2dfac;
	WICFactory wicfac;

	class PageWindow : public Window
	{
		friend MainWindow;
	private:
		RenderTarget rt;
	public:
		PageWindow(HWND parent);
		
		LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	} pageWindow;

	std::vector<Bitmap> pics;
	size_t pic;
	float x;
	float y;
	
	constexpr void setPic(size_t num) noexcept
	{
		pic = num;
		x = 0.0;
		y = 0.0;
	}

	void onResize(unsigned w, unsigned h);
public:
	MainWindow(const std::vector<std::wstring>& files);

	constexpr void nextPic(size_t num = 1) noexcept
	{
		if (pic < pics.size() - 1)
		{
			setPic(pic + num);
		}
	}
	constexpr void prevPic(size_t num = 1) noexcept
	{
		if (pic > 0)
		{
			setPic(pic - num);
		}
	}
	
	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};