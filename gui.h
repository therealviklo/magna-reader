#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <array>
#include <filesystem>
#include <sstream>
#include "window.h"
#include "dv2.h"
// #include "windowsx.h"
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
		openSeries,

		keepPages,
		closePages,
		
		ltr,
		rtl,
		
		realSizeOrWidth,
		width,
		realSizeOrHeight,
		height,
		realSize,

		resetZoom,
		resetImages,
		resetSettings,
		
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

	DV2 dv2;

	MenuRange<MenuId::keepPages, MenuId::closePages, bool> keepPagesMenu;
	MenuRange<MenuId::ltr, MenuId::rtl, bool> readingOrderMenu;
	MenuRange<MenuId::realSizeOrWidth, MenuId::realSize, FitMode> fitModeMenu;

	std::vector<std::wstring> folders;
	std::size_t folder;
	std::vector<Texture> pics;
	size_t pic;
	std::wstring seriesFolder;

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
		calculateZoom();
	}

	void syncMenus() noexcept
	{
		keepPagesMenu.sync(doNotKeepPages);
		readingOrderMenu.sync(ass->easternReadingOrder);
		fitModeMenu.sync(ass->fitMode);
	}

	std::optional<std::vector<std::wstring>> openFileDialogue();
	std::optional<std::vector<std::wstring>> openFolderDialogue(bool multiple = true);
	void loadPics(const std::vector<std::wstring>& files);
	void loadFolders(const std::vector<std::wstring>& folders);
	void loadSeries(std::wstring seriesFoler);

	void centerOnImage();
	void calculateZoom();
public:
	MainWindow(const std::vector<std::wstring>& files);

	void nextPic(size_t num = 1);
	void prevPic(size_t num = 1);
	void nextFolder();
	void prevFolder();

	void draw();

	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};