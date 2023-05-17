#pragma once

#include "CImage.h"

#include <atlstr.h>

/*--------------------------------
	       CImageFactory

        이미지 확장자에 맞게 
   이미지 클래스를 만들어주는 클래스
---------------------------------*/

class CImageFactory
{
public:
	CImageFactory();

public:
	static CImage* Create(const CString& path);
	static CImage* CreateByExtension(const CString& extension);
};

