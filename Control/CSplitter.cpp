#include "CSplitter.h"
#include "CLeftFileBrowserTree.h"
#include "CCenterFileBrowserList.h"

#include "../Define/UiDefine.h"

CSplitter::CSplitter(CLeftFileBrowserTree* p1, CCenterFileBrowserList* p2) :
	m_leftPlane(p1),
	m_rightPlane(p2),
	m_isDrag(false),
	m_hCaptureWnd(nullptr),
	m_posX(UI_LEFT_FILE_BROWSER_TREE_WIDTH)
{
}

CSplitter::~CSplitter()
{
}

LRESULT CSplitter::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSplitter::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CSplitter::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSplitter::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetParent().GetClientRect(&rc);
	UINT treeViewHeight = rc.bottom - rc.top - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	//::MoveWindow(m_hWnd, m_leftPlane->GetWidth(), UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT, UI_SPLITTER_WIDTH, treeViewHeight, FALSE);
	::MoveWindow(m_hWnd, m_leftPlane->GetWidth(), UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT, UI_SPLITTER_WIDTH, treeViewHeight, TRUE);

	return 0;
}

LRESULT CSplitter::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 드래그 상태
	if (m_isDrag)
	{
		// 스플리터에서의 X좌표
		int x = LOWORD(lParam);

		// 메인위도우 에서의 X좌표
		int mainPosX = x + m_leftPlane->GetWidth();

		if (mainPosX != m_oldPosX)
		{
			// 부모윈도우에 메시지 전달
			SendMessage(m_hCaptureWnd, uMsg, wParam, (LPARAM)mainPosX);
			//::PostMessage(m_hCaptureWnd, uMsg, wParam, (LPARAM)mainPosX);
			m_oldPosX = mainPosX;
		}

		return 0;
	}

	return 0;
}

LRESULT CSplitter::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hCaptureWnd == NULL)
	{
		HWND hParentWnd = GetParent();

		// 커서가 윈도우 영역을 벗어나도 계속해서 마우스 메시지를 줌
		SetCapture();
		m_hCaptureWnd = hParentWnd;
		m_isDrag = true;
	}

	// 스플리터 이동 시작할겁니다를 MainWindow에 알려주자.
	SendMessage(GetParent(), uMsg, wParam, lParam);
	return 0;
}

LRESULT CSplitter::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hCaptureWnd)
	{
		// 스플리터에서의 X좌표
		int x = LOWORD(lParam);

		// 메인위도우 에서의 X좌표
		int mainPosX = x + m_leftPlane->GetWidth();

		// 부모윈도우에 메시지 전달
		SendMessage(m_hCaptureWnd, uMsg, wParam, (LPARAM)mainPosX);

		m_hCaptureWnd = NULL;
		m_isDrag = false;

		ReleaseCapture();
	}
	return 0;
}

LRESULT CSplitter::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	return 0;
}

bool CSplitter::IsDrag() const
{
	return m_isDrag;
}

void CSplitter::SetDragFlag(bool isDrag)
{
	m_isDrag = isDrag;
}

void CSplitter::SetCaptureWnd(HWND captureWnd)
{
	m_hCaptureWnd = captureWnd;
}

void CSplitter::SetPosX(int posX)
{
	m_posX = posX;
}

int CSplitter::GetPosX() const
{
	return m_posX;
}



