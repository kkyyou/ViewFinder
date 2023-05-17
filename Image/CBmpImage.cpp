#include "CBmpImage.h"

CBmpImage::CBmpImage()
{
	m_format = eImageFormat::IMAGE_FORMAT_BMP;
}

CBmpImage::~CBmpImage()
{
}

CBmpImage* CBmpImage::Copy()
{
	CBmpImage* newBmpImage	  = new CBmpImage();
	newBmpImage->m_path	      = m_path;
	newBmpImage->m_width	  = m_width;
	newBmpImage->m_height	  = m_height;
	newBmpImage->m_bitmapData = m_bitmapData;
	newBmpImage->m_format	  = m_format;
	newBmpImage->m_bpp		  = m_bpp;
	newBmpImage->m_stride	  = m_stride;

	return newBmpImage;
}

BOOL CBmpImage::Load(const CString& path)
{
	if (path.IsEmpty())	return false;

	m_path = path;

	HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		return false;
	}

	// BITMAPFILEHEADER 읽기
	BITMAPFILEHEADER bmpHeader;
	DWORD readBytes;
	if (!ReadFile(hFile, &bmpHeader, sizeof(BITMAPFILEHEADER), &readBytes, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	// BMP 파일 체크
	if (bmpHeader.bfType != 0x4D42) {
		CloseHandle(hFile);
		return false;
	}

	// BITMAPINFOHEADER 읽기
	BITMAPINFOHEADER bmpInfo;
	if (!ReadFile(hFile, &bmpInfo, sizeof(BITMAPINFOHEADER), &readBytes, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	// 이미지 사이즈 셋팅
	if (bmpInfo.biWidth <= 0 || bmpInfo.biHeight <= 0)
		return false;

	//m_width = bmpInfo.biWidth;
	//m_height = bmpInfo.biHeight;
	Create(bmpInfo.biWidth, bmpInfo.biHeight, bmpInfo.biBitCount);

	// 파일 포인터를 비트맵헤더부분을 건너뛴 부분으로 설정 (비트맵 데이터는 헤더 뒤 부터 시작되기때문에) 
	if (SetFilePointer(hFile, bmpHeader.bfOffBits, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hFile);
		return false;
	}

	// 이전에 로드한게 존재하면 clear.
	if (!m_bitmapData.empty())
	{
		m_bitmapData.clear();
	}

	// width에 가까운 4의 배수 바이트 구하기 (bmp파일 형식이 width가 4의 배수 바이트임)
	int stride = ((((m_width * 24) + 31) & ~31) >> 3);

	// 비트맵 데이터 사이즈
	DWORD bitmapSize = stride * m_height;
	m_bitmapData.resize(bitmapSize);

	// 이미지 데이터를 한 줄 씩 읽는다.
	int toReadBytes = bmpInfo.biWidth * 3;
	DWORD imageBytesRead;
	if (!ReadFile(hFile, m_bitmapData.data(), bitmapSize, &imageBytesRead, NULL)) 
	{
		CloseHandle(hFile);
		return NULL;
	}

	// 해제
	CloseHandle(hFile);

	return true;
}

BOOL CBmpImage::Load()
{
	return Load(m_path);
}

BOOL CBmpImage::Save(const CString& path)
{
	// BITMAPFILEHEADER 셋팅
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = 0x4D42;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = (DWORD)bmpHeader.bfOffBits + (DWORD)m_bitmapData.size();
	
	// BITMAPINFOHEADER
	BITMAPINFOHEADER bmpInfo = CreateBitmapInfoHeader();

	// 파일을 쓰기위해 연다.
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// bmpHeader와 bmpInfo를 파일에 쓴다.
	DWORD writtenBytes;
	if (!WriteFile(hFile, &bmpHeader, sizeof(BITMAPFILEHEADER), &writtenBytes, NULL)
		|| !WriteFile(hFile, &bmpInfo, sizeof(BITMAPINFOHEADER), &writtenBytes, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	// 이미지 데이터를 m_bitmapData에 저장
	if (!WriteFile(hFile, m_bitmapData.data(), (DWORD)m_bitmapData.size(), &writtenBytes, NULL))
	{
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);

	return true;
}
