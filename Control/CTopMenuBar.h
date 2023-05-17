#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <vector>

/*------------------------
        CTopMenuBar

        상단 메뉴바
--------------------------*/

class CCenterFileBrowserList;
class CImageViewer;
class CLeftFileBrowserTree;
class CLeftBottomPreview;
class CFilePathBar;
class CMainWindow;
class CTopMenuBar : public CWindowImpl<CTopMenuBar>
{
public:
    CTopMenuBar();
    virtual ~CTopMenuBar();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CTopMenuBar)
        MESSAGE_HANDLER(WM_CREATE,  OnCreate)
        MESSAGE_HANDLER(WM_PAINT,   OnPaint)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_SIZE,    OnSize)
    END_MSG_MAP()

    LRESULT                         OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                         OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                         OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                         OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                         OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    void                            OnFileBtnClicked();
    void                            OnHomeBtnClicked();
    void                            OnViewLargerBtnClicked();
    void                            OnIconViewBtnClicked();
    void                            OnPreviewBtnClicked();
    void                            OnViewListBtnClicked();
    void                            OnExitBtnClicked();
    void                            OnEditBtnClicked();
    void                            OnSaveBtnClicked();

    void                            SetVisibleFileFuncBtns(const BOOL visible);
    void                            SetVisibleHomeFuncBtns(const BOOL visible);

    void                            SetCenterFileBrowserList(CCenterFileBrowserList* centerFileBrowserList);
    void                            SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserList);
    void                            SetLeftBottomPreview(CLeftBottomPreview* leftBottomPreview);
    void                            SetImageViewer(CImageViewer* imageViewer);
    void                            SetFilePathBar(CFilePathBar* filePathBar);
    void                            SetMainWindow(CMainWindow* mainWindow);

    void                            HideViewLargerBtn() const;
    void                            HideViewListBtn() const;

private:
    std::vector<UINT>               m_vFileBtnIds;  /* 파일 탭의 버튼들 */
    std::vector<UINT>               m_vHomeBtnIds;  /* 홈 탭의 버튼들   */

    CMainWindow*                    m_mainWindow;
    CCenterFileBrowserList*         m_centerFileBrowserList;
    CLeftFileBrowserTree*           m_leftFileBrowserTree;
    CLeftBottomPreview*             m_leftBottomPreview;
    CImageViewer*                   m_imageViewer;
    CFilePathBar*                   m_filePathBar;
};

