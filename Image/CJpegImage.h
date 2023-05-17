#pragma once

#include "CImage.h"

/*------------------------
		 CJpegImage

    JPEG 포맷 이미지 클래스
--------------------------*/

class CJpegImage : public CImage
{
public:
	CJpegImage();
	virtual ~CJpegImage();

	virtual CJpegImage*			Copy();

	virtual BOOL				Load(const CString& path);
	virtual BOOL				Load();
	virtual BOOL				Save(const CString& path);
	
	void						GetImageJpgDecodedData(BYTE* src, int height, int srcStride);
public:
	CString						GetDLLPath();
};

