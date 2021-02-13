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
					(static_cast<float>(rc.right) - zoomedSize.w) / 2.0F - mw.x,
					-mw.y,
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
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
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

void MainWindow::loadPics(const std::vector<std::wstring>& files)
{
	pics.clear();
	setPic(0);
	pics.reserve(files.size());
	for (const auto& i : files)
	{
		try
		{
			pics.emplace_back(i.c_str(), wicfac, pageWindow.rt);
		}
		catch (const WinError& e)
		{
			pics.clear();
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

		if (minX < maxX)
		{
			x = std::clamp<float>(x, minX, maxX);
		}
		else
		{
			x = 0.0F;
		}

		if (0.0F < maxY)
		{
			y = std::clamp<float>(y, 0.0F, maxY);
		}
		else
		{
			y = 0.0F;
		}
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

MainWindow::MainWindow(const std::vector<std::wstring>& files) :
	Window(
		defWindowClass,
		WS_OVERLAPPEDWINDOW,
		WS_EX_OVERLAPPEDWINDOW,
		L"Magna Reader",
		Menu{
			SubMenu{
				L"File",
				Menu{
					MenuItem{L"Open", MenuId::open}
				}
			}
		}
	),
	pageWindow(*this),
	pic(0),
	easternReadingOrder(false),
	fitMode(FitMode::width),
	x(0.0F),
	y(0.0F),
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
					y -= 120.0F / 1.3F;
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_DOWN:
				{
					y += 120.0F / 1.3F;
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_PRIOR:
				{
					const RECT size = pageWindow.getSize();
					y -= static_cast<float>(size.bottom) * 0.95F;
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_NEXT:
				{
					const RECT size = pageWindow.getSize();
					y += static_cast<float>(size.bottom) * 0.95F;
					centerOnImage();
					InvalidateRect(*this, nullptr, FALSE);
				}
				return 0;
				case VK_HOME:
				{
					y = 0.0F;
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
						y = maxY;
					}
					else
					{
						y = 0.0F;
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
				x = (x + mousePos.x) * zoomFactor - mousePos.x;
				y = (y + mousePos.y + rc.bottom / 2.0F) * zoomFactor - mousePos.y - rc.bottom / 2.0F;
			}
			else
			{
				y -= static_cast<float>(delta) / 1.3F;
			}
			centerOnImage();
			InvalidateRect(*this, nullptr, FALSE);
		}
		return 0;
		case WM_MOUSEHWHEEL:
		{
			const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			x += static_cast<float>(delta) / 1.3F;
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
					case MenuId::open:
					{
						const auto files = openFileDialogue();
						if (files)
						{
							loadPics(*files);
							InvalidateRect(*this, nullptr, FALSE);
						}
					}
					return 0;
				}
			}
		}
		break;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}