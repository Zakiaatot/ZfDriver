#include <Windows.h>
#include "Utils.h"

void Utils::AlertError(const wchar_t* msg)
{
	MessageBox(NULL, msg, L"ZfDriver Error", MB_OK);
}

bool Utils::ReleaseResource
(
	unsigned int uResourceId,
	const wchar_t* szResourceType,
	const wchar_t* szFileName
)
{
	const wchar_t* msg = NULL;

	do
	{
		// Find Resource
		HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(uResourceId), szResourceType);
		if (hRsrc == NULL)
		{
			msg = L"Find Resource Error!";
			break;
		}

		// Get Resource Size
		DWORD dwSize = SizeofResource(NULL, hRsrc);
		if (dwSize <= 0)
		{
			msg = L"Get Resource Error!";
			break;
		}

		// Load Resource
		HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
		if (hGlobal == NULL)
		{
			msg = L"Load Resource Error!";
			break;
		}

		// Lock Resource
		LPVOID lpRes = LockResource(hGlobal);
		if (lpRes == NULL)
		{
			msg = L"Lock Resource Error!";
			break;
		}
		HANDLE hFile = CreateFile
		(
			szFileName,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hFile == NULL)
		{
			msg = L"Create File Error!";
			break;
		}
		DWORD dwWriten = 0;
		bool bRes = WriteFile(hFile, lpRes, dwSize, &dwWriten, NULL);
		if (bRes == false || dwWriten <= 0)
		{
			msg = L"Write File Error!";
			break;
		}
	} while (false);

	if (msg == NULL)
	{
		return true;
	}
	else
	{
		Utils::AlertError(msg);
		return false;
	}
}

void Utils::GetAppPath(wchar_t* szCurFile)
{
	GetModuleFileName(0, szCurFile, MAX_PATH);
	size_t i = wcslen(szCurFile) - 1;
	for (; i > 0; --i)
	{
		if (szCurFile[i] == L'\\')
		{
			szCurFile[i + 1] = L'\0';
			break;
		}
	}
}