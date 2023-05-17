#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

/*------------------------
        CFilePathBar

 현재 파일 경로를 나타내는 바
--------------------------*/

class CLeftFileBrowserTree;
class CCenterFileBrowserList;
class CFilePathBar : public CWindowImpl<CFilePathBar>
{
public:
    CFilePathBar();

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CFilePathBar)
        MESSAGE_HANDLER(WM_CREATE,          OnCreate)
        MESSAGE_HANDLER(WM_PAINT,           OnPaint)
        MESSAGE_HANDLER(WM_DESTROY,         OnDestroy)
        MESSAGE_HANDLER(WM_COMMAND,         OnCommand)
        MESSAGE_HANDLER(WM_SIZE,            OnSize)
    END_MSG_MAP()

    LRESULT                                 OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT                                 OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
                                            
    void                                    OnMoveBtnClickd(const CString& path);
                                            
public:                                     
    void                                    SetCurPathToEdit(const CString& path);
    void                                    SubclassEditWindow(HWND hEdit);
    void                                    SetLeftFileBrowserTree(CLeftFileBrowserTree* leftFileBrowserTree);
    void                                    SetMainView(CCenterFileBrowserList* centerFileBrowserList);
                                            
    CLeftFileBrowserTree*                   GetLeftFileBrowserTree() const;
    CCenterFileBrowserList*                 GetMainView()            const;
    CString                                 GetCurrentPath();
    CString                                 GetPrevPath();

private:                                    
    HWND                                    m_hEdit;
                                            
    CLeftFileBrowserTree*                   m_leftFileBrowserTree;
    CCenterFileBrowserList*                 m_centerFileBrowserList;
};

