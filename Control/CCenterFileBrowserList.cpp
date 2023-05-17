#include "CCenterFileBrowserList.h"
#include "CFilePathBar.h"
#include "CLeftFileBrowserTree.h"
#include "CImageViewer.h"
#include "CLeftBottomPreview.h"
#include "CTopMenuBar.h"
#include "CMainWindow.h"

#include "../Define/UiDefine.h"
#include "../resource.h"
#include "../Utils/CUtils.h"
#include "../Image/CImageFactory.h"

#include <string>
#include <CommCtrl.h>
#include <unordered_map>
#include <Shlwapi.h>
#include <thread>

using namespace std;

#define FOLDER_ICON_WIDTH  32
#define FOLDER_ICON_HEIGHT 32

CCenterFileBrowserList::CCenterFileBrowserList() :
	m_mainWindow(nullptr),
	m_filePathBar(nullptr),
	m_leftFileBrowserTree(nullptr),
	m_imageViewer(nullptr),
	m_topMenuBar(nullptr),
	m_selectedItemPath(L""),
	m_changedColumnCntWidth(0),
	m_hImgLargeList(nullptr),
	m_hListView(nullptr),
	m_leftBottomPreview(nullptr),
	m_dirObserverThread(nullptr),
	m_observeDirParam(ObserveDirParams()),
	m_fileViewType(eFileViewType::VIEW_PREVIEW),
	m_listViewItemInfoList(),
	m_curFolderCount(0),
	m_thumnailWidth(64),
	m_thumnailHeight(64)
{
}

CCenterFileBrowserList::~CCenterFileBrowserList()
{
	DeleteListItemImage();
	for (CImage* img : m_thumnailImages)
		delete img;
}

DWORD WINAPI ObserveDirectory(LPVOID lpParam)
{
	ObserveDirParams* odp = reinterpret_cast<ObserveDirParams*>(lpParam);
	if (!odp)
		return 0;

	CCenterFileBrowserList* centerFileBrowser = odp->centerFileBrowser;
	if (!centerFileBrowser)
		return 0;

	// 감시 경로
	CString path = odp->path;
	if (path.IsEmpty())
		return 0;

	// 감시 폴더 핸들
	HANDLE file = CreateFile(path,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL);

	// 비동기로 ReadDirectoryChangesW을 실행한다.
	HANDLE hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	if (hEvent == nullptr)
		return 0;

	OVERLAPPED overlapped;
	overlapped.hEvent = hEvent;
	uint8_t change_buf[1024];
	BOOL success = ReadDirectoryChangesW(
		file, change_buf, 1024, TRUE,
		FILE_NOTIFY_CHANGE_FILE_NAME |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_LAST_WRITE,
		NULL, &overlapped, NULL);

	// bStop == false : 쓰레드 실행
	while (!odp->bStop)
	{
		// 디렉토리 감시 진행 중...
		DWORD result = WaitForSingleObject(hEvent, 0);
		Sleep(1);

		if (result == WAIT_OBJECT_0)
		{
			// 디렉토리 변경 감지

			// 리스트뷰 변경 적용.
			::SendMessage(centerFileBrowser->m_hWnd, WM_DIR_CHANGED, 0, 0);

			// 다시 감시 시작.
			BOOL success = ReadDirectoryChangesW(
				file, change_buf, 1024, TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_LAST_WRITE,
				NULL, &overlapped, NULL);
		}
	}

	// bStop == true : 쓰레드 종료
	return 0;
}

LRESULT CCenterFileBrowserList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetParent().GetClientRect(&rect);

	// 리스트뷰
	m_hListView = ::CreateWindow(WC_LISTVIEW, NULL,
		WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_OWNERDATA,
		0, 0, rect.right - rect.left, rect.bottom - rect.top,
		m_hWnd, NULL, _AtlBaseModule.GetModuleInstance(), NULL);

	ListView_SetExtendedListViewStyleEx(m_hListView, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);

	return 0;
}

LRESULT CCenterFileBrowserList::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

LRESULT CCenterFileBrowserList::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseHandle(m_dirObserverThread);
	DeleteObject(m_hImgLargeList);
	DeleteObject(m_hListView);
	return 0;
}

LRESULT CCenterFileBrowserList::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 메인윈도우에 사이즈 맞추기.
	RECT rect;
	GetParent().GetClientRect(&rect);
	UINT clientWidth = rect.right - rect.left;
	UINT clientHeight = rect.bottom - rect.top;
	UINT myWidth = clientWidth - m_leftFileBrowserTree->GetWidth() - UI_SPLITTER_WIDTH;
	UINT myHeight = clientHeight - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	UINT myPosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
	MoveWindow(m_leftFileBrowserTree->GetWidth() + UI_SPLITTER_WIDTH, myPosY, myWidth - UI_SPLITTER_WIDTH, myHeight, FALSE);

	// CMainWindow 좌표 기준
	UINT listViewHeight = clientHeight - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
	UINT listviewPosY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
	::MoveWindow(m_hListView, 0, 0, myWidth - UI_SPLITTER_WIDTH, listViewHeight, FALSE);



	// 이거 리스트뷰에 LVS_AUTOARRANGE 주면 알아서 되...네..?
	// 
	// 
	//// 현재 들어갈 수 있는 최대 칼럼 카운트 계산
	//RECT rcItem;
	//int firstItem = ListView_GetTopIndex(m_hListView);
	//ListView_GetItemRect(m_hListView, firstItem, &rcItem, LVIR_BOUNDS);
	//int itemWidth = rcItem.right - rcItem.left + (rcItem.left * 2);
	//if (itemWidth == 0)	return 0;
	//int curMaxItems = myWidth / itemWidth;

	//// 칼럼 카운트가 변경되기 전의 최대 칼럼 카운트
	//int prevMaxItems = m_changedColumnCntWidth / itemWidth;

	// 윈도우 사이즈가 줄어들어 스크롤바가 생기는 경우 OR
	// 윈도우 사이즈가 늘어나서 리스트뷰 아이템 하나가 우측에 들어갈 공간이 생기는 경우
	//LONG_PTR dwStyle = ::GetWindowLongPtr(m_hListView, GWL_STYLE);
	//if (dwStyle & WS_HSCROLL || curMaxItems > prevMaxItems)
	//{
	//	// 선택되어 있는 아이템 기억
	//	int selectedItem = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);

	//	// 리스트뷰 아이템 다시 넣기
	//	PopulateListView(m_filePathBar->GetCurrentPath());
	//	m_changedColumnCntWidth = myWidth;
	//	
	//	// 선택되어있었던거 다시 선택
	//	if (selectedItem != -1) 
	//	{
	//		ListView_SetItemState(m_hListView, selectedItem, LVIS_SELECTED, LVIS_SELECTED);
	//	}
	//}

	return 0;
}

LRESULT CCenterFileBrowserList::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPNMHDR pnmh = (LPNMHDR)lParam;
	switch (pnmh->code)
	{
	case NM_DBLCLK:			OnListItemDBClick();			break;
	case NM_CLICK:			OnListItemSelected();	 		break;
	case LVN_ITEMCHANGED:   OnListItemChanged(lParam);		break;
	case LVN_GETDISPINFO:	OnListItemGetDispInfo(lParam);	break;
	default:												break;
	}
	return 0;
}

LRESULT CCenterFileBrowserList::OnDirChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PopulateListView(m_observeDirParam.path);
	return 0;
}

// path 내부의 파일/폴더를 찾아 리스트뷰를 채운다.
bool CCenterFileBrowserList::PopulateListView(const CString& path)
{
	if (path.IsEmpty())	return false;

	// 경로에 해당하는 파일 컨텐츠들을 가져온다.
	unsigned int oldFolderCount = m_curFolderCount;
	m_curFolderCount = 0;
	vector<CString> vContents = CUtils::GetFolderContents(path, &m_curFolderCount);
	if (vContents.empty())
	{
		m_curFolderCount = oldFolderCount;
		return false;
	}

	// 뷰 타입별 리스트뷰 채우기
	switch (m_fileViewType)
	{
	case eFileViewType::VIEW_ICON:		PopulateListViewIcon(path, vContents);		break;
	case eFileViewType::VIEW_PREVIEW:	PopulateListViewThumnail(path, vContents);  break;
	default:																		break;
	}

	return true;
}

void CCenterFileBrowserList::PopulateListViewIcon(const CString& path, const vector<CString>& vContents)
{
	if (!m_threadPool.IsStop())	
		m_threadPool.Stop();

	ClearListView(vContents.size());

	// 비트맵 로드
	HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, L"Images/imageList.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBitMap)
	{
		ImageList_Destroy(m_hImgLargeList);
		m_hImgLargeList = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), ILC_COLOR24, 0, 1);
		int nImageIndex = ImageList_Add(m_hImgLargeList, hBitMap, NULL);
		SendMessage(m_hListView, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)m_hImgLargeList);
		DeleteObject(hBitMap);
	}

	// 아이콘 보기
	for (size_t i = 0; i < vContents.size(); i++)
	{
		CString fileName = vContents[i];

		CString copyPath = path;
		copyPath.Append(L"\\");
		copyPath.Append(fileName);
		CUtils::RemoveDriveSymbol(&copyPath);

		ListViewItemInfo previewImgInfo;
		previewImgInfo.fileName = fileName;
		previewImgInfo.path = copyPath;

		m_listViewItemInfoList.push_back(previewImgInfo);
	}
}

void CCenterFileBrowserList::PopulateListViewThumnail(const CString& path, const std::vector<CString>& vContents)
{
	m_threadPool.Stop();  // 쓰레드풀의 쓰레드들이 일을끝내고 모두 Stop 할 때까지 대기한다.
	m_threadPool.InitThreadPool(thread::hardware_concurrency());

	ClearListView(vContents.size());
	m_listViewItemInfoList = GetListViewItemInfoList(path, vContents);

	// 폴더 아이콘을 썸네일 크기에 맞춰 추가한다.
	InitImageList();

	// 썸네일 이미지벡터 초기화
	for (CImage* img : m_thumnailImages)	delete img;
	m_thumnailImages.clear();
	m_thumnailImages.resize(m_listViewItemInfoList.size());

	// 쓰레드 풀 이미지 로드
	int index = 1;
	for (ListViewItemInfo& item : m_listViewItemInfoList)
	{
		if (CUtils::IsFolder(item.path))	continue;
		m_threadPool.EnqueueJob([this, &item, index]() 
			{ 
				if (m_threadPool.IsStop() == false)
					StartLoadThumnail(item, index, GetDC());
			}
		);
		index++;
	}
}

void CCenterFileBrowserList::InitImageList()
{
	// 초기화
	ImageList_RemoveAll(m_hImgLargeList);
	ImageList_Destroy(m_hImgLargeList);
	m_hImgLargeList = ImageList_Create(m_thumnailWidth, m_thumnailHeight, ILC_COLOR24, 0, 1);

	// 폴더 이미지 이미지 리스트에 추가
	AddFolderImageInImageList();

	HDC hdc = GetDC();

	// 이미지 로드 전 썸네일 검은색
	for (ListViewItemInfo& item : m_listViewItemInfoList)
	{
		// 썸네일 HBITMAP
		HDC memDC = ::CreateCompatibleDC(hdc);
		HBITMAP bm = ::CreateCompatibleBitmap(hdc, m_thumnailWidth, m_thumnailHeight);
		HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(memDC, bm);

		// 하얗게 바탕을 칠한다.
		RECT rcBorder;
		rcBorder.left = rcBorder.top = 0;
		rcBorder.right = m_thumnailWidth;
		rcBorder.bottom = m_thumnailHeight;
		HBRUSH hBrushBk = ::CreateSolidBrush(RGB(255, 255, 255));
		FillRect(memDC, &rcBorder, hBrushBk);
		SelectObject(memDC, pOldBitmapImage);

		// 이미지 리스트에 추가
		ImageList_Add(m_hImgLargeList, bm, NULL);

		DeleteDC(memDC);
		DeleteObject(bm);
		DeleteObject(hBrushBk);
	}

	// 이미지 리스트 셋팅
	ListView_SetImageList(m_hListView, m_hImgLargeList, LVSIL_NORMAL);
}

void CCenterFileBrowserList::AddFolderImageInImageList()
{
	// 폴더 이미지 추가
	HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, L"Images/folder.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBitMap == nullptr)	return;

	// 폴더 아이콘의 HDC
	HDC hdc = CreateCompatibleDC(GetDC());
	(HBITMAP)SelectObject(hdc, hBitMap);

	// 썸네일 크기의 HBITMAP 만들기
	HDC memHdc = CreateCompatibleDC(GetDC());
	HBITMAP bm = ::CreateCompatibleBitmap(GetDC(), m_thumnailWidth, m_thumnailHeight);
	HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(memHdc, bm);

	// 썸네일 그리기
	SetStretchBltMode(memHdc, HALFTONE);
	StretchBlt(memHdc, 0, 0, m_thumnailWidth, m_thumnailHeight, hdc, 0, 0, FOLDER_ICON_WIDTH, FOLDER_ICON_HEIGHT, SRCCOPY);
	SelectObject(memHdc, pOldBitmapImage);

	// 이미지리스트에 추가
	int nImageIndex = ImageList_Add(m_hImgLargeList, bm, NULL);

	DeleteDC(hdc);
	DeleteDC(memHdc);
	DeleteObject(hBitMap);
	DeleteObject(bm);
}

void CCenterFileBrowserList::SetFilePathBar(CFilePathBar* filePathBar)
{
	m_filePathBar = filePathBar;
}

void CCenterFileBrowserList::SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserTree)
{
	m_leftFileBrowserTree = leftFileBrowserTree;
}

void CCenterFileBrowserList::SetChangedColumnCntWidth(UINT width)
{
	m_changedColumnCntWidth = width;
}

void CCenterFileBrowserList::SetImageViewer(CImageViewer* imageViewer)
{
	m_imageViewer = imageViewer;
}

void CCenterFileBrowserList::SetLeftBottomPreview(CLeftBottomPreview* leftBottomPreview)
{
	m_leftBottomPreview = leftBottomPreview;
}

void CCenterFileBrowserList::SetTopMenuBar(CTopMenuBar* topMenuBar)
{
	m_topMenuBar = topMenuBar;
}

void CCenterFileBrowserList::SetMainWindow(CMainWindow* mainWindow)
{
	m_mainWindow = mainWindow;
}

void CCenterFileBrowserList::SetFileViewType(const eFileViewType fileViewType)
{
	m_fileViewType = fileViewType;
}

CString CCenterFileBrowserList::GetSelectedItemPath() const
{
	return m_selectedItemPath;
}

CString CCenterFileBrowserList::GetListItemPath() const
{
	// 선택된 파일/폴더 찾는다.
	int nIndex = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
	ListViewItemInfo listViewItemInfo = m_listViewItemInfoList[nIndex];
	return listViewItemInfo.path;
}

HWND CCenterFileBrowserList::GetListViewHandle() const
{
	return m_hListView;
}

eFileViewType CCenterFileBrowserList::GetFileViewType() const
{
	return m_fileViewType;
}

void CCenterFileBrowserList::StartObserveDirectory(const CString& path)
{
	CString copyPath = path;
	CUtils::RemoveDriveSymbol(&copyPath);

	// 쓰레드가 실행중이면 중지 시킨다.
	if (m_observeDirParam.bStop == false)
	{
		m_observeDirParam.bStop = true;
		WaitForSingleObject(m_dirObserverThread, INFINITE);
		CloseHandle(m_dirObserverThread);
	}

	// 새로운 디렉토리 경로 감지를 시킨다.
	m_observeDirParam.bStop = false;
	m_observeDirParam.path = copyPath;
	m_observeDirParam.centerFileBrowser = this;

	m_dirObserverThread = CreateThread(NULL, 0, ObserveDirectory, &m_observeDirParam, 0, NULL);
	if (m_dirObserverThread == NULL)
	{
		return;
	}
}

vector<ListViewItemInfo> CCenterFileBrowserList::GetListViewItemInfoList() const
{
	return m_listViewItemInfoList;
}

void CCenterFileBrowserList::OnListItemDBClick()
{
	int nIndex = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
	if (nIndex != -1)
	{
		// 더블클릭된 파일/폴더 찾는다.
		CString path = GetListItemPath();
		m_selectedItemPath = path;

		// .. 폴더를 더블클릭하여 상위 폴더로 갈때 경로에 \\..이 붙는것을 없애기 위해.
		CString rightThree = path.Right(3);
		if (rightThree.Compare(L"\\..") == 0)
		{
			path.Truncate(path.GetLength() - 3);
			int index = path.ReverseFind('\\');
			if (index != -1)
			{
				path.Delete(index, path.GetLength() - index);
			}
		}
		m_filePathBar->SetCurPathToEdit(path);

		// 리스트뷰 채워넣기
		if (PopulateListView(path) == false)
		{
			// 이미지인 경우
			if (m_imageViewer->ImageLoad(path))
			{
				// 크게보기
				ShowViewLargeImageViewer();

				// 크게보기 버튼 숨기기, 목록보기 보이기
				m_topMenuBar->HideViewLargerBtn();
				m_mainWindow->ShowSplitter(false);
			}
		}
		else
		{
			// 폴더인 경우 해당 폴더의 변화를 감시한다.
			StartObserveDirectory(path);
		}

		// LeftFileBrowserTree 아이템 선택하기
		if (m_leftFileBrowserTree)
		{
			string stdStr(CW2A(path.GetString()));
			m_leftFileBrowserTree->SelectTreeItem(stdStr);
		}
	}
}

void CCenterFileBrowserList::OnListItemSelected()
{
	int nIndex = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
	if (nIndex == -1)	return;

	// 선택된 파일 경로
	CString path = GetListItemPath();
	CString newSelectedPath = path;
	CUtils::RemoveDriveSymbol(&newSelectedPath);
	if (newSelectedPath.IsEmpty())
		return;

	// 유효한 패스 체크
	if (!CUtils::IsValidFilePath(newSelectedPath))
		return;

	// 폴더는 제외
	if (CUtils::IsFolder(newSelectedPath))
		return;

	// 이미 선택 되어있었으면 제외
	if (m_selectedItemPath.Compare(newSelectedPath) == 0)
		return;

	m_selectedItemPath = newSelectedPath;
	if (m_imageViewer->ImageLoad(m_selectedItemPath))
	{
		m_leftBottomPreview->StartPreview();
	}
}

void CCenterFileBrowserList::OnListItemChanged(LPARAM lParam)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
	if (pnmv->uChanged & LVIF_STATE && (pnmv->uNewState & LVIS_SELECTED))
	{
		int selectedItem = pnmv->iItem;
		ListViewItemInfo lvItem = m_listViewItemInfoList[selectedItem];
		if (lvItem.image)
		{
			delete lvItem.image;
			lvItem.image = nullptr;
		}

		ListView_SetItemState(m_hListView, selectedItem, LVIS_SELECTED, LVIS_SELECTED);
		OnListItemSelected();
	}
}

void CCenterFileBrowserList::OnListItemGetDispInfo(LPARAM lParam)
{
	if (m_listViewItemInfoList.empty())
		return;

	NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam;

	int itemIndex = plvdi->item.iItem;
	if (m_listViewItemInfoList.size() <= (size_t)itemIndex)
		return;

	ListViewItemInfo listViewItemInfo = m_listViewItemInfoList[itemIndex];

	CString fileName = listViewItemInfo.fileName;
	CString path = listViewItemInfo.path;

	// 파일 이름
	if ((plvdi->item.mask & LVIF_TEXT) == LVIF_TEXT)
	{
		lstrcpy(plvdi->item.pszText, fileName);
	}

	// 이미지
	if ((plvdi->item.mask & LVIF_IMAGE) == LVIF_IMAGE)
	{
		switch (m_fileViewType)
		{
		case eFileViewType::VIEW_ICON:
		{
			int imageIndex = 0;
			CString extension = PathFindExtension(path);
			if	    (extension.Compare(L".bmp") == 0)  imageIndex = (int)eImageType::IMAGE_BMP;
			else if (extension.Compare(L".jpg") == 0)  imageIndex = (int)eImageType::IMAGE_JPG;
			else if (extension.Compare(L".jpeg") == 0) imageIndex = (int)eImageType::IMAGE_JPG;
			else if (extension.IsEmpty())			   imageIndex = (int)eImageType::IMAGE_FOLDER;

			plvdi->item.iImage = imageIndex;
		} break;
		case eFileViewType::VIEW_PREVIEW:
		{
			if (CUtils::IsFolder(path))
			{
				plvdi->item.iImage = 0;
			}
			// 썸네일
			else
			{
				plvdi->item.iImage = itemIndex - (m_curFolderCount - 1);
			}

		} break;
		default: break;
		}
	}
	
}

void CCenterFileBrowserList::DeleteListItemImage()
{
	for (ListViewItemInfo pvii : m_listViewItemInfoList)
	{
		CImage* image = pvii.image;
		if (image)	delete image;
	}
	m_listViewItemInfoList.clear();
}

bool CCenterFileBrowserList::StartLoadThumnail(ListViewItemInfo& item, int index, HDC hdc)
{
	HBITMAP hbitmap;
	CImage* image = nullptr;
	if (!CUtils::LoadThumnail(item.path, m_thumnailWidth, m_thumnailHeight, index, GetDC(), hbitmap, m_threadPool, &image))
	{
		DeleteObject(hbitmap);
		return false;
	}

	if (m_threadPool.IsStop())
	{
		DeleteObject(hbitmap);
		return true;
	}

	if (image)
	{
		m_thumnailImages[index] = image;
	}
	
	// 이미지 리스트에 추가
	if (!ImageList_Replace(m_hImgLargeList, index, hbitmap, NULL))
	{
		DeleteObject(hbitmap);
		return false;
	}

	if (m_threadPool.IsStop())
	{
		DeleteObject(hbitmap);
		return true;
	}

	// 리스트뷰 아이템 다시 그리기
	// ##### SendMessage 사용하면 쓰레드풀과 메인쓰레드 데드락 발생 #####
	//SendMessage(m_hListView, LVM_REDRAWITEMS, (WPARAM)index + (m_curFolderCount - 1), (LPARAM)index + (m_curFolderCount - 1));
	PostMessageA(m_hListView, LVM_REDRAWITEMS, (WPARAM)index + (m_curFolderCount - 1), (LPARAM)index + (m_curFolderCount - 1));

	if (m_threadPool.IsStop())
	{
		DeleteObject(hbitmap);
		return true;
	}

	DeleteObject(hbitmap);

	return true;
}

void CCenterFileBrowserList::ResizeThumnail(int width, int height)
{
	m_thumnailWidth = width;
	m_thumnailHeight = height;
	
	InitImageList();

	// 썸네일
	HDC hdc = GetDC();
	HDC memHdc = ::CreateCompatibleDC(hdc);

	int index = 0;
	for (CImage* image : m_thumnailImages)
	{
		if (!image)	continue;
		index++;

		// 비율 계산해서 이미지 그릴 위치 정하기
		int XDest, YDest, nDestWidth, nDestHeight;
		CUtils::CalcThumnailPos(m_thumnailWidth, m_thumnailHeight, image->W(), image->H(), XDest, YDest, nDestWidth, nDestHeight);

		HBITMAP bm = ::CreateCompatibleBitmap(hdc, m_thumnailWidth, m_thumnailHeight);
		HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(memHdc, bm);

		// 하얗게 바탕을 칠한다.
		RECT rcBorder;
		rcBorder.left = rcBorder.top = 0;
		rcBorder.right = m_thumnailWidth;
		rcBorder.bottom = m_thumnailHeight;
		HBRUSH hBrushBk = ::CreateSolidBrush(RGB(255, 255, 255));
		FillRect(memHdc, &rcBorder, hBrushBk);

		// 썸네일 그리기
		SetStretchBltMode(memHdc, HALFTONE);
		BITMAPINFOHEADER bmpInfoHeader = image->CreateBitmapInfoHeader();
		StretchDIBits(memHdc, XDest, YDest, nDestWidth, nDestHeight, 0, 0, image->W(), image->H(), image->GetBitmapData().data()
			, (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(memHdc, pOldBitmapImage);

		// 이미지 리스트에 추가
		ImageList_Replace(m_hImgLargeList, index, bm, NULL);

		PostMessageA(m_hListView, LVM_REDRAWITEMS, (WPARAM)index + (m_curFolderCount - 1), (LPARAM)index + (m_curFolderCount - 1));
		DeleteObject(bm);
	}

	DeleteDC(memHdc);
}

int CCenterFileBrowserList::GetThumnailWidth() const
{
	return m_thumnailWidth;
}

void CCenterFileBrowserList::SetThumnailWidth(int thumWidth)
{
	m_thumnailWidth = thumWidth;
}

int CCenterFileBrowserList::GetThumnailHeight() const
{
	return m_thumnailHeight;
}

void CCenterFileBrowserList::SetThumnailHeight(int thumHeight)
{
	m_thumnailHeight = thumHeight;
}

vector<ListViewItemInfo> CCenterFileBrowserList::GetListViewItemInfoList(const CString& curPath, const std::vector<CString>& vContents)
{
	vector<ListViewItemInfo> listViewInfoList;
	listViewInfoList.reserve(vContents.size());

	for (size_t i = 0; i < vContents.size(); i++)
	{
		CString fileName = vContents[i];
		CString copyPath = curPath;
		copyPath.Append(L"\\");
		copyPath.Append(fileName);
		CUtils::RemoveDriveSymbol(&copyPath);

		ListViewItemInfo listItemImgInfo;
		listItemImgInfo.index = (int)i;
		listItemImgInfo.path = copyPath;
		listItemImgInfo.fileName = fileName;
		listViewInfoList.push_back(listItemImgInfo);
	}
	return listViewInfoList;
}

void CCenterFileBrowserList::ClearListView(size_t size)
{
	ListView_DeleteAllItems(m_hListView);
	DeleteListItemImage();
	ListView_SetItemCount(m_hListView, size);
}

void CCenterFileBrowserList::ShowViewLargeImageViewer()
{
	this->ShowWindow(SW_HIDE);
	m_leftFileBrowserTree->ShowWindow(SW_HIDE);
	m_leftBottomPreview->ShowWindow(SW_HIDE);
	m_imageViewer->ShowWindow(SW_NORMAL);
}

