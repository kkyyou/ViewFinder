#include "MyOpencvApi.h"
#include "CImageFactory.h"

#include "../Utils/CUtils.h"

namespace MyOpencvApi
{
	Mat	ToCvMat(CImage& img)
	{
		int type = CV_8UC1;
		switch (img.bpp())
		{
		case 8:		type = CV_8UC1; break;
		case 24:	type = CV_8UC3; break;
		case 32:	type = CV_8UC4; break;
		}

		Mat mat(img.H(), img.W(), type);

		std::vector<BYTE>* v = img.GetBitmapDataRef();
		int bpp = img.bpp();
		int w = img.W();
		int h = img.H();

		int srcStride = img.Stride();
		int dstStride = w * 3;

		// data copy
		BYTE* src = img.GetBitmapDataRef()->data();
		BYTE* dst = mat.data;
		for (int y = 0; y < h; y++)
		{
			memcpy(dst, src, dstStride);
			src += srcStride;
			dst += dstStride;
		}

		// 위 아래 뒤집기
		cv::flip(mat, mat, 0);

		return mat;
	}

	CImage* CreateFromCvMat(const Mat& mat, const CString& path)
	{
		CImage* img = CImageFactory::Create(path);
		if (!img)	return nullptr;

		int bpp;
		switch (mat.type())
		{
		case CV_8UC1:	bpp = 8;	break;
		case CV_8UC4:	bpp = 32;	break;
		default:
			bpp = 24;
		}
		int w = mat.cols;
		int h = mat.rows;

		img->Create(w, h, bpp);
		img->SetPath(path);
		int dstStride = img->Stride();
		int srcStride = w * 3;

		// 위 아래 뒤집기
		cv::flip(mat, mat, 0);

		// data copy
		BYTE* src = mat.data;
		BYTE* dst = img->GetBitmapDataRef()->data();
		for (int y = 0; y < h; y++)
		{
			memcpy(dst, src, srcStride);
			src += srcStride;
			dst += dstStride;
		}

		return img;
	}

}