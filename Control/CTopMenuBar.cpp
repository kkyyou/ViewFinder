#include "CTopMenuBar.h"
#include "CImageViewer.h"
#include "CCenterFileBrowserList.h"
#include "CLeftBottomPreview.h"
#include "CLeftFileBrowserTree.h"
#include "CFilePathBar.h"
#include "CMainWindow.h"

#include "../Utils/CFileDialog.h"
#include "../Utils/CFontInfo.h"
#include "../resource.h"
#include "../Define/UiDefine.h"
#include "../resource.h"

CTopMenuBar::CTopMenuBar() :
	m_centerFileBrowserList(nullptr),
	m_imageViewer(nullptr),
	m_leftFileBrowserTree(nullptr),
	m_leftBottomPreview(nullptr),
	m_filePathBar(nullptr),
	m_mainWindow(nullptr)
{
}

CTopMenuBar::~CTopMenuBar()
{
}

LRESULT CTopMenuBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CFontInfo* fontInfo = CFontInfo::GetInstance();

	int iPadding = UI_PADDING_5PX;

	// MENU
	//HWND fileBtn = CreateWindow(L"BUTTON", L"파일", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	//	iPadding, UI_PADDING_1PX, UI_SIZE_50PX, UI_SIZE_25PX, m_hWnd,
	//	(HMENU)IDC_FILE_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	//SendMessage(fileBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));

	//iPadding += (UI_SIZE_50PX + UI_PADDING_1PX);
	//HWND homeBtn = CreateWindow(L"BUTTON", L"홈", WS_TABSTOP | WS_CHILD | WS_VISIBLE,
	//	iPadding, UI_PADDING_1PX, UI_SIZE_50PX, UI_SIZE_25PX, m_hWnd,
	//	(HMENU)IDC_HOME_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	//SendMessage(homeBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));

	// 파일메뉴 종료 버튼
	//iPadding = UI_PADDING_5PX;
	//HWND exitBtn = CreateWindow(L"BUTTON", L"종료", WS_CHILD | BS_PUSHBUTTON | BS_BITMAP,
	//	iPadding, UI_SIZE_35PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
	//	(HMENU)IDC_EXIT_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	//m_vFileBtnIds.push_back(IDC_EXIT_BTN);
	//SendMessage(exitBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	//HANDLE exitHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_EXIT_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	//SendMessage(exitBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)exitHBitmap);

	// 홈메뉴 크게보기 버튼
	HWND viewLargerBtn = CreateWindow(L"BUTTON", L"크게보기", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | BS_BITMAP,
		iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
		(HMENU)IDC_VIEW_LARGER_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	m_vHomeBtnIds.push_back(IDC_VIEW_LARGER_BTN);
	SendMessage(viewLargerBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	HANDLE LargerViewHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_LARGEVIEW_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	SendMessage(viewLargerBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)LargerViewHBitmap);

	// 홈메뉴 아이콘 보기 버튼
	iPadding += (UI_SIZE_70PX + UI_PADDING_10PX);
	HWND viewIconBtn = CreateWindow(L"BUTTON", L"아이콘", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | BS_BITMAP,
		iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
		(HMENU)IDC_ICONVIEW_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	m_vHomeBtnIds.push_back(IDC_ICONVIEW_BTN);
	SendMessage(viewIconBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	HANDLE iconViewHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_ICON_VIEW_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	SendMessage(viewIconBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)iconViewHBitmap);

	// 홈메뉴 미리보기 버튼
	iPadding += (UI_SIZE_70PX + UI_PADDING_10PX);
	HWND previewBtn = CreateWindow(L"BUTTON", L"미리보기", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | BS_BITMAP,
		iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
		(HMENU)IDC_PREVIEW_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	m_vHomeBtnIds.push_back(IDC_PREVIEW_BTN);
	SendMessage(previewBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	HANDLE previewHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_PREVIEW_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	SendMessage(previewBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)previewHBitmap);

	// 홈메뉴 목록보기 버튼
	iPadding = UI_PADDING_5PX;
	HWND viewListBtn = CreateWindow(L"BUTTON", L"목록보기", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | BS_BITMAP,
		iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
		(HMENU)IDC_VIEW_LIST_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	SendMessage(viewListBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	::ShowWindow(viewListBtn, SW_HIDE);
	m_vHomeBtnIds.push_back(IDC_VIEW_LIST_BTN);
	HANDLE viewListHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_VIEW_LIST_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	SendMessage(viewListBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)viewListHBitmap);

	// 편집하기 버튼
	//iPadding += (UI_SIZE_70PX + UI_PADDING_10PX);
	//HWND editBtn = CreateWindow(L"BUTTON", L"편집하기", WS_CHILD | BS_PUSHBUTTON | BS_BITMAP,
	//	iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
	//	(HMENU)IDC_EDIT_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	//m_vHomeBtnIds.push_back(IDC_EDIT_BTN);
	//SendMessage(editBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	//HANDLE editHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_EDIT_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	//SendMessage(editBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)editHBitmap);

	//// 저장하기 버튼
	//iPadding += (UI_SIZE_70PX + UI_PADDING_10PX);
	//HWND saveBtn = CreateWindow(L"BUTTON", L"저장하기", WS_CHILD | BS_PUSHBUTTON | BS_BITMAP,
	//	iPadding, UI_SIZE_25PX, UI_SIZE_70PX, UI_SIZE_70PX, m_hWnd,
	//	(HMENU)IDC_SAVE_BTN, _AtlBaseModule.GetModuleInstance(), NULL);
	//m_vHomeBtnIds.push_back(IDC_SAVE_BTN);
	//SendMessage(saveBtn, WM_SETFONT, (WPARAM)fontInfo->GetDefaultFont(), MAKELPARAM(TRUE, 0));
	//HANDLE saveHBitmap = LoadImage(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDB_SAVE_BMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	//SendMessage(saveBtn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)saveHBitmap);

	return 0;
}

LRESULT CTopMenuBar::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CTopMenuBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DeleteObject(GetDlgItem(IDC_FILE_BTN));
	DeleteObject(GetDlgItem(IDC_HOME_BTN));
	DeleteObject(GetDlgItem(IDC_VIEW_LIST_BTN));
	for (UINT btnId : m_vFileBtnIds) DeleteObject(GetDlgItem(btnId));
	for (UINT btnId : m_vHomeBtnIds) DeleteObject(GetDlgItem(btnId));
	return 0;
}

LRESULT CTopMenuBar::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (LOWORD(wParam))
	{
	case IDC_FILE_BTN:			OnFileBtnClicked();			break;
	case IDC_HOME_BTN:			OnHomeBtnClicked();			break;
	case IDC_VIEW_LARGER_BTN:   OnViewLargerBtnClicked();	break;
	case IDC_ICONVIEW_BTN:	    OnIconViewBtnClicked();		break;
	case IDC_PREVIEW_BTN:		OnPreviewBtnClicked();		break;
	case IDC_VIEW_LIST_BTN:		OnViewListBtnClicked();		break;
	case IDC_EXIT_BTN:			OnExitBtnClicked();			break;
	case IDC_EDIT_BTN:			OnEditBtnClicked();			break;
	case IDC_SAVE_BTN:			OnSaveBtnClicked();			break;
	default:												break;
	}
	return 0;
}

LRESULT CTopMenuBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetParent().GetClientRect(&rect);
	UINT clientWidth = rect.right - rect.left;
	MoveWindow(0, 0, clientWidth - UI_SPLITTER_WIDTH, UI_TOP_MENU_BAR_HEIGHT, TRUE);

	return 0;
}

// 파일 버튼을 클릭하면 파일관련 버튼만 보이도록 한다.
void CTopMenuBar::OnFileBtnClicked()
{
	SetVisibleHomeFuncBtns(false);
	SetVisibleFileFuncBtns(true);
}

// 홈 버튼을 클릭하면 홈관련 버튼만 보이도록 한다.
void CTopMenuBar::OnHomeBtnClicked()
{
	SetVisibleFileFuncBtns(false);
	SetVisibleHomeFuncBtns(true);
}

void CTopMenuBar::OnViewLargerBtnClicked()
{
	CString selectedPath = m_centerFileBrowserList->GetSelectedItemPath();
	if (selectedPath.IsEmpty())
		return;

	m_filePathBar->SetCurPathToEdit(selectedPath);

	m_centerFileBrowserList->ShowViewLargeImageViewer();

	m_mainWindow->ShowSplitter(false);

	// 크게보기 숨기기, 목록보기 보이기
	HideViewLargerBtn();
}

void CTopMenuBar::OnIconViewBtnClicked()
{
	eFileViewType curFileViewType = m_centerFileBrowserList->GetFileViewType();
	if (curFileViewType == eFileViewType::VIEW_ICON)
		return;

	m_centerFileBrowserList->SetFileViewType(eFileViewType::VIEW_ICON);
	CString path = m_filePathBar->GetCurrentPath();
	if (path.IsEmpty())	return;
	m_centerFileBrowserList->PopulateListView(path);
}

void CTopMenuBar::OnPreviewBtnClicked()
{
	eFileViewType curFileViewType = m_centerFileBrowserList->GetFileViewType();
	if (curFileViewType == eFileViewType::VIEW_PREVIEW)
		return;

	m_centerFileBrowserList->SetFileViewType(eFileViewType::VIEW_PREVIEW);
	CString path = m_filePathBar->GetCurrentPath();
	if (path.IsEmpty())	return;
	m_centerFileBrowserList->PopulateListView(path);
}

void CTopMenuBar::OnViewListBtnClicked()
{
	// 목록보기 숨기기, 크게보기 보이기
	HideViewListBtn();

	m_imageViewer->ShowWindow(SW_HIDE);

	m_centerFileBrowserList->ShowWindow(SW_SHOWNORMAL);
	m_leftFileBrowserTree->ShowWindow(SW_SHOWNORMAL);
	m_leftBottomPreview->ShowWindow(SW_SHOWNORMAL);
	m_mainWindow->ShowSplitter(true);

	// 이미지 보기가 끝났으니 경로는 상위 경로
	m_filePathBar->SetCurPathToEdit(m_filePathBar->GetPrevPath());

	m_imageViewer->HidePalette();
	
	// 이미지 크게보기 후 잔상 지우기
	m_mainWindow->EraseAfterImage();
}

void CTopMenuBar::OnExitBtnClicked()
{
	::DestroyWindow(m_mainWindow->m_hWnd);
}

void CTopMenuBar::OnEditBtnClicked()
{
	m_imageViewer->IsPaletteVisible() ? m_imageViewer->HidePalette() : m_imageViewer->ShowPalette();
}

void CTopMenuBar::OnSaveBtnClicked()
{
	eImageFormat imgFormat = m_imageViewer->GetImageFormat();
	CString path = CFileDialog::Save(imgFormat);
	if (path.IsEmpty())
	{
		//MessageBox(L"저장 실패", L"저장 실패", MB_OK);
		return;
	}

	m_imageViewer->ImageSave(path);
}

void CTopMenuBar::SetVisibleFileFuncBtns(const BOOL visible)
{
	HWND btn;
	for (UINT btnId : m_vFileBtnIds)
	{
		btn = GetDlgItem(btnId);
		if (::IsWindowVisible(btn) == visible)
			return;

		::ShowWindow(btn, visible);
	}
}

void CTopMenuBar::SetVisibleHomeFuncBtns(const BOOL visible)
{
	HWND btn;
	for (UINT btnId : m_vHomeBtnIds)
	{
		if (btnId == IDC_EDIT_BTN || btnId == IDC_SAVE_BTN || btnId == IDC_VIEW_LIST_BTN)
			continue;
		
		btn = GetDlgItem(btnId);

		if (::IsWindowVisible(btn) == visible)
			return;

		::ShowWindow(btn, visible);
	}
}

void CTopMenuBar::SetCenterFileBrowserList(CCenterFileBrowserList* centerFileBrowserList)
{
	m_centerFileBrowserList = centerFileBrowserList;
}

void CTopMenuBar::SetImageViewer(CImageViewer* imageViewer)
{
	m_imageViewer = imageViewer;
}

void CTopMenuBar::SetFilePathBar(CFilePathBar* filePathBar)
{
	m_filePathBar = filePathBar;
}

void CTopMenuBar::SetMainWindow(CMainWindow* mainWindow)
{
	m_mainWindow = mainWindow;
}

void CTopMenuBar::HideViewLargerBtn() const
{
	HWND hViewLargerBtn = GetDlgItem(IDC_VIEW_LARGER_BTN);
	::ShowWindow(hViewLargerBtn, SW_HIDE);

	HWND hIconViewBtn = GetDlgItem(IDC_ICONVIEW_BTN);
	::ShowWindow(hIconViewBtn, SW_HIDE);

	HWND hPreviewBtn = GetDlgItem(IDC_PREVIEW_BTN);
	::ShowWindow(hPreviewBtn, SW_HIDE);

	HWND hViewListBtn = GetDlgItem(IDC_VIEW_LIST_BTN);
	::ShowWindow(hViewListBtn, SW_SHOWNORMAL);

	HWND hEditBtn = GetDlgItem(IDC_EDIT_BTN);
	::ShowWindow(hEditBtn, SW_SHOWNORMAL);

	HWND hSaveBtn = GetDlgItem(IDC_SAVE_BTN);
	::ShowWindow(hSaveBtn, SW_SHOWNORMAL);

	::SetFocus(m_imageViewer->m_hWnd);
}

void CTopMenuBar::HideViewListBtn() const
{
	HWND hViewLargerBtn = GetDlgItem(IDC_VIEW_LARGER_BTN);
	::ShowWindow(hViewLargerBtn, SW_SHOWNORMAL);

	HWND hIconViewBtn = GetDlgItem(IDC_ICONVIEW_BTN);
	::ShowWindow(hIconViewBtn, SW_SHOWNORMAL);

	HWND hPreviewBtn = GetDlgItem(IDC_PREVIEW_BTN);
	::ShowWindow(hPreviewBtn, SW_SHOWNORMAL);

	HWND hViewListBtn = GetDlgItem(IDC_VIEW_LIST_BTN);
	::ShowWindow(hViewListBtn, SW_HIDE);

	HWND hEditBtn = GetDlgItem(IDC_EDIT_BTN);
	::ShowWindow(hEditBtn, SW_HIDE);

	HWND hSaveBtn = GetDlgItem(IDC_SAVE_BTN);
	::ShowWindow(hSaveBtn, SW_HIDE);
}

void CTopMenuBar::SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserTree)
{
	m_leftFileBrowserTree = leftFileBrowserTree;
}

void CTopMenuBar::SetLeftBottomPreview(CLeftBottomPreview* leftBottomPreview)
{
	m_leftBottomPreview = leftBottomPreview;
}

