#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <array>
#include <filesystem>
#include <sstream>
#include "window.h"
#include "d2d.h"
#include "windowsx.h"
#include <shobjidl_core.h>
#include "settings.h"
#include "menuhelp.h"
#include "dialogue.h"

namespace MenuId
{
	enum MenuId_t : UINT_PTR
	{
		openFiles,
		openFolder,

		keepPages,
		closePages,
		
		ltr,
		rtl,
		
		realSizeOrWidth,
		width,
		realSizeOrHeight,
		height,
		realSize,
		
		startAutoRead,
		stopAutoRead,
		setAutoReadSpeed
	};
}
using MenuId::MenuId_t;

namespace TimerId
{
	enum TimerId_t : UINT_PTR
	{
		slide = 123
	};
}
using TimerId::TimerId_t;

class MainWindow : public Window
{
private:
	static constexpr size_t CurrSettingsVer = 0;
	using CASS = AutoSaveSettings<CurrSettingsVer>;
	using CS = Settings<CurrSettingsVer>;

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
	MenuRange<MenuId::keepPages, MenuId::closePages, bool> keepPagesMenu;
	MenuRange<MenuId::ltr, MenuId::rtl, bool> readingOrderMenu;
	MenuRange<MenuId::realSizeOrWidth, MenuId::realSize, FitMode> fitModeMenu;

	std::vector<Bitmap> pics;
	size_t pic;

	CASS ass;
	bool doNotKeepPages;

	class SlidingPosition
	{
	private:
		float x;
		float y;
		float destX;
		float destY;
		unsigned timeLeft;
		std::optional<float> autoReadPos;
		
		static void callback(MainWindow& wnd);
		
		Timer<MainWindow, &callback> timer;
	public:
		SlidingPosition(HWND hWnd) noexcept :
			x(0.0F),
			y(0.0F),
			destX(0.0F),
			destY(0.0F),
			timeLeft(0U),
			timer(
				TimerId::slide,
				hWnd,
				USER_TIMER_MINIMUM
			) {}

		void slideTo(float x, float y);
		void jumpTo(float x, float y);
		void skipSlide() { jumpTo(destX, destY); }
		void startAutoReadAt(float pos = 0.0F);

		constexpr float getCurrX() const noexcept { return x; }
		constexpr float getCurrY() const noexcept { return y; }
		constexpr float getX() const noexcept { return destX; }
		constexpr float getY() const noexcept { return destY; }

		constexpr bool isAutoReading() const noexcept { return autoReadPos.has_value(); }
	} slidingPosition;
	float userZoom;
	float zoom;

	void autoRead();
	void toggleAutoRead()
	{
		if (slidingPosition.isAutoReading())
		{
			slidingPosition.skipSlide();
		}
		else
		{
			autoRead();
		}
	}
	
	constexpr void setPic(size_t num)
	{
		pic = num;
		slidingPosition.jumpTo(0.0F, 0.0F);
		userZoom = 1.0f;
		calculateZoom();
	}

	std::optional<std::vector<std::wstring>> openFileDialogue();
	std::optional<std::vector<std::wstring>> openFolderDialogue();
	void loadPics(const std::vector<std::wstring>& files);
	void loadFolders(const std::vector<std::wstring>& folders);

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