#include <cstdlib>
#include "utils.h"

int WINAPI wWinMain(
	HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	PWSTR /*pCmdLine*/,
	int /*nCmdShow*/
)
{
	try
	{

	}
	catch (const std::exception& e)
	{
		try
		{
			MessageBoxW(
				nullptr,
				stringToWstring(e.what()).c_str(),
				L"Error",
				MB_ICONERROR
			);
			return EXIT_FAILURE;
		}
		catch (...)
		{
			MessageBoxW(
				nullptr,
				L"Exception thrown when trying to display error message",
				L"Error",
				MB_ICONERROR
			);
			return EXIT_FAILURE;	
		}
	}
	catch (...)
	{
		MessageBoxW(
			nullptr,
			L"Unknown error",
			L"Error",
			MB_ICONERROR
		);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}