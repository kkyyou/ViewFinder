#pragma once
#include <atlbase.h>
#include <atlwin.h>

class CCenterFileBrowserList;
class CLeftFileBrowserTree;
class CSplitter : public CWindowImpl<CSplitter>
{
public:
    CSplitter(CLeftFileBrowserTree* p1, CCenterFileBrowserList* p2);
    virtual ~CSplitter();

    BEGIN_MSG_MAP(CSplitter)
        MESSAGE_HANDLER(WM_CREATE,              OnCreate)
        MESSAGE_HANDLER(WM_PAINT,               OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,             OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,                OnSize)
        MESSAGE_HANDLER(WM_MOUSEMOVE,           OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN,         OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP,           OnLButtonUp)
        MESSAGE_HANDLER(WM_SETCURSOR,           OnSetCursor)
    END_MSG_MAP()

    LRESULT                                     OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                     OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
                                                
    bool                                        IsDrag() const;
    void                                        SetDragFlag(bool isDrag);
    void                                        SetCaptureWnd(HWND captureWnd);
    void                                        SetPosX(int posX);

    int                                         GetPosX() const;

private:
    bool                                        m_isDrag;
    CLeftFileBrowserTree*                       m_leftPlane;
    CCenterFileBrowserList*                     m_rightPlane;
    HWND                                        m_hCaptureWnd;
    int                                         m_posX;
    int                                         m_oldPosX = -1;
};
