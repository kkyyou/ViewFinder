#pragma once

#include <atlbase.h>
#include <atlwin.h>

/*------------------------
    CLeftBottomPreview

      좌측 하단 프리뷰
--------------------------*/

class CCenterFileBrowserList;
class CImageViewer;
class CLeftBottomPreview : public CWindowImpl<CLeftBottomPreview>
{
public:
    CLeftBottomPreview();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CLeftBottomPreview)
        MESSAGE_HANDLER(WM_CREATE,          OnCreate)
        MESSAGE_HANDLER(WM_PAINT,           OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,         OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,            OnSize)
    END_MSG_MAP()

    LRESULT                                 OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    void                                    SetWidth(const UINT width);
    void                                    SetCenterFileBrowserList(CCenterFileBrowserList* centerFileBrowserList);
    void                                    SetImageViewer(CImageViewer* imageViewer);
    void                                    StartPreview();

private:
    UINT                                    m_width;
    UINT                                    m_height;

    CCenterFileBrowserList*                 m_centerFileBrowserList;
    CImageViewer*                           m_imageViewer;

    UINT                                    m_imgStartX;
    UINT                                    m_imgStartY;
    float                                   m_scale;
};

