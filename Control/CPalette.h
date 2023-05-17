#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <vector>

class CPalette : public CWindowImpl<CPalette>
{
public:
    CPalette();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CPalette)
        MESSAGE_HANDLER(WM_CREATE,      OnCreate)
        MESSAGE_HANDLER(WM_PAINT,       OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,     OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,        OnSize)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_SETCURSOR,   OnSetCursor)
    END_MSG_MAP()

    LRESULT                             OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                             OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                             OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                             OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                             OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                             OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    COLORREF                            GetCurrentColorRef() const;
                                        
private:                                
    std::vector<COLORREF>               m_palette;
    COLORREF                            m_curColorRef;
};

