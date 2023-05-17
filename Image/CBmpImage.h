#pragma once

#include "CImage.h"

/*------------------------
		 CBmpImage

    BMP 포맷 이미지 클래스
--------------------------*/

class CBmpImage : public CImage
{
public:
	CBmpImage();
	virtual ~CBmpImage();

	virtual CBmpImage*			Copy();

	virtual BOOL				Load(const CString& path);
	virtual BOOL				Load();
	virtual BOOL				Save(const CString& path);

};

