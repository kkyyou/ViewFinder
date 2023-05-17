#pragma once

class CImage;

namespace MyImageApi
{
	//
	void	Rgb24ToBgr(BYTE* pData, int w, int h, int stride);

	void	FlipUpsideDown(BYTE* pData, int h, int stride);

	bool	CompareImage(CImage& a, CImage& b);
}

