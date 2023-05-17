#pragma once

#include <Windows.h>
#include <atlstr.h>

#define REGISTRY_PATH_VIEWFINDER	L"Software\\ViewFinder"

class CRegistry
{
private:
	CRegistry();

public:
	static CRegistry*	GetInstance();
	static void			DeleteInstance();
	
public:
	bool				SetValue(HKEY hKey, const CString& subKey, DWORD valueType, const CString& valueName, const BYTE* valueData, DWORD size = 0);
	bool				GetValue(HKEY hKey, const CString& subKey, const CString& valueName, LPBYTE lpValue);

public:
	HKEY				m_hKey;

private:
	static CRegistry*	m_instance;
};

