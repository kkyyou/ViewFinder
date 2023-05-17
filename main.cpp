#include "Control/CMainWindow.h"
#include <assert.h>


extern "C" int __cdecl OnMyExit()
{
#ifdef _DEBUG
	if (_CrtDumpMemoryLeaks())
	{
		//assert(0);
		OutputDebugStringA(" # ViewFinder: Memory Leak found!!!!!!!!!!!!!!!!!!!!\n");
		OutputDebugStringA(" # ViewFinder: Memory Leak found!!!!!!!!!!!!!!!!!!!!\n");
		OutputDebugStringA(" # ViewFinder: Memory Leak found!!!!!!!!!!!!!!!!!!!!\n");
	}
	else
	{
		OutputDebugStringA(" # ViewFinder: No memory leak found. ###\n");
	}
#endif
	return 0;
}

_Use_decl_annotations_ int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)

{
#ifdef _DEBUG
	//_CrtSetBreakAlloc(405);

	// 메모리 누수 체크
	_onexit(OnMyExit);

	//_onexit(_CrtDumpMemoryLeaks);                 
#endif

	// 메인 윈도우 생성
	CMainWindow wnd;
	if (!wnd.Create(NULL, CWindow::rcDefault, _T("ViewFinder"), WS_OVERLAPPEDWINDOW))
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	// 프로그램 아이콘 로드
	HICON hIcon = ::LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDI_VIEWFINDER));
	if (hIcon)	wnd.SetIcon(hIcon);

	wnd.ShowWindow(nCmdShow);
	wnd.UpdateWindow();

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	                   
	return (int)msg.wParam;
}
