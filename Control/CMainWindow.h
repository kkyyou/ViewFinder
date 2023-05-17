#pragma once

#include "../Resource.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlctl.h>
#include <vector>

/*------------------------
        CMainWindow

        메인 윈도우
--------------------------*/

class CTopMenuBar;
class CFilePathBar;
class CLeftFileBrowserTree;
class CLeftBottomPreview;
class CCenterFileBrowserList;
class CSplitter;
class CImageViewer;
class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
    CMainWindow();
    virtual ~CMainWindow();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CMainWindow)
        MESSAGE_HANDLER(WM_CREATE,              OnCreate)
        MESSAGE_HANDLER(WM_PAINT,               OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,             OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,                OnSize)
        MESSAGE_HANDLER(WM_MOUSEMOVE,           OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN,         OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP,           OnLButtonUp)
        MESSAGE_HANDLER(WM_KEYDOWN,             OnKeyDown)
        MESSAGE_HANDLER(WM_KEYUP,               OnKeyUp)
        MESSAGE_HANDLER(WM_MOUSEWHEEL,          OnMouseWheel)
        MESSAGE_HANDLER(WM_ERASEBKGND,          OnEraseBkgnd)
        MESSAGE_HANDLER(WM_GETMINMAXINFO,       OnGetMinMaxInfo)
    END_MSG_MAP()

    LRESULT                                     OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    void                                        SetSplitterPosX(int posX);
    void                                        EraseAfterImage();
                                                
private:                                        
    bool                                        CreateChildWindows();
    void                                        DeleteChildWindows();
    void                                        LoadMainWndSizeToRegister();
    void                                        LoadSplitterPosToRegister();
    void                                        LoadLeftTreePathToRegister();
    void                                        GetValidSelectedIndex(int zDelta, int totalItemCount, int* selectedItemIdx);
                                                
private:               
    CTopMenuBar*                                m_topMenuBar;
    CFilePathBar*                               m_filePathBar;
    CLeftFileBrowserTree*                       m_leftFileBrowserTree;
    CLeftBottomPreview*                         m_leftBottomPreview;
    CCenterFileBrowserList*                     m_centerFileBrowserList;
    CImageViewer*                               m_imageViewer;
    CSplitter*                                  m_splitter;
                                                
    int                                         m_mousePosX;
    bool                                        m_pressedCtrl;

    // 스플리터 시작 시점의 화면을 저장하기 위해
    HDC                                         m_prevDC;
    HBITMAP                                     m_prevHBitmap;   
};                                     
