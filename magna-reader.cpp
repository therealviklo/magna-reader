#include <cstdlib>
#include <vector>
#include <sstream>
#include "utils.h"
#include "gui.h"

std::vector<std::wstring> getCmdLineArgs(const wchar_t* cmdLine)
{
	int argc;
	const UHandle<wchar_t**, LocalFree> strs(CommandLineToArgvW(cmdLine, &argc));
	if (!strs) throw std::runtime_error("Failed to convert command line to argv");
	std::vector<std::wstring> vec;
	for (int i = 0; i < argc; i++)
	{
		vec.emplace_back(strs.get()[i]);
	}
	return vec;
}

int WINAPI wWinMain(
	HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	PWSTR pCmdLine,
	int /*nCmdShow*/
)
{
	try
	{
		const auto args = std::wstring(pCmdLine).empty() ? std::vector<std::wstring>{} : getCmdLineArgs(pCmdLine);
		MainWindow mw(args);
		while (mw) mw.update();
	}
	catch (const NoResWinError& e)
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
	catch (const WinError& e)
	{
		try
		{
			std::wostringstream ss;
			ss << e.what()
			   << " (Error code: 0x"
			   << std::hex << e.hr
			   << ")";
			MessageBoxW(
				nullptr,
				ss.str().c_str(),
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