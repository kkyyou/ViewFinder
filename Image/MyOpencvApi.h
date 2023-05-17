#pragma once

#pragma warning(push)
#pragma warning(disable: 26495)
#pragma warning(disable: 26451)
#pragma warning(disable: 26439)
#pragma warning(disable: 6294)
#pragma warning(disable: 6201)
#pragma warning(disable: 6269)
#include "opencv2/opencv.hpp"
#pragma warning(pop)

#include "CImage.h"

namespace MyOpencvApi
{
	using namespace cv;

	Mat			ToCvMat(CImage& img);
	CImage*		CreateFromCvMat(const Mat& mat, const CString& path);

}