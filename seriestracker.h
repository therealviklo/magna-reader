#pragma once
#include <string>
#include <optional>

size_t getFolderNumber(std::wstring seriesTrackerPath, std::wstring path) noexcept;
void saveFolderNumber(std::wstring seriesTrackerPath, std::wstring path, size_t folder) noexcept;
std::optional<std::wstring> getMostRecentSeries(std::wstring seriesTrackerPath) noexcept;
void saveMostRecentSeries(std::wstring seriesTrackerPath, std::wstring seriesPath) noexcept;