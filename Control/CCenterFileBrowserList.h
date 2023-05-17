#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <vector>

#include "../Thread/CThreadPool.h"
#include "../Define/UserMsgDefine.h"

/*------------------------
   CCenterFileBroserList

  중앙 파일 브라우저 리스트
--------------------------*/

class CMainWindow;
class CFilePathBar;
class CLeftFileBrowserTree;
class CImageViewer;
class CLeftBottomPreview;
class CTopMenuBar;
class CCenterFileBrowserList;
class CImage;

struct ObserveDirParams
{
    CString path = L"";
    bool bStop   = true;
    CCenterFileBrowserList* centerFileBrowser = nullptr;
};

struct ListViewItemInfo
{
    int     index = -1;
    CString fileName = L"";
    CString path = L"";
    CImage* image = nullptr;
};

enum class eImageType
{
    IMAGE_FOLDER,
    IMAGE_BMP,
    IMAGE_JPG
};

enum class eFileViewType
{
    VIEW_ICON,
    VIEW_PREVIEW
};

class CCenterFileBrowserList : public CWindowImpl<CCenterFileBrowserList>
{
public:
    CCenterFileBrowserList();
    virtual ~CCenterFileBrowserList();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CCenterFileBrowserList)
        MESSAGE_HANDLER(WM_CREATE,         OnCreate)
        MESSAGE_HANDLER(WM_PAINT,          OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,        OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,           OnSize)
        MESSAGE_HANDLER(WM_NOTIFY,         OnNotify)
        MESSAGE_HANDLER(WM_DIR_CHANGED,    OnDirChanged)
    END_MSG_MAP()

    LRESULT                                OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                OnDirChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    bool                                   PopulateListView(const CString& path);
    void                                   PopulateListViewIcon(const CString& path, const std::vector<CString>& vContents);
    void                                   PopulateListViewThumnail(const CString& path, const std::vector<CString>& vContents);

    void                                   InitImageList();
    void                                   AddFolderImageInImageList();

    void                                   ClearListView(size_t size);

    void                                   SetFilePathBar(CFilePathBar* filePathBar);
    void                                   SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserTree);
    void                                   SetChangedColumnCntWidth(UINT width);
    void                                   SetImageViewer(CImageViewer* imageViewer);
    void                                   SetLeftBottomPreview(CLeftBottomPreview* leftBottomPreview);
    void                                   SetTopMenuBar(CTopMenuBar* topMenuBar);
    void                                   SetMainWindow(CMainWindow* mainWindow);
    void                                   SetFileViewType(const eFileViewType fileViewType);

    void                                   ShowViewLargeImageViewer();

    CString                                GetSelectedItemPath() const;
    CString                                GetListItemPath() const;
    HWND                                   GetListViewHandle() const; 
    eFileViewType                          GetFileViewType() const;

    void                                   StartObserveDirectory(const CString& path);

    std::vector<ListViewItemInfo>          GetListViewItemInfoList() const;
    std::vector<ListViewItemInfo>          GetListViewItemInfoList(const CString& curPath, const std::vector<CString>& vContents);

    void                                   OnListItemDBClick();
    void                                   OnListItemSelected();
    void                                   OnListItemChanged(LPARAM lparam);
    void                                   OnListItemGetDispInfo(LPARAM lParam);
    
    void                                   DeleteListItemImage();

    bool                                   StartLoadThumnail(ListViewItemInfo& item, int index, HDC hdc);

    void                                   ResizeThumnail(int width, int height);

    int                                    GetThumnailWidth() const;
    void                                   SetThumnailWidth(int thumWidth);
    int                                    GetThumnailHeight() const;
    void                                   SetThumnailHeight(int thumHeight);

private:
    HWND                                   m_hListView;
    HIMAGELIST                             m_hImgLargeList;

    CMainWindow*                           m_mainWindow;
    CFilePathBar*                          m_filePathBar;
    CLeftFileBrowserTree*                  m_leftFileBrowserTree;
    CLeftBottomPreview*                    m_leftBottomPreview;
    CImageViewer*                          m_imageViewer;
    CTopMenuBar*                           m_topMenuBar;

    UINT                                   m_changedColumnCntWidth;        // 변경된 칼럼 카운트에 대한 Width
    CString                                m_selectedItemPath;

    HANDLE                                 m_dirObserverThread;
    ObserveDirParams                       m_observeDirParam;
    CThreadPool                            m_threadPool;

    eFileViewType                          m_fileViewType;
    std::vector<ListViewItemInfo>          m_listViewItemInfoList;

    int                                    m_curFolderCount;

//public:
    int                                    m_thumnailWidth;
    int                                    m_thumnailHeight;
    std::vector<CImage*>                   m_thumnailImages;
};

