#include "CFilePathBar.h"

#include "../Define/UiDefine.h"
#include "../resource.h"
#include "../Utils/CFontInfo.h"
#include "../Define/StringDefine.h"
#include "../Utils/CUtils.h"

#include "CLeftFileBrowserTree.h"
#include "CCenterFileBrowserList.h"

#define STATIC_ADDR_POS_X	20
#define STATIC_ADDR_POS_Y	15
#define STATIC_ADDR_WIDTH	50
#define STATIC_ADDR_HEIGHT	20
#define EDIT_POS_X			80
#define EDIT_POS_Y			13
#define EDIT_WIDTH			400
#define EDIT_HEIGHT			20
#define BTN_MOVE_POS_Y		10
#define BTN_MOVE_HEIGHT		25

CFilePathBar::CFilePathBar() :
	m_centerFileBrowserList(nullptr),
	m_leftFileBrowserTree(nullptr),
	m_hEdit(nullptr)
{
}

LRESULT CFilePathBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// STATIC 라벨 생성
	HWND sttAddr = CreateWindow(L"STATIC", L"Address", WS_CHILD | WS_VISIBLE | SS_CENTER, 
		STATIC_ADDR_POS_X, STATIC_ADDR_POS_Y, STATIC_ADDR_WIDTH, STATIC_ADDR_HEIGHT,
		m_hWnd, (HMENU)IDC_STATIC, _AtlBaseModule.GetModuleInstance(), NULL);
	::SetWindowText(sttAddr, L"주소 : ");

	// EDIT 생성
	m_hEdit = CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		EDIT_POS_X, EDIT_POS_Y, EDIT_WIDTH, EDIT_HEIGHT, m_hWnd, 
		NULL, _AtlBaseModule.GetModuleInstance(), NULL);
	CFontInfo* fontInfo = CFontInfo::GetInstance();
	SendMessage(m_hEdit, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));

	// 이동 버튼 생성
	HWND moveBtn = CreateWindow(L"BUTTON", L"이동", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
		EDIT_POS_X + EDIT_WIDTH + 10, BTN_MOVE_POS_Y, UI_SIZE_40PX, BTN_MOVE_HEIGHT, m_hWnd,
		(HMENU)IDC_MOVE_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	SendMessage(moveBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));

	// PROP 저장
	SetProp(m_hEdit, L"CFilePathBar", this);
	
	// EDIT 서브클래싱
	SubclassEditWindow(m_hEdit);
	
	return 0;
}

// Edit 서브클래싱
LRESULT CALLBACK EditWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) 
{
	switch (uMsg) 
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) 
		{
			HWND hEdit = hwnd;
			int len = ::GetWindowTextLength(hEdit);
			if (len > 0)
			{
				TCHAR* path = new TCHAR[len + 1];
				::GetWindowText(hEdit, path, len + 1);
				
				if (CFilePathBar* filePathBar = (CFilePathBar*)GetProp(hwnd, L"CFilePathBar"))
				{
					filePathBar->OnMoveBtnClickd(path);
				}
				
				delete[] path;
			}
			return 0;
		}
		break;
		
	default:
		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}


LRESULT CFilePathBar::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CFilePathBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFilePathBar::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (LOWORD(wParam))
	{
	case IDC_MOVE_BTN:
	{
		int len = ::GetWindowTextLength(m_hEdit);
		if (len > 0)
		{
			TCHAR* path = new TCHAR[len + 1];
			::GetWindowText(m_hEdit, path, len + 1);
			OnMoveBtnClickd(path);
			delete[] path;
		}
		break;
	}
	default: break;
	}
	
	return 0;
}

LRESULT CFilePathBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetParent().GetClientRect(&rect);
	MoveWindow(0, UI_TOP_MENU_BAR_HEIGHT, rect.right - rect.left, UI_FILE_PATH_BAR_HEIGHT, TRUE);

	return 0;
}

void CFilePathBar::OnMoveBtnClickd(const CString& path)
{
	// 유효하지 않은 파일 경로
	if (!CUtils::IsValidFilePath(path))
	{
		MessageBox(STR_INVALID_FILE_PATH);
		return;
	}

	if (m_leftFileBrowserTree)
	{

	}

	if (m_centerFileBrowserList)
	{
		m_centerFileBrowserList->PopulateListView(path);
	}
}

void CFilePathBar::SetCurPathToEdit(const CString& path)
{
	// 경로로 폴더 검색을 할 때는 드라이브 심볼을 지운다.
	CString copyPath = path;
	CUtils::RemoveDriveSymbol(&copyPath);

	::SetWindowText(m_hEdit, copyPath);
}

void CFilePathBar::SubclassEditWindow(HWND hEdit)
{
	SetWindowSubclass(hEdit, EditWindowProc, 0, 0);
}

void CFilePathBar::SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserTree)
{
	m_leftFileBrowserTree = leftFileBrowserTree;
}

void CFilePathBar::SetMainView(CCenterFileBrowserList* centerFileBrowserList)
{
	m_centerFileBrowserList = centerFileBrowserList;
}

CLeftFileBrowserTree* CFilePathBar::GetLeftFileBrowserTree() const
{
	return m_leftFileBrowserTree;
}

CCenterFileBrowserList* CFilePathBar::GetMainView() const
{
	return m_centerFileBrowserList;
}

CString CFilePathBar::GetCurrentPath()
{
	CString curPath;
	int len = ::GetWindowTextLength(m_hEdit);
	if (len > 0)
	{
		TCHAR* path = new TCHAR[len + 1];
		::GetWindowText(m_hEdit, path, len + 1);
		curPath = path;

		delete[] path;
	}

	return curPath;
}

CString CFilePathBar::GetPrevPath()
{
	CString prevPath = GetCurrentPath();
	int index = prevPath.ReverseFind('\\');
	if (index != -1)
	{
		prevPath.Delete(index, prevPath.GetLength() - index);
	}

	return prevPath;
}
