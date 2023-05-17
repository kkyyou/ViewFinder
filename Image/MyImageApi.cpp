#include "CImage.h"
#include "MyImageApi.h"



namespace MyImageApi
{
	// RGB를 BGR 순서로 변환한다.
	void Rgb24ToBgr(BYTE* pData, int w, int h, int stride)
	{
		BYTE t;
		for (int y = 0; y < h; y++)
		{
			BYTE* p = pData + (INT64)stride * y;
			for (int x = 0; x < w; x++)
			{
				t = *(p + 2);
				*(p + 2) = *p;
				*p = t;

				p += 3;
			}
		}
	}

	// 위 아래를 뒤집는다.
	void FlipUpsideDown(BYTE* pData, int h, int stride)
	{
		std::vector<BYTE> buf(stride);

		int h2 = h / 2;
		for (int y = 0; y < h2; y++)
		{
			BYTE* p = pData + (INT64)stride * y;
			BYTE* q = pData + (INT64)stride * (h - 1 - y);
			{
				memcpy(buf.data(), q, stride);
				memcpy(q, p, stride);
				memcpy(p, buf.data(), stride);
				p += stride;
				q -= stride;
			}
		}
	}

	bool CompareImage(CImage& a, CImage& b)
	{
		if (a.W() != b.W()) return false;
		if (a.H() != b.H()) return false;
		if (a.Stride() != b.Stride()) return false;

		const int size = (a.W() * a.bpp()) / 8;
		const int stride = a.Stride();
		const int h = a.H();
		for (int y = 0; y < h; y++)
		{
			BYTE* p = a.GetBitmapData().data() + (INT64)a.Stride() * y;
			BYTE* q = b.GetBitmapData().data() + (INT64)b.Stride() * y;
			{
				if (0 != memcmp(q, p, size)) return false;

				p += stride;
				q += stride;
			}
		}

		return true;
	}

}
