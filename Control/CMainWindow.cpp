#include "CMainWindow.h"
#include "CTopMenuBar.h"
#include "CFilePathBar.h"
#include "CLeftFileBrowserTree.h"
#include "CLeftBottomPreview.h"
#include "CCenterFileBrowserList.h"
#include "CSplitter.h"
#include "CImageViewer.h"

#include "../Define/UiDefine.h"
#include "../Utils/CFontInfo.h"
#include "../Utils/CRegistry.h"
#include "../Utils/CUtils.h"

#include <stdlib.h>
#include <unordered_map>
using namespace std;

CMainWindow::CMainWindow() :
	m_topMenuBar(nullptr),
	m_filePathBar(nullptr),
	m_leftFileBrowserTree(nullptr),
	m_leftBottomPreview(nullptr),
	m_centerFileBrowserList(nullptr),
	m_splitter(nullptr),
	m_imageViewer(nullptr),
	m_mousePosX(0),
	m_pressedCtrl(false),
	m_prevDC(nullptr),
	m_prevHBitmap(nullptr)
{
}

CMainWindow::~CMainWindow()
{
	DeleteChildWindows();
}

LRESULT CMainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//자식 윈도우들 만들기
	if (!CreateChildWindows())
	{
		return 0;
	}

	// 레지스트리 값으로 윈도우 사이즈를 조정
	LoadMainWndSizeToRegister();

	// 레지스트리 값으로 스플리터 위치 조정
	LoadSplitterPosToRegister();

	// 레지스트리 경로로 트리 확장 및 선택
	LoadLeftTreePathToRegister();

	return 0;
}

LRESULT CMainWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CMainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 이전 창 크기 위치 기억한다.
	RECT rcMainWindow;
	GetClientRect(&rcMainWindow);
	ClientToScreen(&rcMainWindow);

	// MainWindow
	DWORD wndPosX = static_cast<DWORD>(rcMainWindow.left);
	CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_DWORD, L"WindowPosX", (const BYTE*)&wndPosX);

	DWORD wndPosY = static_cast<DWORD>(rcMainWindow.top);
	CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_DWORD, L"WindowPosY", (const BYTE*)&wndPosY);
	
	DWORD wndWidth = static_cast<DWORD>(rcMainWindow.right - rcMainWindow.left);
	CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_DWORD, L"WindowWidth", (const BYTE*)&wndWidth);
	
	DWORD wndHeight = static_cast<DWORD>(rcMainWindow.bottom - rcMainWindow.top);
	CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_DWORD, L"WindowHeight", (const BYTE*)&wndHeight);

	// Splitter Pos X
	DWORD splitterPosX = static_cast<DWORD>(m_splitter->GetPosX());
	CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_DWORD, L"SplitterPosX", (const BYTE*)&splitterPosX);

	// 트리 선택 경로
	CString selectedPath = L"";

	HWND hTreeView = m_leftFileBrowserTree->GetHTreeView();
	HTREEITEM hSelectedItem = (HTREEITEM)SendMessage(hTreeView, TVM_GETNEXTITEM, TVGN_CARET, 0);
	if (hSelectedItem != NULL)
	{
		TVITEMEX tvItem = { 0 };
		tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE;
		tvItem.hItem = hSelectedItem;

		SendMessage(hTreeView, TVM_GETITEM, 0, (LPARAM)&tvItem);

		// 해당 트리아이템의 경로를 가져온다.
		selectedPath = reinterpret_cast<TCHAR*>(tvItem.lParam);
		DWORD size = (DWORD)selectedPath.GetLength() * sizeof(TCHAR);
		CRegistry::GetInstance()->SetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, REG_SZ, L"SelectedPath", (const BYTE*)(LPCTSTR)selectedPath, size);
	}

	PostQuitMessage(0);

	// Clean Up
	CFontInfo::GetInstance()->DeleteInstance();
	CRegistry::GetInstance()->DeleteInstance();

	return 0;
}

LRESULT CMainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != SIZE_MINIMIZED)
	{
		::SendMessage(m_topMenuBar->m_hWnd, WM_SIZE, wParam, lParam);
		::SendMessage(m_filePathBar->m_hWnd, WM_SIZE, wParam, lParam);
		::SendMessage(m_leftFileBrowserTree->m_hWnd, WM_SIZE, wParam, lParam);
		::SendMessage(m_leftBottomPreview->m_hWnd, WM_SIZE, wParam, lParam);
		::SendMessage(m_centerFileBrowserList->m_hWnd, WM_SIZE, wParam, lParam);
		::SendMessage(m_imageViewer->m_hWnd, WM_SIZE, wParam, lParam);
	
		// 이미지 크게보기 후 잔상 지우기
		EraseAfterImage();
	}	

	return 0;
}

LRESULT CMainWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_mousePosX = LOWORD(lParam);
	if (m_splitter->IsDrag()) 
	{
		int minPosX = UI_LEFT_FILE_BROWSER_TREE_WIDTH - UI_SPLITTER_WIDTH / 2;
		if (m_mousePosX < minPosX)
			m_mousePosX = minPosX;
		
		RECT rc;
		GetClientRect(&rc);
		InvalidateRect(&rc);

		// 영역 계산
		RECT rcMainWindow;
		GetClientRect(&rcMainWindow);

		// 더블 버퍼링
		HDC hdc = GetDC();
		HDC memDC = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rcMainWindow.right, rcMainWindow.bottom);
		HBITMAP oldHbitmap = (HBITMAP)SelectObject(memDC, hBitmap);

		// 스플리터 시작 전의 화면을 memDC에 그린다.
		BitBlt(memDC, 0, 0, rcMainWindow.right, rcMainWindow.bottom, m_prevDC, 0, 0, SRCCOPY);

		HPEN blackPen = CreatePen(PS_SOLID, 4, RGB(0, 0, 0));
		HPEN oldPen = (HPEN)SelectObject(memDC, blackPen);
		HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
		UINT clientHeight = rcMainWindow.bottom - rcMainWindow.top;

		// memDC에 스플리터 프리뷰 라인을 그려준다.
		MoveToEx(memDC, m_mousePosX, UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT + 1, NULL);
		LineTo(memDC, m_mousePosX, clientHeight);

		// hdc에 옮긴다.
		BitBlt(hdc, 0, 0, rcMainWindow.right, rcMainWindow.bottom, memDC, 0, 0, SRCCOPY);

		// Clean Up
		SelectObject(hdc, oldHbitmap);
		DeleteObject(hBitmap);
		DeleteObject(blackPen);
		DeleteObject(nullBrush);

		ReleaseDC(memDC);
	}

	return 0;
}

LRESULT CMainWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_splitter && m_splitter->IsDrag())
	{
		m_mousePosX = LOWORD(lParam);

		// 스플리터 시작 시점에 화면을 DC에 저장
		RECT rc;
		GetClientRect(&rc);

		HDC hdc = GetDC();

		if (m_prevDC)	   DeleteObject(m_prevDC);
		if (m_prevHBitmap) DeleteObject(m_prevHBitmap);

		m_prevDC = CreateCompatibleDC(hdc);
		m_prevHBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		SelectObject(m_prevDC, m_prevHBitmap);

		// 현재 화면을 m_prevDC에 그려놓는다. 
		BitBlt(m_prevDC, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);
	}
	return 0;
}

LRESULT CMainWindow::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_splitter)
	{
		int x = LOWORD(lParam);
		SetSplitterPosX(x);
		Invalidate();
	}
	return 0;
}

LRESULT CMainWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == VK_CONTROL)
	{
		m_pressedCtrl = true;
	}

	return 0;
}

LRESULT CMainWindow::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == VK_CONTROL)
	{
		m_pressedCtrl = false;
	}

	return 0;
}

LRESULT CMainWindow::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (m_imageViewer && m_imageViewer->IsWindowVisible())
	{
		if (m_imageViewer->IsPressedCtrl())
		{
			// 줌 인 , 줌 아웃
			zDelta > 0 ? m_imageViewer->ZoomIn() : m_imageViewer->ZoomOut();
		}
		else
		{
			// 리스트뷰 핸들
			HWND hListView = m_centerFileBrowserList->GetListViewHandle();

			// 맨처음 (../ 폴더) or 마지막 아이템인경우 첫 아이템으로 지정
			int totalItemCount = ListView_GetItemCount(hListView);
			int selectedItemIdx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

			// 유효한 SelectedIndex 구하기
			GetValidSelectedIndex(zDelta, totalItemCount, &selectedItemIdx);

			// 이전 선택 클리어
			ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);

			// 다음 아이템 선택
			ListView_SetItemState(hListView, selectedItemIdx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_centerFileBrowserList->OnListItemSelected();

			vector<ListViewItemInfo> listViewInfoList = m_centerFileBrowserList->GetListViewItemInfoList();
			if (selectedItemIdx < (int)listViewInfoList.size())
			{
				// RePaint
				m_imageViewer->Invalidate();
				
				CString path = listViewInfoList[selectedItemIdx].path;
				m_filePathBar->SetCurPathToEdit(path);
			}
		}
	}
	else
	{
		if (m_centerFileBrowserList->GetFileViewType() == eFileViewType::VIEW_PREVIEW)
		{
			int value = zDelta > 0 ? THUMNAIL_MIN : -THUMNAIL_MIN;
			int thumnailWidth = m_centerFileBrowserList->GetThumnailWidth();
			int thumnailHeight = m_centerFileBrowserList->GetThumnailHeight();
			int newThumnailWidth = thumnailWidth + value;
			int newThumnailHeight = thumnailHeight + value;

			if (newThumnailWidth < THUMNAIL_MIN || newThumnailHeight < THUMNAIL_MIN)
			{
				m_centerFileBrowserList->SetThumnailWidth(THUMNAIL_MIN);
				m_centerFileBrowserList->SetThumnailHeight(THUMNAIL_MIN);
				return 0;
			}

			if (newThumnailWidth > THUMNAIL_MAX || newThumnailHeight > THUMNAIL_MAX)
			{
				m_centerFileBrowserList->SetThumnailWidth(THUMNAIL_MAX);
				m_centerFileBrowserList->SetThumnailHeight(THUMNAIL_MAX);
				return 0;
			}

			m_centerFileBrowserList->ResizeThumnail(newThumnailWidth, newThumnailHeight);
		}

	}

	return 0;
}

LRESULT CMainWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CMainWindow::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
	pMinMax->ptMinTrackSize.x = UI_MAIN_WINDOW_MIN_WIDTH;
	pMinMax->ptMinTrackSize.y = UI_MAIN_WINDOW_MIN_HEIGHT;
	return 0;
}

void CMainWindow::SetSplitterPosX(int posX)
{
	if (posX < UI_LEFT_FILE_BROWSER_TREE_WIDTH)
		posX = UI_LEFT_FILE_BROWSER_TREE_WIDTH;

	RECT rc;
	GetClientRect(&rc);
	UINT windowWidth = rc.right - rc.left;
	UINT windowHeight = rc.bottom - rc.top;

	UINT treeViewHeight = rc.bottom - rc.top - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	UINT treeViewPosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;

	int changedWidth = posX - UI_SPLITTER_WIDTH;
	m_leftFileBrowserTree->SetWidth(changedWidth);
	m_leftBottomPreview->SetWidth(changedWidth);

	::MoveWindow(m_leftFileBrowserTree->m_hWnd, 0, treeViewPosY, changedWidth, treeViewHeight, TRUE);
	::MoveWindow(m_leftBottomPreview->m_hWnd, 0, treeViewPosY, changedWidth, treeViewHeight, TRUE);
	::MoveWindow(m_splitter->m_hWnd, changedWidth, treeViewPosY, UI_SPLITTER_WIDTH, treeViewPosY, TRUE);
	::MoveWindow(m_centerFileBrowserList->m_hWnd, changedWidth + UI_SPLITTER_WIDTH, treeViewPosY, windowWidth - changedWidth - UI_SPLITTER_WIDTH, treeViewHeight, TRUE);

	ReleaseCapture();
	m_splitter->SetCaptureWnd(NULL);
	m_splitter->SetDragFlag(false);
	m_splitter->SetPosX(posX);
}

void CMainWindow::EraseAfterImage()
{
	RECT rcMainWindow;
	GetClientRect(&rcMainWindow);

	RECT rcRightMargin = rcMainWindow;
	RECT rcSplitterMargin = rcMainWindow;

	rcRightMargin.left = rcRightMargin.right - 10;
	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
	HDC hdc = GetDC();
	FillRect(hdc, &rcRightMargin, brush);

	rcSplitterMargin.top = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
	rcSplitterMargin.left = m_leftFileBrowserTree->GetWidth();
	rcSplitterMargin.right = m_leftFileBrowserTree->GetWidth() + 10;
	FillRect(hdc, &rcSplitterMargin, brush);

	DeleteObject(brush);
}

bool CMainWindow::CreateChildWindows()
{
	RECT rect;
	GetClientRect(&rect);
	LONG clientWidth = rect.right - rect.left;
	LONG clientHeight = rect.bottom - rect.top;

	// 상단 메뉴 바 생성
	m_topMenuBar = new CTopMenuBar();
	RECT rcTopMenuBar = { 0, 0, clientWidth, UI_TOP_MENU_BAR_HEIGHT };
	if (!m_topMenuBar->Create(m_hWnd, rcTopMenuBar, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER))
	{
		return false;
	}
	m_topMenuBar->ShowWindow(SW_SHOWNORMAL);
	m_topMenuBar->UpdateWindow();

	// 파일 경로 바 생성
	m_filePathBar = new CFilePathBar();
	RECT rcFilePathBar = { 0, UI_TOP_MENU_BAR_HEIGHT, clientWidth, UI_FILE_PATH_BAR_HEIGHT };
	if (!m_filePathBar->Create(m_hWnd, rcFilePathBar, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN)))
	{
		return false;
	}
	m_filePathBar->ShowWindow(SW_SHOWNORMAL);
	m_filePathBar->UpdateWindow();

	// 좌측 파일 브라우저 트리
	m_leftFileBrowserTree = new CLeftFileBrowserTree();
	LONG leftBrowserTreeHeight = clientHeight - UI_LEFT_DOWN_PREVIEW_HEIGHT - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	LONG leftBrowserTreePosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
	RECT rcLeftFileBrowserTree = { 0, leftBrowserTreePosY, UI_LEFT_FILE_BROWSER_TREE_WIDTH, leftBrowserTreeHeight };
	if (!m_leftFileBrowserTree->Create(m_hWnd, rcLeftFileBrowserTree, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER)))
	{
		return false;
	}
	m_leftFileBrowserTree->ShowWindow(SW_SHOWNORMAL);
	m_leftFileBrowserTree->UpdateWindow();

	// 좌측 하단 프리뷰
	m_leftBottomPreview = new CLeftBottomPreview();
	LONG leftDownPreviewPosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT + leftBrowserTreeHeight;
	RECT rcLeftDownPreview = { 0, leftDownPreviewPosY, UI_LEFT_FILE_BROWSER_TREE_WIDTH, UI_LEFT_DOWN_PREVIEW_HEIGHT };
	if (!m_leftBottomPreview->Create(m_hWnd, rcLeftDownPreview, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER)))
	{
		return false;
	}
	m_leftBottomPreview->ShowWindow(SW_SHOWNORMAL);
	m_leftBottomPreview->UpdateWindow();

	// 중앙 파일 브라우저 리스트뷰
	m_centerFileBrowserList = new CCenterFileBrowserList();
	m_centerFileBrowserList->SetFilePathBar(m_filePathBar);
	m_centerFileBrowserList->SetLeftFileBrowserTree(m_leftFileBrowserTree);
	LONG centerListViewWidth = clientWidth - UI_LEFT_FILE_BROWSER_TREE_WIDTH;
	LONG centerListViewHeight = clientHeight - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	RECT rcCenterListView = { UI_LEFT_FILE_BROWSER_TREE_WIDTH + UI_SPLITTER_WIDTH, leftBrowserTreePosY, centerListViewWidth, centerListViewHeight };
	if (!m_centerFileBrowserList->Create(m_hWnd, rcCenterListView, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER)))
	{
		return false;
	}
	m_centerFileBrowserList->ShowWindow(SW_SHOWNORMAL);
	m_centerFileBrowserList->UpdateWindow();
	m_centerFileBrowserList->SetChangedColumnCntWidth(centerListViewWidth);

	// 트리뷰 / 리스트뷰 스플리터
	m_splitter = new CSplitter(m_leftFileBrowserTree, m_centerFileBrowserList);
	RECT rcSplitter = { UI_LEFT_FILE_BROWSER_TREE_WIDTH - UI_SPLITTER_WIDTH / 2, leftBrowserTreePosY, UI_SPLITTER_WIDTH, centerListViewHeight };
	if (!m_splitter->Create(m_hWnd, rcSplitter, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN)))
	{
		return false;
	}
	m_splitter->ShowWindow(SW_SHOWNORMAL);
	m_splitter->UpdateWindow();

	// 이미지 뷰어
	m_imageViewer = new CImageViewer();
	RECT rcImageViewer = { 0, leftBrowserTreePosY, clientWidth, centerListViewHeight };
	if (!m_imageViewer->Create(m_hWnd, rcImageViewer, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER)))
	{
		return false;
	}
	m_imageViewer->ShowWindow(SW_HIDE);
	m_imageViewer->UpdateWindow();

	// 각 클래스간 메모리 공유
	m_topMenuBar->SetCenterFileBrowserList(m_centerFileBrowserList);
	m_topMenuBar->SetLeftFileBrowserTree(m_leftFileBrowserTree);
	m_topMenuBar->SetLeftBottomPreview(m_leftBottomPreview);
	m_topMenuBar->SetImageViewer(m_imageViewer);
	m_topMenuBar->SetFilePathBar(m_filePathBar);
	m_topMenuBar->SetMainWindow(this);

	m_leftFileBrowserTree->SetMainView(m_centerFileBrowserList);
	m_leftFileBrowserTree->SetFilePathBar(m_filePathBar);

	m_filePathBar->SetLeftFileBrowserTree(m_leftFileBrowserTree);
	m_filePathBar->SetMainView(m_centerFileBrowserList);

	m_centerFileBrowserList->SetImageViewer(m_imageViewer);
	m_centerFileBrowserList->SetLeftBottomPreview(m_leftBottomPreview);
	m_centerFileBrowserList->SetTopMenuBar(m_topMenuBar);
	m_centerFileBrowserList->SetMainWindow(this);

	m_leftBottomPreview->SetCenterFileBrowserList(m_centerFileBrowserList);
	m_leftBottomPreview->SetImageViewer(m_imageViewer);

	return true;
}

void CMainWindow::DeleteChildWindows()
{
	delete m_topMenuBar;
	delete m_filePathBar;
	delete m_leftFileBrowserTree;
	delete m_leftBottomPreview;
	delete m_centerFileBrowserList;
	delete m_splitter;
	delete m_imageViewer;
}

void CMainWindow::LoadMainWndSizeToRegister()
{
	RECT rcDefault;
	GetClientRect(&rcDefault);
	ClientToScreen(&rcDefault);

	LONG windowWidth = 0, windowHeight = 0, windowPosX = 0, windowPosY = 0;

	if (!CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"WindowWidth", (LPBYTE)&windowWidth))
	{
		windowWidth = rcDefault.right - rcDefault.left;
	}

	if (!CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"WindowHeight", (LPBYTE)&windowHeight))
	{
		windowHeight = rcDefault.bottom - rcDefault.top;
	}

	if (!CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"WindowPosX", (LPBYTE)&windowPosX))
	{
		windowPosX = rcDefault.left;
	}

	if (!CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"WindowPosY", (LPBYTE)&windowPosY))
	{
		windowPosY = rcDefault.top;
	}

	// 윈도우 사이즈 변경
	RECT rcMainWindow{ windowPosX, windowPosY, windowPosX + windowWidth, windowPosY + windowHeight };
	MoveWindow(&rcMainWindow);
}

void CMainWindow::LoadSplitterPosToRegister()
{
	LONG splitterPosX = 0;

	// 스플리터 시작 위치
	if (!CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"SplitterPosX", (LPBYTE)&splitterPosX))
	{
		splitterPosX = UI_LEFT_FILE_BROWSER_TREE_WIDTH;
	}

	// 스플리터 위치 변경
	SetSplitterPosX(splitterPosX);
}

void CMainWindow::LoadLeftTreePathToRegister()
{
	// 종료전 선택한 경로
	TCHAR* selectedTreePath = new TCHAR[MAX_PATH];
	if (CRegistry::GetInstance()->GetValue(HKEY_CURRENT_USER, REGISTRY_PATH_VIEWFINDER, L"SelectedPath", (LPBYTE)selectedTreePath))
	{
		// ex) C:\users\kykim\OneDrive\바탕화면
		CString selectedPath = selectedTreePath;
		string stdSelectedPath(CW2A(selectedPath.GetString()));

		unordered_map<string, HTREEITEM> treeViewItemMap = m_leftFileBrowserTree->GetTreeItemMap();

		// \로 스플릿해서 최상위 폴더부터 마지막까지 EXPAND한다.
		int start = 0;
		CString token = selectedPath.Tokenize(L"\\", start);
		CString subPath = token;
		while (token.Compare(L"") != 0)
		{
			string stdSubPath(CW2A(subPath.GetString()));

			// 서브 경로 트리아이템을 EXPAND
			treeViewItemMap = m_leftFileBrowserTree->GetTreeItemMap();
			HTREEITEM hItem = treeViewItemMap[stdSubPath];
			if (hItem)      
			{
				// 현재 경로의 내부 파일 경로를 찾는다.
				vector<CString> innerPaths = m_leftFileBrowserTree->GetInnerFilePaths(subPath);

				// 내부 파일 경로를 돌면서 아이템을 추가한다.
				for (CString innerPath : innerPaths)
				{
					string stdInnerPath(CW2A(innerPath.GetString()));

					// 내부파일의 HTREEITEM을 찾는다. (추가해야 할 아이템의 부모 아이템)
					treeViewItemMap = m_leftFileBrowserTree->GetTreeItemMap();
					HTREEITEM hItem = treeViewItemMap[stdInnerPath];

					// 채워넣는다.
					m_leftFileBrowserTree->PopulateTreeView(innerPath, hItem);
				}

				TreeView_Expand(m_leftFileBrowserTree->GetHTreeView(), hItem, TVE_EXPAND);  
			}

			token = selectedPath.Tokenize(L"\\", start);
			subPath += L"\\";
			subPath += token;
		}

		// 선택
		m_leftFileBrowserTree->SelectTreeItem(stdSelectedPath);
	}
	delete[] selectedTreePath;
}

void CMainWindow::GetValidSelectedIndex(int zDelta, int totalItemCount, int* selectedItemIdx)
{
	if (!selectedItemIdx)
		return;

	// 다음 이미지
	if (zDelta > 0)
	{
		if (*selectedItemIdx == totalItemCount - 1)
			*selectedItemIdx = 1;
		else
			(*selectedItemIdx)++;
	}
	// 이전 이미지
	else
	{
		if (*selectedItemIdx == 1)
			*selectedItemIdx = totalItemCount - 1;
		else
			(*selectedItemIdx)--;
	}
	HWND hListView = m_centerFileBrowserList->GetListViewHandle();
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = *selectedItemIdx;
	ListView_GetItem(hListView, &lvItem);

	TCHAR* pszPath = reinterpret_cast<TCHAR*>(lvItem.lParam);
	CString path = pszPath;
	CUtils::RemoveDriveSymbol(&path);

	// 폴더인 경우
	if (PathIsDirectory(path))
	{
		// 유효한 이미지 파일 Index를 구할때까지 재귀호출한다.
		GetValidSelectedIndex(zDelta, totalItemCount, selectedItemIdx);
	}
}
