#include "dialogue.h"

INT_PTR displayDialogueBox(const DialogueBox& db, HWND hWnd)
{
	auto alignSizeWord = [](size_t& i) -> void {
		i = ((i + 1) >> 1) << 1;
	};
	auto alignSizeDWord = [](size_t& i) -> void {
		i = ((i + 3) >> 2) << 2;
	};

	size_t totalSize = 0;
	totalSize += sizeof(DLGTEMPLATE); // Först en DLGTEMPLATE
	alignSizeWord(totalSize);
	totalSize += sizeof(uint16_t); // Sedan ingen meny
	alignSizeWord(totalSize);
	totalSize += sizeof(uint16_t); // Sedan ingen window class
	alignSizeWord(totalSize);
	totalSize += (db.db.title.size() + 1) * sizeof(wchar_t); // Sedan en sträng med titeln

	for (const auto& i : db.dbis)
	{
		alignSizeDWord(totalSize);
		totalSize += sizeof(DLGITEMTEMPLATE); // Först en DLFITEMTEMPLATE;
		alignSizeWord(totalSize);
		totalSize += sizeof(uint16_t) * 2; // Sedan en window class
		alignSizeWord(totalSize);
		totalSize += (i.title.size() + 1) * sizeof(wchar_t); // Sedan texten
		alignSizeWord(totalSize);
		totalSize += sizeof(uint16_t); // Ingen creation data
	}

	auto alignPtrWord = [&alignSizeWord](std::byte*& ptr) -> void {
		alignSizeWord(reinterpret_cast<size_t&>(ptr));
	};
	auto alignPtrDWord = [&alignSizeDWord](std::byte*& ptr) -> void {
		alignSizeDWord(reinterpret_cast<size_t&>(ptr));
	};

	std::unique_ptr<std::byte[]> dbt = std::make_unique<std::byte[]>(totalSize);
	std::byte* cp = dbt.get(); // Current Position

	DLGTEMPLATE& dbd = (DLGTEMPLATE&)*cp;
	dbd.style = db.db.style;
	dbd.dwExtendedStyle = 0;
	dbd.cdit = db.dbis.size();
	dbd.x = 0;
	dbd.y = 0;
	dbd.cx = db.db.w;
	dbd.cy = db.db.h;

	cp += sizeof(DLGTEMPLATE);
	alignPtrWord(cp);

	uint16_t& menu = (uint16_t&)*cp;
	menu = 0;

	cp += sizeof(uint16_t);
	alignPtrWord(cp);

	uint16_t& wndClass = (uint16_t&)*cp;
	wndClass = 0;

	cp += sizeof(uint16_t);
	alignPtrWord(cp);

	wchar_t* dlgTitle = (wchar_t*)cp;
	for (const wchar_t* i = db.db.title.c_str(); (*dlgTitle++ = *i++); );

	cp += (db.db.title.size() + 1) * sizeof(wchar_t);

	for (const auto& i : db.dbis)
	{
		alignPtrDWord(cp);

		DLGITEMTEMPLATE& dbid = (DLGITEMTEMPLATE&)*cp;
		dbid.style = i.style;
		dbid.dwExtendedStyle = 0;
		dbid.x = i.x;
		dbid.y = i.y;
		dbid.cx = i.w;
		dbid.cy = i.h;
		dbid.id = i.id;

		cp += sizeof(DLGITEMTEMPLATE);
		alignPtrWord(cp);

		uint16_t* wndClassArray = (uint16_t*)cp;
		wndClassArray[0] = 0xFFFF;
		wndClassArray[1] = i.wndClass;

		cp += sizeof(uint16_t) * 2;
		alignPtrWord(cp);

		wchar_t* dlgItemTitle = (wchar_t*)cp;
		for (const wchar_t* j = i.title.c_str(); (*dlgItemTitle++ = *j++); );

		cp += (i.title.size() + 1) * sizeof(wchar_t);
		alignPtrWord(cp);

		uint16_t& creationDataSize = (uint16_t&)*cp;
		creationDataSize = 0;
	}

	SetLastError(0);
	INT_PTR res = DialogBoxIndirectParamW(
		GetModuleHandleW(nullptr),
		(DLGTEMPLATE*)dbt.get(),
		hWnd,
		db.dlgProc,
		0
	);
	if (DWORD errCode = GetLastError(); (res == 0 || res == -1) && errCode)
		throw WinError(L"Failed to display dialogue box", errCode);
	return res;
}