#pragma once
#include "win.h"
#include "window.h"
#include "settings.h"

template <UINT_PTR from, UINT_PTR to, typename OptionsType>
class MenuRange
{
public:
	HMENU handle;
	
	void set(OptionsType option, OptionsType& o) noexcept
	{
		CheckMenuRadioItem(
			handle,
			from,
			to,
			from + static_cast<UINT_PTR>(option),
			MF_BYCOMMAND
		);
		o = option;
	}
	
	template <size_t ver>
	void set(OptionsType option, OptionsType Settings<ver>::* m, AutoSaveSettings<ver>& ass) noexcept
	{
		CheckMenuRadioItem(
			handle,
			from,
			to,
			from + static_cast<UINT_PTR>(option),
			MF_BYCOMMAND
		);
		ass.set(m, option);
	}

	void sync(const OptionsType& o) noexcept
	{
		CheckMenuRadioItem(
			handle,
			from,
			to,
			from + static_cast<UINT_PTR>(o),
			MF_BYCOMMAND
		);
	}
};