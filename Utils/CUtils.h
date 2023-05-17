#pragma once
#include "../Thread/CThreadPool.h"

#include <atlstr.h>
#include <vector>

/*------------------------
		  CUtils

	     유틸 클래스
--------------------------*/
class CImage;
namespace CUtils
{
	bool				  IsValidFilePath(const CString& path);
	bool				  IsSupportedExtension(const CString& extension); 
	bool				  IsFolder(const CString& path);
						  
	void				  RemoveDriveSymbol(CString* path);
						  
	bool				  IsBMP(const CString& path);
	bool				  IsJPG(const CString& path);
	bool				  IsPNG(const CString& path);

	std::vector<CString>  GetFolderContents(const CString& path, int* forderCount = nullptr);
	bool                  LoadThumnail(const CString& path, int thumnailWidth, int thumnailHeight, int index, HDC hdc, HBITMAP& hbitmap, CThreadPool& threadPool, CImage** image);
	void				  CalcThumnailPos(int thumWidth, int thumHeight, int imgWidth, int imgHeight, int& destX, int& destY, int& destWidth, int& destHeight);
}