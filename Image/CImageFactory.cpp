#include "CImageFactory.h"
#include "CBmpImage.h"
#include "CJpegImage.h"

#include "../Utils/CUtils.h"

CImageFactory::CImageFactory()
{
}

CImage* CImageFactory::Create(const CString& path)
{
	if		(CUtils::IsBMP(path)) return new CBmpImage();
	else if (CUtils::IsJPG(path)) return new CJpegImage();
	else                          return nullptr;
}

CImage* CImageFactory::CreateByExtension(const CString& extension)
{
	if (extension.Compare(L".bmp") == 0)
	{
		return new CBmpImage();
	}
	else if (extension.Compare(L".jpg") == 0 || extension.Compare(L".jpeg"))
	{
		return new CJpegImage();
	}

	return nullptr;
}
