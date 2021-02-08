#include <cstdlib>
#include <vector>
#include <sstream>
#include "utils.h"
#include "gui.h"
#include "lippincott.h"
#include "d2d.h"

std::vector<std::wstring> getCmdLineArgs(const wchar_t* cmdLine)
{
	int argc;
	const UHandle<wchar_t**, LocalFree> strs(CommandLineToArgvW(cmdLine, &argc));
	if (!strs) throw std::runtime_error("Failed to convert command line to argv");
	std::vector<std::wstring> vec;
	vec.reserve(argc);
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
		COMHandler ch;

		const auto args = std::wstring(pCmdLine).empty() ? std::vector<std::wstring>{} : getCmdLineArgs(pCmdLine);
		MainWindow mw(args);
		while (mw) mw.update();
	}
	catch (...)
	{
		lippincott();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}