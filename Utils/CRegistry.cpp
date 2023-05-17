#include "CRegistry.h"

CRegistry* CRegistry::m_instance = nullptr;

CRegistry::CRegistry() :
	m_hKey(nullptr)
{
}

CRegistry* CRegistry::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new CRegistry();
	}

	return m_instance;
}

void CRegistry::DeleteInstance()
{
	if (m_instance)
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

bool CRegistry::SetValue(HKEY hKey, const CString& subKey, DWORD valueType, const CString& valueName, const BYTE* valueData, DWORD size)
{
	bool success = false;
	if (RegCreateKeyEx(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &m_hKey, NULL) == ERROR_SUCCESS)
	{
		if (valueType == REG_DWORD)
		{
			RegSetValueEx(m_hKey, valueName, 0, REG_DWORD, valueData, sizeof(DWORD));
			success = true;
		}            
		else if (valueType == REG_SZ)
		{
			RegSetValueEx(m_hKey, valueName, 0, REG_SZ, valueData, size);
			success = true;
		}
		else
		{
			success = false;
		}
	}

	RegCloseKey(m_hKey);

	return success;
}

bool CRegistry::GetValue(HKEY hKey, const CString& subKey, const CString& valueName, LPBYTE lpValue)
{
	bool success = false;
	DWORD dwSize = 0;
	if (RegOpenKeyEx(hKey, subKey, 0, KEY_QUERY_VALUE, &m_hKey) == ERROR_SUCCESS)
	{
		RegQueryValueExW(m_hKey, valueName, NULL, NULL, NULL, &dwSize);
		if (dwSize)
		{
			if (RegQueryValueEx(m_hKey, valueName, NULL, NULL, lpValue, &dwSize) == ERROR_SUCCESS)
			{
				success = true;
			}
		}
	}

	RegCloseKey(m_hKey);

	return success;
}
