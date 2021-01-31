#include "gui.h"

LRESULT MainWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
 	   case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(*this, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_MENUBAR+1));
			for (const auto& i : pics)
				i.blit(hdc, 0, 0, 500, 500);

			EndPaint(*this, &ps);
		}
		return 0;
	}
	return DefWindowProcW(*this, msg, wParam, lParam);
}