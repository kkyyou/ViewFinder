#include "CLeftBottomPreview.h"
#include "CImageViewer.h"
#include "CCenterFileBrowserList.h"

#include "../Image/CImage.h"
#include "../Define/UiDefine.h"

CLeftBottomPreview::CLeftBottomPreview() :
	m_width(UI_LEFT_FILE_BROWSER_TREE_WIDTH - (UI_SPLITTER_WIDTH / 2)),
	m_height(UI_LEFT_DOWN_PREVIEW_HEIGHT),
	m_centerFileBrowserList(nullptr),
	m_imageViewer(nullptr),
	m_imgStartX(0),
	m_imgStartY(0),
	m_scale(1)
{
}

LRESULT CLeftBottomPreview::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CLeftBottomPreview::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);

	if (m_imageViewer)
	{
		CImage* image = m_imageViewer->GetImage();
		if (!image || image->GetPath().IsEmpty())
		{
			EndPaint(&ps);
			return 0;
		}
		
		image->Draw(hdc, m_imgStartX, m_imgStartY, m_scale);
	}

	EndPaint(&ps);
	return 0;
}

LRESULT CLeftBottomPreview::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CLeftBottomPreview::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 바텀 프리뷰 사이즈
	RECT rect;
	GetParent().GetClientRect(&rect);
	UINT clientWidth = rect.right - rect.left;
	UINT clientHeight = rect.bottom - rect.top;
	UINT leftBrowserTreeHeight = clientHeight - UI_LEFT_DOWN_PREVIEW_HEIGHT - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	UINT leftDownPreviewPosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT + leftBrowserTreeHeight;
	MoveWindow(0, leftDownPreviewPosY, m_width, UI_LEFT_DOWN_PREVIEW_HEIGHT, TRUE);

	StartPreview();
	return 0;      
}

void CLeftBottomPreview::SetWidth(const UINT width)
{
	m_width = width;
}

void CLeftBottomPreview::SetCenterFileBrowserList(CCenterFileBrowserList* centerFileBrowserList)
{
	m_centerFileBrowserList = centerFileBrowserList;
}

void CLeftBottomPreview::SetImageViewer(CImageViewer* imageViewer)
{
	m_imageViewer = imageViewer;
}

void CLeftBottomPreview::StartPreview()
{
	// 이미지 그리기 시작 점, 너비, 높이, 비율 계산
	if (m_imageViewer)
	{
		CImage* image = m_imageViewer->GetImage();
		if (image && !image->GetPath().IsEmpty())
		{
			UINT centerX = m_width / 2;
			UINT centerY = m_height / 2;

			int imgWidth = image->GetWidth();
			int imgHeight = image->GetHeight();

			float scaleWidth = (float)m_width / (float)imgWidth;
			float scaleHeight = (float)m_height / (float)imgHeight;
			m_scale = MIN(scaleWidth, scaleHeight);

			if (m_scale < 1)
			{
				m_imgStartX = centerX - (UINT)(imgWidth * m_scale / 2);
				m_imgStartY = centerY - (UINT)(imgHeight * m_scale / 2);
			}
			else
			{
				m_imgStartX = centerX - (UINT)(imgWidth / 2);
				m_imgStartY = centerY - (UINT)(imgHeight / 2);
				m_scale = 1;
			}
		}
	}

	Invalidate();
}
