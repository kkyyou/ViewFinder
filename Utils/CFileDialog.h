#pragma once

#include <atlstr.h>
#include <ShlObj.h>

#include "../Image/CImage.h"

class CFileDialog
{
public:
	static CString ShowFileDialog(const CLSID fileDlgType, eImageFormat imageFormat = eImageFormat::INVALID);
	static CString Open();
	static CString Save(eImageFormat imageFormat);
};
