#include "CLeftFileBrowserTree.h"
#include "CCenterFileBrowserList.h"
#include "CFilePathBar.h"

#include "../resource.h"
#include "../Define/UiDefine.h"
#include "../Utils/CUtils.h"

#include <string>
#include <ShlObj_core.h>

using namespace std;
CLeftFileBrowserTree::CLeftFileBrowserTree() :
	m_hTreeView(nullptr),
	m_images(nullptr),
	m_centerFileBrowserList(nullptr),
	m_width(UI_LEFT_FILE_BROWSER_TREE_WIDTH - (UI_SPLITTER_WIDTH / 2)),
	m_filePathBar(nullptr)
{
}

CLeftFileBrowserTree::~CLeftFileBrowserTree()
{
}

LRESULT CLeftFileBrowserTree::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetParent().GetClientRect(&rect);
	UINT clientHeight = rect.bottom - rect.top;
	UINT leftBrowserTreeHeight = clientHeight - UI_LEFT_DOWN_PREVIEW_HEIGHT - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;

	// 트리뷰 생성
	m_hTreeView = CreateWindow(
		WC_TREEVIEW,
		NULL,
		WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, 0, m_width, leftBrowserTreeHeight,
		m_hWnd,
		(HMENU)IDC_FILE_DIR_TREE_CTRL,
		_AtlBaseModule.GetModuleInstance(),
		NULL
	);

	TreeView_SetExtendedStyle(m_hTreeView, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);

	// 이미지 리스트 로드
	HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, L"Images/treeImageList.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBitMap)
	{
		m_images = ImageList_Create(16, 16, ILC_COLOR24, 0, 1);
		int nImageIndex = ImageList_Add(m_images, hBitMap, NULL);
		SendMessage(m_hTreeView, TVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)m_images);
		DeleteObject(hBitMap);
	}

	// 트리뷰 채우기
	StartPopulateTreeView();

	return 0;
}

LRESULT CLeftFileBrowserTree::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DeleteTreeViewItemLParams(TreeView_GetRoot(m_hTreeView));
	TreeView_DeleteAllItems(m_hTreeView);
	DeleteObject(m_hTreeView);
	ImageList_Destroy(m_images);
	return 0;
}

// 트리뷰를 채운다.
void CLeftFileBrowserTree::PopulateTreeView(LPCWSTR dirPath, HTREEITEM hParent)
{
	// 트리에 폴더구조 채워넣기
	WIN32_FIND_DATAW findData;

	// 경로로 폴더 검색을 할 때는 드라이브 심볼을 지운다.
	CString findPath = dirPath;
	CUtils::RemoveDriveSymbol(&findPath);
	HANDLE hFind = FindFirstFile((findPath + L"\\*.*"), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 폴더만
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;
			
			// 파일 체크
			if (false == isIncludedPathCheck(findData.cFileName))
				continue;

			CString path = (dirPath + std::wstring(L"\\")).c_str();
			path.Append(findData.cFileName);

			// 이미 추가된거면 추가안한다.
			std::string strPath2(CW2A(path.GetString()));
			if (m_treeItemMap[strPath2] != nullptr)
			{
				FindNextFile(hFind, &findData);
				continue;
			}

			TCHAR* pszPath = new TCHAR[path.GetLength() + 1];
			lstrcpy(pszPath, path);

			// 새 트리 아이템
			// 트리 아이템의 lParam(경로)는 드라이브 심볼(#DRV) 포함이다.
			TVITEMW tvItem;
			ZeroMemory(&tvItem, sizeof(tvItem));
			tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
			tvItem.pszText = findData.cFileName;
			tvItem.lParam = (LPARAM)pszPath;
			tvItem.iImage = (int)eImageType::IMAGE_FOLDER;

			TVINSERTSTRUCTW tvInsert;
			ZeroMemory(&tvInsert, sizeof(tvInsert));
			tvInsert.hParent = hParent;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.item = tvItem;
			HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);

			// 트리아이템을 빨리 찾을 수 있도록 해시테이블에 넣는다.
			std::string strPath(CW2A(path.GetString()));
			//m_treeItemMap.insert({ strPath, hItem });
			m_treeItemMap[strPath] = hItem;
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}
}

void CLeftFileBrowserTree::DeleteTreeViewItemLParams(HTREEITEM hItem)
{
	while (hItem)
	{
		TCHAR szText[MAX_PATH] = { 0 };
		TVITEM tvItem = { 0 };
		tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
		tvItem.hItem = hItem;
		tvItem.pszText = szText;
		tvItem.cchTextMax = MAX_PATH;
		TreeView_GetItem(m_hTreeView, &tvItem);

		// 트리아이템 경로 메모리 해제
		TCHAR* pszPath = reinterpret_cast<TCHAR*>(tvItem.lParam);
		delete[] pszPath;

		// 자식 아이템이 존재하면 재귀호출
		HTREEITEM hChildItem = TreeView_GetChild(m_hTreeView, hItem);
		if (hChildItem)
		{
			DeleteTreeViewItemLParams(hChildItem);
		}

		// 같은 레벨에 있는 아이템 찾기
		hItem = TreeView_GetNextSibling(m_hTreeView, hItem);
	}
}

bool CLeftFileBrowserTree::isIncludedPathCheck(const CString& fileName)
{
	if (fileName.Compare(L".") == 0 || fileName.Compare(L"..") == 0)
		return false;

	if (fileName.Compare(L"ProgramData") == 0 || fileName.GetAt(0) == '$')
		return false;

	if (fileName.Compare(L"System Volume Information") == 0 || fileName.Compare(L"Recovery") == 0)
		return false;

	if (fileName.Compare(L"OneDriveTemp") == 0 || fileName.Compare(L"Documents and Settings") == 0)
		return false;

	return true;
}

std::vector<CString> CLeftFileBrowserTree::GetInnerFilePaths(LPCWSTR dirPath)
{
	vector<CString> innerPaths;
	WIN32_FIND_DATAW findData;

	// 경로로 폴더 검색을 할 때는 드라이브 심볼을 지운다.
	CString findPath = dirPath;
	CUtils::RemoveDriveSymbol(&findPath);
	HANDLE hFind = FindFirstFile(findPath + L"\\*.*", &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 폴더만
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// 파일 체크
			if (!isIncludedPathCheck(findData.cFileName))
				continue;

			CString path = (dirPath + std::wstring(L"\\")).c_str();
			path.Append(findData.cFileName);
			innerPaths.push_back(path);
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);

	}

	return innerPaths;
}

void CLeftFileBrowserTree::SetMainView(CCenterFileBrowserList* centerFileBrowserList)
{
	m_centerFileBrowserList = centerFileBrowserList;
}

void CLeftFileBrowserTree::SetFilePathBar(CFilePathBar* filePathBar)
{
	m_filePathBar = filePathBar;
}

void CLeftFileBrowserTree::SelectTreeItem(const std::string& path)
{
	HTREEITEM hTreeItem = m_treeItemMap[path];
	if (hTreeItem == NULL)
		return;

	TreeView_SelectItem(m_hTreeView, hTreeItem);

	RECT rc;
	::GetClientRect(m_hTreeView, &rc);
	::InvalidateRect(m_hTreeView, &rc, TRUE);
}

void CLeftFileBrowserTree::SetWidth(UINT width)
{
	m_width = width;
}

UINT CLeftFileBrowserTree::GetWidth() const
{
	return m_width;
}

HWND CLeftFileBrowserTree::GetHTreeView() const
{
	return m_hTreeView;
}

std::unordered_map<std::string, HTREEITEM> CLeftFileBrowserTree::GetTreeItemMap() const
{
	// Key :경로 Value : HTREEITEM 
	// 트리아이템을 빨리 찾기 위한 용도
	return m_treeItemMap;
}

void CLeftFileBrowserTree::StartPopulateTreeView()
{
	// 파일 브라우저 트리 구성 
	
	// 즐겨찾기
	TCHAR* favorites = new TCHAR[MAX_PATH];
	ZeroMemory(favorites, sizeof(MAX_PATH));
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FAVORITES, NULL, 0, favorites)))
	{
		TVITEMW tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
		tvItem.iImage = (int)eImageType::IMAGE_FOLDER;
		tvItem.iSelectedImage = (int)eImageType::IMAGE_FOLDER;
		tvItem.lParam = (LPARAM)favorites;

		TCHAR text[5] = L"즐겨찾기";
		tvItem.pszText = text;

		TVINSERTSTRUCTW tvInsert;
		ZeroMemory(&tvInsert, sizeof(tvInsert));
		tvInsert.hParent = TVI_ROOT;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.item = tvItem;

		HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);

		// 트리아이템을 빨리 찾을 수 있도록 해시테이블에 넣는다.
		CString path = favorites;
		std::string strPath(CW2A(path.GetString()));
		m_treeItemMap.insert({ strPath, hItem });

		PopulateTreeView(favorites, hItem);
	}

	// 드라이브들 경로 구하기
	TCHAR drivePaths[MAX_PATH];
	GetLogicalDriveStrings(MAX_PATH, drivePaths);
	vector<CString> drives;
	TCHAR* ptr = drivePaths;
	while (*ptr)
	{
		UINT type = GetDriveType(ptr);
		if (type == DRIVE_FIXED || type == DRIVE_RAMDISK)
		{
			CString cs = ptr;
			cs.Remove('\\');

			// 드라이브 확인용 심볼 
			cs.Insert(0, L"#DRV");
			
			drives.push_back(cs);

			//drives.push_back(ptr);
		}
		ptr += _tcslen(ptr) + 1;
	}

	// 드라이브 트리 아이템 생성
	for (CString drivePath : drives)
	{
		TCHAR* pszDrivePath = new TCHAR[drivePath.GetLength() + 1];
		ZeroMemory(pszDrivePath, sizeof(drivePath.GetLength() + 1));

		_tcscpy_s(pszDrivePath, drivePath.GetLength() + 1, drivePath);

		CString driveName = drivePath;
		CUtils::RemoveDriveSymbol(&driveName);

		int start = 0;
		CString token = driveName.Tokenize(L":", start);
		driveName = token;
		driveName.Append(L"드라이브");

		// 드라이브 트리 아이템 만들기
		TVITEMW tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
		tvItem.iImage = (int)eImageType::IMAGE_CDRIVE;
		tvItem.iSelectedImage = (int)eImageType::IMAGE_CDRIVE;
		tvItem.lParam = (LPARAM)pszDrivePath;
		tvItem.pszText = (LPWSTR)(LPCTSTR)driveName;

		TVINSERTSTRUCTW tvInsert;
		ZeroMemory(&tvInsert, sizeof(tvInsert));
		tvInsert.hParent = TVI_ROOT;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.item = tvItem;

		// TREE 1 Depth 추가 (드라이브)
		HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);
		std::string strPath(CW2A(drivePath.GetString()));
		m_treeItemMap.insert({ strPath, hItem });

		// TREE 2 Depth 추가 (드라이브 하위 폴더)
		PopulateTreeView(drivePath, hItem);
	}

	// 바탕화면
	TCHAR* deskTopPath = new TCHAR[MAX_PATH];
	ZeroMemory(deskTopPath, sizeof(MAX_PATH));
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, deskTopPath)))
	{
		TVITEMW tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
		tvItem.iImage = (int)eImageType::IMAGE_DESKTOP;
		tvItem.iSelectedImage = (int)eImageType::IMAGE_DESKTOP;
		tvItem.lParam = (LPARAM)deskTopPath;

		TCHAR text[5] = L"바탕화면";
		tvItem.pszText = text;

		TVINSERTSTRUCTW tvInsert;
		ZeroMemory(&tvInsert, sizeof(tvInsert));
		tvInsert.hParent = TVI_ROOT;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.item = tvItem;

		HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);
		PopulateTreeView(deskTopPath, hItem);

		// 트리아이템을 빨리 찾을 수 있도록 해시테이블에 넣는다.
		CString path = deskTopPath;
		std::string strPath(CW2A(path.GetString()));
		m_treeItemMap.insert({ strPath, hItem });
	}
	return;
}

LRESULT CLeftFileBrowserTree::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CLeftFileBrowserTree::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetParent().GetClientRect(&rect);
	UINT clientWidth = rect.right - rect.left;
	UINT clientHeight = rect.bottom - rect.top;

	// CMainWindow 좌표 기준
	UINT leftBrowserTreeHeight = clientHeight - UI_LEFT_DOWN_PREVIEW_HEIGHT - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	UINT leftBrowserTreePosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
	MoveWindow(0, leftBrowserTreePosY, m_width, leftBrowserTreeHeight, FALSE);

	// CLeftFileBrowserTree 좌표 기준
	::MoveWindow(m_hTreeView, 0, 0, m_width, leftBrowserTreeHeight, FALSE);
	
	return 0;
}

LRESULT CLeftFileBrowserTree::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	NMHDR* pnmhdr = (NMHDR*)lParam;
	if (pnmhdr->code == TVN_SELCHANGED)
	{
		NM_TREEVIEW* pnmTreeView = (NM_TREEVIEW*)lParam;
		HTREEITEM hSelectedItem = pnmTreeView->itemNew.hItem;

		TCHAR szText[256];
		TVITEM tvItem;
		tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE;
		tvItem.hItem = hSelectedItem;
		tvItem.pszText = szText;
		tvItem.cchTextMax = 256;
		SendMessage(m_hTreeView, TVM_GETITEM, 0, (LPARAM)&tvItem);

		// 해당 트리아이템의 경로를 가져온다.
		CString path = reinterpret_cast<TCHAR*>(tvItem.lParam);
		if (m_centerFileBrowserList)
		{
			m_centerFileBrowserList->PopulateListView(path);
			m_centerFileBrowserList->StartObserveDirectory(path);
		}

		if (m_filePathBar)
			m_filePathBar->SetCurPathToEdit(path);
	}
	else if (pnmhdr->code == TVN_ITEMEXPANDED)
	{
		NM_TREEVIEW* pnmTreeView = (NM_TREEVIEW*)lParam;
		HTREEITEM hSelectedItem = pnmTreeView->itemNew.hItem;

		TCHAR szText[256];
		TVITEM tvItem;
		tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE;
		tvItem.hItem = hSelectedItem;
		tvItem.pszText = szText;
		tvItem.cchTextMax = 256;
		SendMessage(m_hTreeView, TVM_GETITEM, 0, (LPARAM)&tvItem);


		//
		//	C드라이브 같은 경우, 항목이 너무많아서 프로그램 실행전에 모두 찾아 넣는것은 무리가있어보인다.
		//	따라서, 사용자가 트리 항목을 Expand 했을 때 Expand할 아이템의 +2 Depth 아이템을 추가한다.
		//	
		//	ex) C드라이브를 Expand하면 C드라이브 내부 폴더들의 내부를 채워넣는다.
		//

		// 해당 트리아이템의 경로를 가져온다.
		CString path = reinterpret_cast<TCHAR*>(tvItem.lParam);

		// 현재 경로의 내부 파일 경로를 찾는다.
		vector<CString> innerPaths = GetInnerFilePaths(path);

		// 내부 파일 경로를 돌면서 아이템을 추가한다.
		for (CString innerPath : innerPaths)
		{
			string stdInnerPath(CW2A(innerPath.GetString()));

			// 내부파일의 HTREEITEM을 찾는다. (추가해야 할 아이템의 부모 아이템)
			HTREEITEM hItem = m_treeItemMap[stdInnerPath];

			// 채워넣는다.
			PopulateTreeView(innerPath, hItem);
		}

	}
	return 0;
}
