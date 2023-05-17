#include "CUtils.h"
#include "../Image/MyOpencvApi.h"
#include "../Image/CImage.h"
#include "../Image/CImageFactory.h"

#pragma warning(push)
#pragma warning(disable: 26495)
#pragma warning(disable: 26451)
#pragma warning(disable: 26439)
#pragma warning(disable: 6294)
#pragma warning(disable: 6201)
#pragma warning(disable: 6269)
#include "opencv2/opencv.hpp"
#pragma warning(pop)

#include <algorithm>

using namespace std;
using namespace cv;

bool CUtils::IsValidFilePath(const CString& path)
{
    return PathFileExists(path) == TRUE;
}

// @parameter : extension ex) .bmp, .jpg
bool CUtils::IsSupportedExtension(const CString& extension)
{
    if (extension.Compare(L".bmp") == 0)
        return true;

    if (extension.Compare(L".jpg") == 0)
        return true;

    return false;
}

bool CUtils::IsFolder(const CString& path)
{
	WIN32_FIND_DATAW findData;

	// 경로로 폴더 검색을 할 때는 드라이브 심볼을 지운다.
	CString findPath = path;
	CUtils::RemoveDriveSymbol(&findPath);
	HANDLE hFind = FindFirstFile(findPath, &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// 폴더가 아니다.
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return false;
	}
	else
	{
		return false;
	}

    return true;
}

void CUtils::RemoveDriveSymbol(CString* path)
{
	if (path == nullptr)
		return;

	if (path->GetLength() < 4)
		return;

	// 드라이브 경로면 심볼 제거한다.
	if (path->Left(4).Compare(L"#DRV") == 0)
	{
		path->Delete(0, 4);
	}
}

bool CUtils::IsBMP(const CString& path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// BMP Header
	BITMAPFILEHEADER bmpHeader;
	DWORD bytesRead;
	if (!ReadFile(hFile, &bmpHeader, sizeof(BITMAPFILEHEADER), &bytesRead, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	// BMP 체크
	if (bmpHeader.bfType != 0x4D42)
	{
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);
	return true;
}

bool CUtils::IsJPG(const CString& path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// 헤더
	char jpegHeader[2];
	DWORD bytesRead;
	if (!ReadFile(hFile, jpegHeader, 2, &bytesRead, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	// JPG 체크
	if ((unsigned char)jpegHeader[0] == 0xFF && (unsigned char)jpegHeader[1] == 0xD8)
	{
		CloseHandle(hFile);
		return true;
	}

	CloseHandle(hFile);
	return false;
}

bool CUtils::IsPNG(const CString& path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	BYTE buffer[8];
	DWORD bytesRead;
	if (!ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL))
	{
		CloseHandle(hFile);
		return false;
	}


	// PNG 체크
	if (buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47 && buffer[4] == 0x0D && buffer[5] == 0x0A && buffer[6] == 0x1A && buffer[7] == 0x0A) 
	{
		CloseHandle(hFile);
		return true;
	}

	CloseHandle(hFile);
	return false;
}

// path 내부의 파일/폴더를 찾는다
vector<CString> CUtils::GetFolderContents(const CString& path, int* folderCount)
{
	vector<CString> contents;

	WIN32_FIND_DATAW findData;

	// 경로로 폴더 검색을 할 때는 드라이브 심볼을 지운다.
	CString findPath = path;
	CUtils::RemoveDriveSymbol(&findPath);
	HANDLE hFind = FindFirstFile((findPath + L"\\*"), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (wcscmp(findData.cFileName, L".") == 0)
				continue;

			CString copyPath = findPath;
			copyPath.Append(L"\\");
			copyPath.Append(findData.cFileName);

			// 확장자 체크
			CString extension = PathFindExtension(copyPath);
			if (!CUtils::IsFolder(copyPath) && extension.IsEmpty())
				continue;

			if (extension.Compare(L".bmp") != 0 && extension.Compare(L".jpg") != 0
				/*&& extension.Compare(L".png") != 0*/ && extension.Compare(L".jpeg") != 0
				&& wcscmp(findData.cFileName, L"..") != 0 && extension.IsEmpty() == false)
			{
				continue;
			}

			if (folderCount)
			{
				if (CUtils::IsFolder(copyPath))	(*folderCount)++;
			}

			contents.push_back(findData.cFileName);

		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}

	// 폴더 우선 정렬
	std::sort(contents.begin(), contents.end(), [findPath](const CString& a, const CString& b)
		{
			CString fullPathA = L"", fullPathB = L"";
			fullPathA = findPath + L"\\" + a;
			fullPathB = findPath + L"\\" + b;
			return CUtils::IsFolder(fullPathA) > CUtils::IsFolder(fullPathB);
		});

	return contents;
}

bool CUtils::LoadThumnail(const CString& path, int thumnailWidth, int thumnailHeight, int index, HDC hdc, HBITMAP& hbitmap, CThreadPool& threadPool, CImage** image)
{
	if (threadPool.IsStop())	return false;
	
	// 이미지 로드
	cv::String cvPath((const char*)CStringA(path));
	Mat imgMat = imread(cvPath);
	if (imgMat.empty())	return false;

	// 비율 계산해서 이미지 그릴 위치 정하기
	int XDest, YDest, nDestWidth, nDestHeight;
	CUtils::CalcThumnailPos(thumnailWidth, thumnailHeight, imgMat.cols, imgMat.rows, XDest, YDest, nDestWidth, nDestHeight);

	// CImage : 썸네일 가지고있기위해 적당한 이미지 사이즈로 리사이즈 (nDestWidth * 6, nDestHeight * 6)
	Mat newMat;
	cv::resize(imgMat, newMat, cv::Size(nDestWidth * 6, nDestHeight * 6));

	if (threadPool.IsStop())	return false;

	// Thumnail Image
	*image = MyOpencvApi::CreateFromCvMat(newMat, path);
	if (!*image)	return false;

	if (threadPool.IsStop())
	{
		delete (*image);
		return false;
	}

	// 썸네일
	HDC hDC = ::CreateCompatibleDC(hdc);
	HBITMAP bm = ::CreateCompatibleBitmap(hdc, thumnailWidth, thumnailHeight);
	HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(hDC, bm);

	// 하얗게 바탕을 칠한다.
	RECT rcBorder;
	rcBorder.left = rcBorder.top = 0;
	rcBorder.right = thumnailWidth;
	rcBorder.bottom = thumnailHeight;
	HBRUSH hBrushBk = ::CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hDC, &rcBorder, hBrushBk);

	// 썸네일 그리기
	SetStretchBltMode(hDC, HALFTONE);
	BITMAPINFOHEADER bmpInfoHeader = (*image)->CreateBitmapInfoHeader();
	StretchDIBits(hDC, XDest, YDest, nDestWidth, nDestHeight, 0, 0, (*image)->W(), (*image)->H(), (*image)->GetBitmapData().data()
		, (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS, SRCCOPY);
	SelectObject(hDC, pOldBitmapImage);
	
	if (threadPool.IsStop())
	{
		delete (*image);
		return false;
	}

	// HBITMAP
	hbitmap = bm;

	// 정리
	DeleteDC(hDC);
	DeleteObject(hBrushBk);

	if (threadPool.IsStop())
	{
		delete (*image);
		return false;
	}

	// 이미지 해제
	//delete image;
	
	return true;
}

void CUtils::CalcThumnailPos(int thumWidth, int thumHeight, int imgWidth, int imgHeight, int& destX, int& destY, int& destWidth, int& destHeight)
{
	const float fRatio = (float)thumWidth / thumHeight;
	const float fImgRatio = (float)imgHeight / imgWidth;
	if (fImgRatio > fRatio)
	{
		destWidth = (int)(thumHeight / fImgRatio);
		destX = (thumWidth - destWidth) / 2;
		destY = 0;
		destHeight = thumHeight;
	}
	else
	{
		destX = 0;
		destWidth = thumWidth;
		destHeight = (int)(thumWidth * fImgRatio);
		destY = (thumHeight - destHeight) / 2;
	}
}
