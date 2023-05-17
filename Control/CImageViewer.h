#pragma once

#include <atlbase.h>
#include <atlwin.h>

#include "../Image/CImage.h"


/*------------------------
      CLeftDownPreview

        상단 메뉴바
--------------------------*/

struct PosColorInfo
{
    COLORREF colorRef;
    int      x;
    int      y;
};

class CPalette;
class CImageViewer : public CWindowImpl<CImageViewer>
{
public:
    CImageViewer();
    ~CImageViewer();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CImageViewer)
        MESSAGE_HANDLER(WM_CREATE,       OnCreate)
        MESSAGE_HANDLER(WM_PAINT,        OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,      OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,         OnSize)
        MESSAGE_HANDLER(WM_MOUSEMOVE,    OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN,  OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP,    OnLButtonUp)
        MESSAGE_HANDLER(WM_KEYDOWN,      OnKeyDown)
        MESSAGE_HANDLER(WM_KEYUP,        OnKeyUp)
        MESSAGE_HANDLER(WM_ERASEBKGND,   OnEraseBkgnd)
    END_MSG_MAP()

    LRESULT                              OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                              OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
    bool                                 ImageLoad(const CString& path);
    bool                                 ImageSave(const CString& path);
    
    CImage*                              GetImage();
    
    void                                 ShowPalette();
    void                                 HidePalette();
    
    RECT                                 GetImageRect(const CImage& image);
    eImageFormat                         GetImageFormat() const;
    
    void                                 CaptureImage();
    bool                                 IsPaletteVisible() const;

    void                                 ZoomIn();
    void                                 ZoomOut();

    bool                                 IsPressedCtrl() const;

    void                                 DrawImage(HDC hdc, RECT* rect, float zoomScale);
    void                                 DrawBackground(HDC hdc, RECT* rect);
    void                                 DrawPixelInfo(HDC hdc);
    void                                 DrawOutline(HDC hdc, RECT* rect);


private:
    CImage*                              m_image;
    CImage*                              m_editedImage;
    PosColorInfo                         m_curPosColorInfo;
    RECT                                 m_rgbInfoRect;

    CPalette*                            m_palette;
    bool                                 m_isDrag;
    bool                                 m_pressedCtrl;

    POINT                                m_ptPrev;

    float                                m_zoomScale;
};


