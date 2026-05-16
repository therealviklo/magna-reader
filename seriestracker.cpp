#include "seriestracker.h"
#include <filesystem>
#include <fstream>
#include "json.h"
#include "utils.h"

JSONValue readJsonFromFile(std::wstring seriesTrackerPath)
{
	const auto size = std::filesystem::file_size(seriesTrackerPath);
	std::string content(size, '\0');
	{
		std::ifstream in(seriesTrackerPath, std::ios::binary);
		in.read(content.data(), size);
	}

	return parseJson(content);
}

size_t getFolderNumber(std::wstring seriesTrackerPath, std::wstring path) noexcept
{
	try
	{
		if (!std::filesystem::exists(seriesTrackerPath))
		{
			return 0;
		}
	
		const std::string spath = wstringToString(std::filesystem::canonical(path));
	
		const auto obj = std::get<JSONObject>(readJsonFromFile(seriesTrackerPath));
		const auto& folderNumbers = std::get<JSONObject>(obj.at("folderNumbers"));
		if (!folderNumbers.contains(spath))
		{
			return 0;
		}
	
		return std::get<double>(folderNumbers.at(spath));
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to load series number (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
		return 0;
	}
}

void saveFolderNumber(std::wstring seriesTrackerPath, std::wstring path, size_t folder) noexcept
{
	try
	{
		const std::string spath = wstringToString(std::filesystem::canonical(path));
		
		JSONObject obj;
		if (std::filesystem::exists(seriesTrackerPath))
			obj = std::get<JSONObject>(readJsonFromFile(seriesTrackerPath));
		if (!obj.contains("folderNumbers"))
			obj.emplace("folderNumbers", JSONObject{});
		JSONObject& folderNumbers = std::get<JSONObject>(obj.at("folderNumbers"));
		folderNumbers.insert_or_assign(spath, (JSONValue)(double)folder);
	
		const std::string jsonString = encodeJson(obj);
	
		std::ofstream out(seriesTrackerPath, std::ios::binary);
		out.write(jsonString.data(), jsonString.size());
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to save series number (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
	}
}

std::optional<std::wstring> getMostRecentSeries(std::wstring seriesTrackerPath) noexcept
{
	try
	{
		if (!std::filesystem::exists(seriesTrackerPath))
			return std::nullopt;
		JSONObject obj = std::get<JSONObject>(readJsonFromFile(seriesTrackerPath));
		if (!obj.contains("mostRecentSeries"))
			return std::nullopt;
		return stringToWstring(std::get<std::string>(obj.at("mostRecentSeries")));
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to get most recent series (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
		return std::nullopt;
	}
}

void saveMostRecentSeries(std::wstring seriesTrackerPath, std::wstring seriesPath) noexcept
{
	try
	{
		JSONObject obj;
		if (std::filesystem::exists(seriesTrackerPath))
			obj = std::get<JSONObject>(readJsonFromFile(seriesTrackerPath));
		obj.insert_or_assign("mostRecentSeries", (JSONValue)wstringToString(seriesPath));

		const std::string jsonString = encodeJson(obj);
	
		std::ofstream out(seriesTrackerPath, std::ios::binary);
		out.write(jsonString.data(), jsonString.size());
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to save most recent series (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
	}
}
