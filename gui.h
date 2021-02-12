#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <array>
#include "window.h"
#include "d2d.h"
#include "windowsx.h"
#include <shobjidl_core.h>

namespace MenuId
{
	enum MenuId_t : UINT_PTR
	{
		open
	};
}
using MenuId::MenuId_t;

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

	bool easternReadingOrder;
	enum struct FitMode {
		realSizeOrWidth,
		width,
		realSizeOrHeight,
		height,
		realSize
	} fitMode;

	float x;
	float y;
	float userZoom;
	float zoom;
	
	constexpr void setPic(size_t num)
	{
		pic = num;
		x = 0.0f;
		y = 0.0f;
		userZoom = 1.0f;
		calculateZoom();
	}

	std::optional<std::vector<std::wstring>> openFileDialogue();
	void loadPics(const std::vector<std::wstring>& files);

	void centerOnImage();
	void calculateZoom();
	void onResize(int w, int h);
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