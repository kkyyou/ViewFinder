#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <string>

#include <unordered_map>
#include <unordered_set>

/*------------------------
   CLeftFileBrowserTree

   좌측 파일브라우저 트리
--------------------------*/

class CCenterFileBrowserList;
class CFilePathBar;
class CLeftFileBrowserTree : public CWindowImpl<CLeftFileBrowserTree>
{
public:
    CLeftFileBrowserTree();
    virtual ~CLeftFileBrowserTree();

    enum class eImageType
    {
        IMAGE_FOLDER,
        IMAGE_DESKTOP,
        IMAGE_CDRIVE
    };

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CLeftFileBrowserTree)
        MESSAGE_HANDLER(WM_CREATE,             OnCreate)
        MESSAGE_HANDLER(WM_PAINT,              OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,            OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,               OnSize)
        MESSAGE_HANDLER(WM_NOTIFY,             OnNotify)
    END_MSG_MAP()

    LRESULT                                    OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                    OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                    OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                    OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                    OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
                                               
    void                                       SetMainView(CCenterFileBrowserList* centerFileBrowserList);
    void                                       SetFilePathBar(CFilePathBar* filePathBar);
    void                                       SelectTreeItem(const std::string& path);
    void                                       SetWidth(UINT width);
                                               
    UINT                                       GetWidth() const;
    HWND                                       GetHTreeView() const;

    std::unordered_map<std::string, HTREEITEM> GetTreeItemMap() const;
    void                                       StartPopulateTreeView();

    std::vector<CString>                       GetInnerFilePaths(LPCWSTR dirPath);
    bool                                       isIncludedPathCheck(const CString& fileName);
    void                                       PopulateTreeView(LPCWSTR dirPath, HTREEITEM hParent);

private:
    void                                       DeleteTreeViewItemLParams(HTREEITEM hItem);

private:
    HWND                                       m_hTreeView;
    HIMAGELIST                                 m_images;

    CCenterFileBrowserList*                    m_centerFileBrowserList;
    CFilePathBar*                              m_filePathBar;

    UINT                                       m_width;

    std::unordered_map<std::string, HTREEITEM> m_treeItemMap;           // 경로로 TreeItem을 찾을 때 사용된다.
};

