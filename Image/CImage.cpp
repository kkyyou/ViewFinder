#include "CImage.h"
#include "MyOpencvApi.h"

#include "../Image/CImageFactory.h"

using namespace std;
using namespace cv;

CImage::CImage() :
	m_width(0),
	m_height(0),
	m_format(eImageFormat::INVALID),
	m_bpp(24),
	m_stride(0)
{
}

CImage::CImage(const CString& path) :
	m_path(path),
	m_width(0),
	m_height(0),
	m_format(eImageFormat::INVALID),
	m_bpp(24),
	m_stride(0)
{
}

CImage::CImage(const CImage& src) :
	m_path(src.m_path),
	m_width(src.m_width),
	m_height(src.m_height),
	m_bitmapData(src.m_bitmapData),
	m_format(src.m_format),
	m_bpp(src.bpp()),
	m_stride(src.Stride())
{

}

CImage::~CImage()
{
	m_bitmapData.clear();
}

BOOL CImage::SaveDiffExtension(const CString& path)
{
	CString extension = PathFindExtension(path);
	CImage* image = CImageFactory::CreateByExtension(extension);
	if (!image)	return false;

	image->SetPath(path);
	image->SetBitmapData(m_bitmapData);
	image->SetWidth(m_width);
	image->SetHeight(m_height);

	if (!image->Save(path))
	{
		delete image;
		return false;
	}

	delete image;
	return true;
}

bool CImage::Create(int w, int h, int bpp)
{
	Free();

	m_width = w;
	m_height = h;
	m_bpp = bpp;
	m_stride = GetBmpStride(bpp);
	try
	{
		m_bitmapData = vector<BYTE>(GetBmpStride(bpp) * h);
	}
	catch (std::exception const& /* e */)
	{
		return false;
	}

	return !m_bitmapData.empty();
}

void CImage::Free()
{
	m_bitmapData.clear();
	m_width = 0;
	m_height = 0;
	m_bpp = 0;
	m_stride = 0;
}

bool CImage::GetRawDataFrom(BYTE* srcData, int w, int h, int srcStride)
{
	if (GetWidth() != w || GetHeight() != h)
	{
		if (!Create(w, h)) return false;
	}

	int size = min(m_stride, srcStride);
	BYTE* src = srcData;
	BYTE* dst = m_bitmapData.data();
	for (int y = 0; y < h; y++)
	{
 		memcpy(dst, src, size);
		src += srcStride;
		dst += m_stride;
	}

	return true;
}


int CImage::GetWidth() const
{
	return m_width;
}

int CImage::GetHeight() const
{
	return m_height;
}

CString CImage::GetPath() const
{
	return m_path;
}

std::vector<BYTE> CImage::GetBitmapData() const
{
	return m_bitmapData;
}

std::vector<BYTE>* CImage::GetBitmapDataRef()
{
	return &m_bitmapData;
}

void CImage::SetBitmapData(const vector<BYTE>& data)
{
	m_bitmapData = data;
}

void CImage::SetWidth(const int width)
{
	m_width = width;
}

void CImage::SetHeight(const int height)
{
	m_height = height;
}

void CImage::SetPath(const CString& path)
{
	m_path = path;
}

void CImage::SetImageFormat(const eImageFormat& imgFormat)
{
	m_format = imgFormat;
}

void CImage::Draw(HDC hdc, int x, int y, float zoomScale)
{
	if (m_bitmapData.empty())
		return;

	// 그리기
	BITMAPINFOHEADER bmpInfoHeader = CreateBitmapInfoHeader();
	SetStretchBltMode(hdc, HALFTONE);
	StretchDIBits(hdc, x, y, (int)(m_width * zoomScale), (int)(m_height * zoomScale), 0, 0, m_width, m_height, m_bitmapData.data()
		, (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS, SRCCOPY);
}

void CImage::GetOneDimenData(vector<vector<BYTE>>& src, vector<BYTE>* dest)
{
	size_t index = 0;
	for (const vector<BYTE>& vb : src)
	{
		for (const BYTE& b : vb)
		{
			(*dest)[index++] = b;
		}
	}
}

void CImage::GetTwoDimenBmpData(const vector<BYTE>& src, vector<vector<BYTE>>* dest)
{
	if (dest == nullptr)
		return;

	int stride = GetBmpStride(24);
	int padding = stride - (m_width * 3);

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < stride; x++)
		{
			// stride 추가
			if (x >= stride - padding)
			{
				(*dest)[y][x] = 0;
			}
			else
			{
				(*dest)[y][x] = src[y * m_width * 3 + x];
			}
		}
	}
}

void CImage::GetTwoDimenJpgData(const std::vector<BYTE>& src, std::vector<std::vector<BYTE>>* dest)
{
	if (dest == nullptr)
		return;

	int stride = GetBmpStride(24);
	int padding = stride - (m_width * 3);

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < stride; x++)
		{
			// stride 제외
			if (x >= stride - padding)
			{
				continue;
			}
			else
			{
				(*dest)[y][x] = src[y * stride + x];
			}
		}
	}
}

void CImage::FlipUpAndDown(vector<vector<BYTE>>* imageData)
{
	if (imageData == nullptr)
		return;

	// 위 아래 뒤집기
	vector<BYTE>* p = &((*imageData)[0]);
	vector<BYTE>* q = &((*imageData)[imageData->size() - 1]);
	while (p < q)
	{
		vector<BYTE> temp = *p;
		*p = *q;
		*q = temp;

		p++;
		q--;
	}
}

void CImage::ReverseRgbOrder(std::vector<std::vector<BYTE>>* imageData)
{
	if (imageData == nullptr)
		return;

	if (imageData->size() == 0)
		return;

	int stride = GetBmpStride(24);
	int padding = stride - (m_width * 3);
	size_t rowSize = (*imageData)[0].size();
	
	vector<BYTE> vBGR(rowSize);
	int index = 0;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < stride; x += 3)
		{
			// stride 인 경우
			if (x >= stride - padding)
			{
				for (int padCnt = 0; padCnt < padding; padCnt++)
				{
					vBGR[index++] = (*imageData)[y][x + padCnt];
				}
			}
			else
			{
				vBGR[index++] = (*imageData)[y][x + 2];		// B
				vBGR[index++] = (*imageData)[y][x + 1];		// G
				vBGR[index++] = (*imageData)[y][x];			// R
			}
		}

		(*imageData)[y] = vBGR;

		vBGR.clear();
		vBGR.resize(rowSize);
		index = 0;
	}
}

int CImage::GetBmpStride(int bitCount)
{
	// bmp 파일의 한 행은 4의 배수 바이트
	// 1. width의 총 비트 수를 구하고
	// 2. +31을한다 : 가장 가까운 32의 배수로 반올림하기위해
	// 3. & (~31) 연산으로 가장가까운 32의 배수로 내림한다.
	// 4. 3비트를 오른쪽으로 쉬프트연산하여 8로 나눈다 (바이트로 변환)
	return ((((m_width * bitCount) + 31) & ~31) >> 3);
}

BITMAPINFOHEADER CImage::CreateBitmapInfoHeader()
{
	// 픽셀당 비트수 24 = 3바이트
	int biBitCount = 24;
	int stride = GetBmpStride(biBitCount);

	BITMAPINFOHEADER bmpInfoHeader = { 0 };
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = m_width;
	bmpInfoHeader.biHeight = m_height;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = biBitCount;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = stride * m_height;
	bmpInfoHeader.biXPelsPerMeter = 0;
	bmpInfoHeader.biYPelsPerMeter = 0;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;

	return bmpInfoHeader;
}

eImageFormat CImage::GetImageFormat() const
{
	return m_format;
}