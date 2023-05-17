#include "CPalette.h"

#include "../Define/UiDefine.h"

CPalette::CPalette() :
	m_curColorRef(COLORREF())
{
}

LRESULT CPalette::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_palette = { 
		RGB(255, 0, 0),     RGB(0, 255, 0),   RGB(0, 0, 255),
		RGB(255, 255, 0),   RGB(255, 0, 255), RGB(0, 255, 255), 
		RGB(255, 255, 255), RGB(0, 0, 0) 
	};

	return 0;
}

LRESULT CPalette::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);

	for (size_t i = 0; i < m_palette.size(); i++)
	{
		RECT rect = { (LONG)i * 20, 0, (LONG)(i + 1) * 20, 20 };
		HBRUSH hBrush = CreateSolidBrush(m_palette[i]);
		FillRect(hdc, &rect, hBrush);
		DeleteObject(hBrush);
	}

	EndPaint(&ps);
	return 0;
}

LRESULT CPalette::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CPalette::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rcImageViewer;
	GetParent().GetClientRect(&rcImageViewer);

	RECT rcPalette{ rcImageViewer.right - UI_PALETTE_WIDTH - 5, rcImageViewer.top + 5, rcImageViewer.right - 5, UI_PALETTE_HEIGHT + 5 };
	MoveWindow(&rcPalette);

	return 0;
}

LRESULT CPalette::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	m_curColorRef = GetPixel(GetDC(), x, y);

	return 0;
}

LRESULT CPalette::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetCursor(LoadCursor(NULL, IDC_HAND));
	return 0;
}

COLORREF CPalette::GetCurrentColorRef() const
{
	return m_curColorRef;
}
