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
	)
{
	DragAcceptFiles(*this, TRUE);
}

LRESULT MainWindow::PageWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
 		case WM_PAINT:
		{
			auto& mw = getParent<MainWindow>();
			const RECT rc = getSize();

			rt.beginDraw();
			rt.clear();

			if (!mw.pics.empty())
			{
				const auto& bmp = mw.pics.at(mw.pic);
				const struct {
					float w;
					float h;
				} zoomedSize{
					static_cast<float>(bmp.getWidth()) * mw.zoom * mw.userZoom,
					static_cast<float>(bmp.getHeight()) * mw.zoom * mw.userZoom
				};
				rt.drawBitmap(
					bmp,
					(static_cast<float>(rc.right) - zoomedSize.w) / 2.0F - mw.slidingPosition.getCurrX(),
					-mw.slidingPosition.getCurrY(),
					zoomedSize.w,
					zoomedSize.h
				);
			}

			if (rt.endDraw(mw.d2dfac))
				ValidateRect(*this, nullptr);
		}
		return 0;
		case WM_SIZE:
		{
			rt.resize(LOWORD(lParam), HIWORD(lParam));
			auto& mw = getParent<MainWindow>();
			mw.calculateZoom();
		}
		return 0;
		case WM_DROPFILES:
		{
			auto& mw = getParent<MainWindow>();
			const UHandle<HDROP, DragFinish> drop(reinterpret_cast<HDROP>(wParam));
			const unsigned count = DragQueryFileW(drop.get(), 0xFFFFFFFF, nullptr, 0);
			std::vector<std::wstring> filenames;
			filenames.reserve(count);
			for (unsigned i = 0; i < count; i++)
			{
				const unsigned charCount = DragQueryFileW(drop.get(), i, nullptr, 0);
				if (charCount == 0) throw WinError(L"Failed to get dropped file name size");
				std::vector<wchar_t> buffer(charCount + 1);
				if (DragQueryFileW(drop.get(), i, &buffer[0], buffer.size()) == 0)
					throw WinError(L"Failed to get dropped file name");
				filenames.emplace_back(buffer.data());
			}
			mw.loadPics(filenames);
			InvalidateRect(mw, nullptr, FALSE);
		}
		return 0;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}

void MainWindow::SlidingPosition::callback(MainWindow& wnd)
{
	if (wnd.slidingPosition.timeLeft == 0U)
	{
		wnd.slidingPosition.timer.stop();
	}
	else
	{
		wnd.slidingPosition.x += (wnd.slidingPosition.destX - wnd.slidingPosition.x) / wnd.slidingPosition.timeLeft;
		wnd.slidingPosition.y += (wnd.slidingPosition.destY - wnd.slidingPosition.y) / wnd.slidingPosition.timeLeft;
		wnd.slidingPosition.timeLeft--;
		InvalidateRect(wnd, nullptr, FALSE);
	}
}

void MainWindow::SlidingPosition::slideTo(float x, float y)
{
	destX = x;
	destY = y;
	if (timeLeft == 0U) timeLeft = 5U;
	timer.start();
}

void MainWindow::SlidingPosition::jumpTo(float x, float y)
{
	timer.stop();
	timeLeft = 0U;
	destX = x;
	destY = y;
	this->x = x;
	this->y = y;
}

std::optional<std::vector<std::wstring>> MainWindow::openFileDialogue()
{
	HRESULT hr = 0;
	
	ComPtr<IFileOpenDialog> fop;
	if (FAILED(hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_ALL,
		__uuidof(IFileOpenDialog),
		&fop
	))) throw WinError(L"Failed to create file dialogue", hr);

	DWORD flags = 0;
	if (FAILED(hr = fop->GetOptions(&flags)))
		throw WinError(L"Failed to get file dialogue options", hr);

	if (FAILED(hr = fop->SetOptions(flags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT)))
		throw WinError(L"Failed to set file dialogue options", hr);

	constexpr auto fileExts = std::to_array<COMDLG_FILTERSPEC>({
		{L"All Images", L"*.bmp;*.gif;*.ico;*.jpeg;*.jpe;*.jpg;*.png;*.tiff;*.tif"},
		{L"All", L"*.*"},
		{L"Bitmap", L"*.bmp"},
		{L"GIF", L"*.gif"},
		{L"Icon", L"*.ico"},
		{L"JPEG", L"*.jpeg;*.jpe;*.jpg"},
		{L"PNG", L"*.png"},
		{L"TIFF", L"*.tiff;*.tif"},
	});

	if (FAILED(hr = fop->SetFileTypes(
		fileExts.size(),
		fileExts.data()
	))) throw WinError(L"Failed to set file dialogue file types", hr);

	if (FAILED(hr = fop->SetFileTypeIndex(1)))
		throw WinError(L"Failed to set default file dialogue file type index", hr);

	if (FAILED(hr = fop->SetDefaultExtension(fileExts[0].pszSpec)))
		throw WinError(L"Failed to set default file dialogue file extension", hr);

	if (FAILED(hr = fop->Show(*this)))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
			return {};
		throw WinError(L"Failed to show file dialogue", hr);
	}

	ComPtr<IShellItemArray> sia;
	if (FAILED(hr = fop->GetResults(&sia)))
		throw WinError(L"Failed to get file dialogue results", hr);
	
	DWORD fileCount = 0;
	if (FAILED(hr = sia->GetCount(&fileCount)))
		throw WinError(L"Failed to get selected file count", hr);

	std::optional<std::vector<std::wstring>> files(std::in_place);
	files->reserve(fileCount);
	for (DWORD i = 0; i < fileCount; i++)
	{
		ComPtr<IShellItem> si;
		if (FAILED(hr = sia->GetItemAt(i, &si)))
			throw WinError(L"Failed to get selected file", hr);

		wchar_t* tmpFilename = nullptr;
		if (FAILED(hr = si->GetDisplayName(
			SIGDN_FILESYSPATH,
			&tmpFilename
		)))	throw WinError(L"Failed to get selected file", hr);
		const UHandle<wchar_t*, CoTaskMemFree> filename(tmpFilename);

		files->emplace_back(filename.get());
	}

	return files;
}

std::optional<std::wstring> MainWindow::openFolderDialogue()
{
	HRESULT hr = 0;
	
	ComPtr<IFileOpenDialog> fop;
	if (FAILED(hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_ALL,
		__uuidof(IFileOpenDialog),
		&fop
	))) throw WinError(L"Failed to create folder dialogue", hr);

	DWORD flags = 0;
	if (FAILED(hr = fop->GetOptions(&flags)))
		throw WinError(L"Failed to get folder dialogue options", hr);

	if (FAILED(hr = fop->SetOptions(flags | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS)))
		throw WinError(L"Failed to set folder dialogue options", hr);

	if (FAILED(hr = fop->Show(*this)))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
			return {};
		throw WinError(L"Failed to show folder dialogue", hr);
	}

	ComPtr<IShellItem> si;
	if (FAILED(hr = fop->GetResult(&si)))
		throw WinError(L"Failed to get folder dialogue result", hr);

	wchar_t* tmpFilename = nullptr;
	if (FAILED(hr = si->GetDisplayName(
		SIGDN_FILESYSPATH,
		&tmpFilename
	)))	throw WinError(L"Failed to get selected folder", hr);
	const UHandle<wchar_t*, CoTaskMemFree> filename(tmpFilename);

	return std::optional<std::wstring>(std::in_place, filename.get());
}

void MainWindow::loadPics(const std::vector<std::wstring>& files)
{
	if (!keepPages)
	{
		pics.clear();
		setPic(0);
	}
	pics.reserve(pics.size() + files.size());
	for (const auto& i : files)
	{
		try
		{
			pics.emplace_back(i.c_str(), wicfac, pageWindow.rt);
		}
		catch (const WinError& e)
		{
			pics.clear();
			setPic(0);
			std::wostringstream ss;
			ss << L"Unable to open file \""
			   << i
			   << L"\" (Error code: 0x"
			   << std::hex << e.hr
			   << L")";
			MessageBoxW(*this, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
			return;
		}
	}
	setPic(0);
}

void MainWindow::loadFolder(const std::wstring& folder)
{
	constexpr auto validFileExts = std::to_array<const wchar_t*>({
		L".bmp",
		L".gif",
		L".ico",
		L".jpeg",
		L".jpe",
		L".jpg",
		L".png",
		L".tiff",
		L".tif",
	});

	std::vector<std::wstring> imgs;
	for (const auto& p : std::filesystem::recursive_directory_iterator(folder))
	{
		if (!p.is_directory())
		{
			const std::wstring ext = p.path().extension();
			for (const auto& vfe : validFileExts)
			{
				if (ext == vfe)
				{
					imgs.emplace_back(p.path());
					break;
				}
			}
		}
	}

	loadPics(imgs);
}

void MainWindow::centerOnImage()
{
	if (!pics.empty())
	{
		const RECT pwSize = pageWindow.getSize();
		const auto picWidth = static_cast<float>(pics[pic].getWidth()) * zoom * userZoom;
		const auto picHeight = static_cast<float>(pics[pic].getHeight()) * zoom * userZoom;

		const float minX = static_cast<float>(pwSize.right) / 2.0F - picWidth / 2.0F;
		const float maxX = picWidth / 2.0F - static_cast<float>(pwSize.right) / 2.0F;

		const float maxY = picHeight - static_cast<float>(pwSize.bottom);

		float newX = 0.0F;
		float newY = 0.0F;

		if (minX < maxX)
		{
			newX = std::clamp<float>(slidingPosition.getX(), minX, maxX);
		}
		else
		{
			newX = 0.0F;
		}

		if (0.0F < maxY)
		{
			newY = std::clamp<float>(slidingPosition.getY(), 0.0F, maxY);
		}
		else
		{
			newY = 0.0F;
		}

		slidingPosition.slideTo(newX, newY);
	}
}

void MainWindow::calculateZoom()
{
	if (!pics.empty())
	{
		switch (fitMode)
		{
			case FitMode::realSizeOrWidth:
			{
				const RECT size = pageWindow.getSize();
				zoom = std::min<float>(size.right / (float)pics[pic].getWidth(), 1.0F);
			}
			break;
			case FitMode::width:
			{
				const RECT size = pageWindow.getSize();
				zoom = size.right / (float)pics[pic].getWidth();
			}
			break;
			case FitMode::realSizeOrHeight:
			{
				const RECT size = pageWindow.getSize();
				zoom = std::min<float>(size.bottom / (float)pics[pic].getHeight(), 1.0F);
			}
			break;
			case FitMode::height:
			{
				const RECT size = pageWindow.getSize();
				zoom = size.bottom / (float)pics[pic].getHeight();
			}
			break;
			case FitMode::realSize:
			{
				zoom = 1.0F;
			}
			break;
		}
	}
}

void MainWindow::onResize(int w, int h)
{
	MoveWindow(pageWindow, 0, 0, w, h, TRUE);
}

MainWindow::MainWindow(const std::vector<std::wstring>& files) : // NOLINT(cppcoreguidelines-pro-type-member-init)
	Window(
		defWindowClass,
		WS_OVERLAPPEDWINDOW,
		WS_EX_OVERLAPPEDWINDOW,
		L"Magna Reader",
		Menu{
			MenuItem::SubMenu{
				L"File",
				Menu{
					MenuItem::String{L"Open Files...", MenuId::openFiles},
					MenuItem::String{L"Open Folder...", MenuId::openFolder},
					MenuItem::Separator{},
					MenuItem::SubMenu{
						L"When Opening Pages",
						Menu{{
							MenuItem::RadioButton{L"Keep Old Ones", false, MenuId::keepPages},
							MenuItem::RadioButton{L"Close Old Ones", true, MenuId::closePages}
						}, keepPagesMenu}
					}
				}
			}
		}
	),
	pageWindow(*this),
	pic(0),
	easternReadingOrder(false),
	fitMode(FitMode::width),
	keepPages(false),
	slidingPosition(*this),
	userZoom(1.0F),
	zoom(1.0F)
{
	const RECT rc = getSize();
	onResize(rc.right, rc.bottom);

	loadPics(files);
}

LRESULT MainWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
		{
			onResize(LOWORD(lParam), HIWORD(lParam));
			InvalidateRect(*this, nullptr, FALSE);
		}
		return 0;
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_LEFT:
				{
					if (easternReadingOrder)
						nextPic();
					else
						prevPic();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_RIGHT:
				{
					if (easternReadingOrder)
						prevPic();
					else
						nextPic();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_UP:
				{
					slidingPosition.slideTo(
						slidingPosition.getX(),
						slidingPosition.getY() - 120.0F / 1.3F
					);
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_DOWN:
				{
					slidingPosition.slideTo(
						slidingPosition.getX(),
						slidingPosition.getY() + 120.0F / 1.3F
					);
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_PRIOR:
				{
					const RECT size = pageWindow.getSize();
					slidingPosition.slideTo(
						slidingPosition.getX(),
						slidingPosition.getY() - static_cast<float>(size.bottom) * 0.95F
					);
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_NEXT:
				{
					const RECT size = pageWindow.getSize();
					slidingPosition.slideTo(
						slidingPosition.getX(),
						slidingPosition.getY() + static_cast<float>(size.bottom) * 0.95F
					);
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_HOME:
				{
					slidingPosition.slideTo(
						slidingPosition.getX(),
						0.0F
					);
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_END:
				{
					const RECT size = pageWindow.getSize();
					const auto picHeight = static_cast<float>(pics[pic].getHeight()) * zoom * userZoom;
					const auto maxY = picHeight - static_cast<float>(size.bottom);
					
					if (0.0F < maxY)
					{
						slidingPosition.slideTo(
							slidingPosition.getX(),
							maxY
						);
					}
					else
					{
						slidingPosition.slideTo(
							slidingPosition.getX(),
							0.0F
						);
					}
					
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			const auto keys = LOWORD(wParam);
			if (((keys & MK_CONTROL) != 0) || ((keys & MK_SHIFT) != 0))
			{
				const RECT rc = getSize();
				POINT mousePos{};
				if (GetCursorPos(&mousePos) == 0)
					throw WinError(L"Failed to get cursor position");
				if (ScreenToClient(*this, &mousePos) == 0)
					throw WinError(L"Failed to convert screen coordinates to client coordinates");
				mousePos.x -= rc.right / 2;
				mousePos.y -= rc.bottom / 2;

				const float zoomFactor = std::exp(static_cast<float>(delta) / 1000.0F);
				userZoom *= zoomFactor;
				slidingPosition.jumpTo(
					(slidingPosition.getX() + mousePos.x) * zoomFactor - mousePos.x,
					(slidingPosition.getY() + mousePos.y + rc.bottom / 2.0F) * zoomFactor - mousePos.y - rc.bottom / 2.0F
				);
				centerOnImage();
				slidingPosition.skipSlide();
			}
			else
			{
				slidingPosition.slideTo(
					slidingPosition.getX(),
					slidingPosition.getY() - static_cast<float>(delta) / 1.3F
				);
				centerOnImage();
			}
			InvalidateRect(*this, nullptr, FALSE);
		}
		return 0;
		case WM_MOUSEHWHEEL:
		{
			const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			slidingPosition.slideTo(
				slidingPosition.getX() + static_cast<float>(delta) / 1.3F,
				slidingPosition.getY()
			);
			centerOnImage();
			InvalidateRect(*this, nullptr, FALSE);
		}
		return 0;
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == 0)
			{
				switch (LOWORD(wParam))
				{
					case MenuId::openFiles:
					{
						const auto files = openFileDialogue();
						if (files)
						{
							loadPics(*files);
							InvalidateRect(*this, nullptr, FALSE);
						}
					}
					return 0;
					case MenuId::openFolder:
					{
						const auto folder = openFolderDialogue();
						if (folder)
						{
							loadFolder(*folder);
							InvalidateRect(*this, nullptr, FALSE);
						}
					}
					return 0;
					case MenuId::keepPages:
					{
						CheckMenuRadioItem(
							keepPagesMenu,
							MenuId::keepPages,
							MenuId::closePages,
							MenuId::keepPages,
							MF_BYCOMMAND
						);
						keepPages = true;
					}
					return 0;
					case MenuId::closePages:
					{
						CheckMenuRadioItem(
							keepPagesMenu,
							MenuId::keepPages,
							MenuId::closePages,
							MenuId::closePages,
							MF_BYCOMMAND
						);
						keepPages = false;
					}
					return 0;
				}
			}
		}
		break;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}