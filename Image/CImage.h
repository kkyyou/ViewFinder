#pragma once

#include <atlstr.h>
#include <vector>

#ifdef _DEBUG
#pragma comment(lib, "opencv/lib/opencv_world470d.lib")
#else
#pragma comment(lib, "opencv/lib/opencv_world470.lib")
#endif

#pragma warning(push)
#pragma warning(disable: 26495)
#pragma warning(disable: 26451)
#pragma warning(disable: 26439)
#pragma warning(disable: 6294)
#pragma warning(disable: 6201)
#pragma warning(disable: 6269)
#include "opencv2/opencv.hpp"
#pragma warning(pop)

/*------------------------
		  CImage

		이미지 클래스
--------------------------*/
enum class eImageFormat
{
	IMAGE_FORMAT_BMP = 1,
	IMAGE_FORMAT_JPG,
	INVALID
};

class CImage
{
public:
	CImage();
	CImage(const CString &path);
	CImage(const CImage& src);

	virtual ~CImage();

	virtual BOOL					Load(const CString& path) = 0;
	virtual BOOL					Load()					  = 0;
	virtual BOOL					Save(const CString& path) = 0;
	virtual CImage*					Copy()					  = 0;

public:
	BOOL							SaveDiffExtension(const CString& path);

	bool							Create(int w, int h, int bpp = 24);
	void							Free();
	bool							GetRawDataFrom(BYTE* src, int w, int h, int srcStride);

	int								GetWidth()		const;
	int								GetHeight()	    const;
	CString							GetPath()		const;
	std::vector<BYTE>				GetBitmapData() const;
	std::vector<BYTE>*				GetBitmapDataRef();

	int								W() const { return m_width; }
	int								H() const { return m_height; }
	int								bpp() const { return m_bpp; }
	int								Stride() const { return m_stride; }

	void							SetBitmapData(const std::vector<BYTE>& data);
	void							SetWidth(const int width);
	void							SetHeight(const int height);
	void							SetPath(const CString& path);
	void							SetImageFormat(const eImageFormat& imgFormat);

	void							Draw(HDC hdc, int x, int y, float zoomScale);

public:
	// 이미지 데이터 변환하는 함수들 클래스 따로 파야하나
	// 일단 여기에 두자.
	void							GetOneDimenData(std::vector<std::vector<BYTE>>& src, std::vector<BYTE>* dest);
	void						    GetTwoDimenBmpData(const std::vector<BYTE>& src, std::vector<std::vector<BYTE>>* dest);
	void						    GetTwoDimenJpgData(const std::vector<BYTE>& src, std::vector<std::vector<BYTE>>* dest);
	void							FlipUpAndDown(std::vector<std::vector<BYTE>>* imageData);
	void							ReverseRgbOrder(std::vector<std::vector<BYTE>>* imageData);

	int								GetBmpStride(int bitCount);
	BITMAPINFOHEADER				CreateBitmapInfoHeader();
	eImageFormat					GetImageFormat() const;

protected:
	CString							m_path;
	int								m_width;
	int								m_height;
	int								m_bpp;
	int								m_stride;
	std::vector<BYTE>				m_bitmapData;
	eImageFormat					m_format;
};

