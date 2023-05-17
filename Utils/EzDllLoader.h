#pragma once


class EzDllLoader
{
protected:
	HMODULE m_hLicense = NULL;

public:
	EzDllLoader() {}
	EzDllLoader(LPCTSTR pszDllFile)
	{
		LoadDll(pszDllFile);
	}
	~EzDllLoader() { Free(); }

	bool LoadDll(LPCTSTR pszDllFile)
	{
		if (m_hLicense) return true;

		if (!PathFileExists(pszDllFile)) return false;

		m_hLicense = ::LoadLibrary(pszDllFile);
		if (!m_hLicense || m_hLicense == INVALID_HANDLE_VALUE) return false;

		return true;
	}

	void Free()
	{
		if (m_hLicense) ::FreeLibrary(m_hLicense);
		m_hLicense = NULL;
	}

	HMODULE Handle() { return m_hLicense; }
	bool	IsLoaded()
	{
		return m_hLicense == NULL || m_hLicense == INVALID_HANDLE_VALUE ? false : true;
	}
};

