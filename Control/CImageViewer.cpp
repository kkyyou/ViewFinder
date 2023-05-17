#include "CImageViewer.h"
#include "CPalette.h"

#include "../Define/UiDefine.h"
#include "../Image/CBmpImage.h"
#include "../Image/CJpegImage.h"
#include "../Image/CImageFactory.h"
#include "../Utils/CUtils.h"

using namespace std;

CImageViewer::CImageViewer() :
    m_image(nullptr),
    m_editedImage(nullptr),
    m_curPosColorInfo(PosColorInfo()),
    m_rgbInfoRect(RECT()),
    m_palette(nullptr),
    m_isDrag(false),
    m_pressedCtrl(false),
    m_ptPrev(POINT()),
    m_zoomScale(1)
{
}

CImageViewer::~CImageViewer()
{
    if (m_image)        delete m_image;
    if (m_editedImage)  delete m_editedImage;
    if (m_palette)      delete m_palette;
}

LRESULT CImageViewer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rcClient;
    GetClientRect(&rcClient);

    // 팔레트 생성
    m_palette = new CPalette();
    RECT rcPalette{ rcClient.right - UI_PALETTE_WIDTH - 5, rcClient.top + 5, rcClient.right - 5, UI_PALETTE_HEIGHT };
    if (!m_palette->Create(m_hWnd, rcPalette, NULL, (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER)))
    {
        return 0;
    }
    m_palette->ShowWindow(SW_HIDE);
    m_palette->UpdateWindow();

    return 0;
}

LRESULT CImageViewer::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
        
    if (m_image && !m_image->GetPath().IsEmpty())
    {
        RECT rc;
        GetClientRect(&rc);

        // 더블 버퍼링
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

        // 배경 검정색 채우기
        DrawBackground(hMemDC, &rc);

        // 이미지 그리기
        DrawImage(hMemDC, &rc, m_zoomScale);

        // 우측 하단에 픽셀 정보 그리기 
        DrawPixelInfo(hMemDC);

        // 편집 시 이미지 뷰어 외곽 하이라이트
        DrawOutline(hMemDC, &rc);

        // 고속 복사
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

        // Clean Up
        SelectObject(hdc, hOldBitmap);
        DeleteObject(hBitmap);
        ReleaseDC(hMemDC); 
    }

    EndPaint(&ps);

    return 0;
}

LRESULT CImageViewer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CImageViewer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rcMainWindow;
    GetParent().GetClientRect(&rcMainWindow);

    // 내 영역
    UINT posX = 0;
    UINT posY = UI_TOP_MENU_BAR_HEIGHT + UI_FILE_PATH_BAR_HEIGHT;
    UINT width = rcMainWindow.right - rcMainWindow.left;
    UINT height = rcMainWindow.bottom - rcMainWindow.top - UI_TOP_MENU_BAR_HEIGHT - UI_FILE_PATH_BAR_HEIGHT;
    MoveWindow(0, posY, width, height, TRUE);

    // RGB 정보 영역
    RECT rcClient;
    GetClientRect(&rcClient);
    int rgbInfoX = rcClient.right - 150;
    int rgbInfoY = rcClient.bottom - 100;
    m_rgbInfoRect = RECT{ rgbInfoX, rgbInfoY, rcClient.right, rcClient.bottom };

    // 팔레트에 OnSize Send
    ::SendMessage(m_palette->m_hWnd, uMsg, wParam, lParam);

    return 0;
}

LRESULT CImageViewer::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    HDC hdc = GetDC();
    COLORREF colorRef = GetPixel(hdc, x, y);

    PosColorInfo posColorInfo;
    posColorInfo.x = x;
    posColorInfo.y = y;
    posColorInfo.colorRef = colorRef;
    m_curPosColorInfo = posColorInfo;

    // RGB 정보 영역 Invalidate
    InvalidateRect(&m_rgbInfoRect);

    // 드래그 상태
    if (m_isDrag && m_palette->IsWindowVisible())
    {
        RECT rcImage = GetImageRect(*m_image);
        if (PtInRect(&rcImage, POINT{ x,y }))
        {
            // 이미지의 좌측 상단이 (0,0) 기준으로 내가 클릭 한 위치 보정
            int imagePosX = x - rcImage.left;
            int imagePosY = y - rcImage.top;

            // 선택한 색상 유효
            if (m_palette->GetCurrentColorRef() != CLR_INVALID)
            {
                // 실제 이미지 너비
                UINT realImageWidth = m_image->GetWidth();
                UINT realImageHeight = m_image->GetHeight();

                // 이미지가 화면에 그려진 영역의 너비
                UINT viewImageWidth = rcImage.right - rcImage.left;
                UINT viewImageHeight = rcImage.bottom - rcImage.top;

                // 화면에 그려진게 실제 이미지의 몇 배인가
                float scaleWidth = (float)viewImageWidth / (float)realImageWidth;
                float scaleHeight = (float)viewImageWidth / (float)realImageWidth;

                // 실제 데이터의 X,Y
                UINT realDataX = (int)(imagePosX / scaleWidth);
                UINT realDataY = (int)(imagePosY / scaleHeight);

                // 현재 클릭 위치와 이전 클릭위치의 크기
                int dx = abs(imagePosX - m_ptPrev.x);
                int dy = abs(imagePosY - m_ptPrev.y);

                // 그리는 방향이 오른쪽인지 왼쪽인지 (1 : 오른쪽 , -1 : 왼쪽)
                int sx = m_ptPrev.x < imagePosX ? 1 : -1;
                int sy = m_ptPrev.y < imagePosY ? 1 : -1;

                // X , Y 크기 차이
                int error = dx - dy;

                int x = m_ptPrev.x;
                int y = m_ptPrev.y;

				COLORREF editColorRef = m_palette->GetCurrentColorRef();
				vector<BYTE>* bitmapData = m_editedImage->GetBitmapDataRef();
                while (true)
                {
                    // 실제 데이터의 X , Y
                    UINT realDataX = (int)(x / scaleWidth);
                    UINT realDataY = (int)(y / scaleHeight);

                    // 실 데이터 인덱스 구하기
                    int index = (realDataX * 3) + (m_editedImage->GetHeight() - realDataY - 1) * m_editedImage->GetBmpStride(24);

                    // 실 데이터 RGB 변경
                    (*bitmapData)[index] = GetBValue(editColorRef);
                    (*bitmapData)[index + 1] = GetGValue(editColorRef);
                    (*bitmapData)[index + 2] = GetRValue(editColorRef);

                    // 내 마우스 포인터 위치까지 그렸으면 break;
                    if (x == imagePosX && y == imagePosY)
                        break;

                    // X 이동
                    int e2 = 2 * error;
                    if (e2 > -dy)
                    {
                        error -= dy;
                        x += sx;
                    }

                    // Y 이동
                    if (e2 < dx)
                    {
                        error += dx;
                        y += sy;
                    }
					InvalidateRect(&rcImage);
                }
            }

            m_ptPrev = POINT{ imagePosX, imagePosY };
        }

    }

    return 0;
}

LRESULT CImageViewer::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetFocus();

    if (!m_image)
    {
        m_isDrag = false;
        return 0;
    }

    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    RECT rcImage = GetImageRect(*m_image);
    if (PtInRect(&rcImage, POINT{ x,y }) && m_palette->IsWindowVisible())
    {
        m_isDrag = true;
        m_ptPrev.x = x - rcImage.left;
        m_ptPrev.y = y - rcImage.top;
    }
    else
    {
        m_isDrag = false;
    }
    return 0;
}

LRESULT CImageViewer::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_isDrag)
    {
		m_isDrag = false;
        //CaptureImage();
    }
    return 0;
}

LRESULT CImageViewer::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == VK_CONTROL)
    {
        m_pressedCtrl = true;
    }
    else if (wParam == VK_ESCAPE)
    {
        if (IsPaletteVisible())
        {
            HidePalette();
        }
    }

    return 0;
}

LRESULT CImageViewer::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == VK_CONTROL)
    {
        m_pressedCtrl = false;
    }

    return 0;
}

bool CImageViewer::ImageLoad(const CString& path)
{
    CString copyPath = path;
    CUtils::RemoveDriveSymbol(&copyPath);

    // 유효 경로 체크
    if (!PathFileExists(copyPath))
        return false;

    // 이미지 체크
    if (m_image && !m_image->GetPath().IsEmpty())
    {
        CString curImgPath = m_image->GetPath();
        if (curImgPath.Compare(copyPath) == 0)                  
        {
            // 이미 로드되있는거는 다시 로드하지말자
            return true;
        }
    }

    // 이미지 메모리 해제
    if (m_image)       { delete m_image;       m_image = nullptr;       }
    if (m_editedImage) { delete m_editedImage; m_editedImage = nullptr; }

    // 이미지 로드
    m_image = CImageFactory::Create(path);
    m_image->Load(path);

    // 이미지 꽉차게 보이도록 비율 계산
    RECT rc;
    GetClientRect(&rc);
    UINT imgViewerWidth = rc.right - rc.left;
    UINT imgViewerHeight = rc.bottom - rc.top;

    float scaleWidth = (float)imgViewerWidth / (float)m_image->GetWidth();
    float scaleHeight = (float)imgViewerHeight / (float)m_image->GetHeight();
    m_zoomScale = min(scaleWidth, scaleHeight);

    // 원본 카피
    m_editedImage = m_image->Copy();
    
    return true;
}

bool CImageViewer::ImageSave(const CString& path)
{
    // 원본 확장자와 저장 확장자가 같으면 그냥 저장.
    CString oldExtension = PathFindExtension(m_editedImage->GetPath());
    CString newExtension = PathFindExtension(path);
    if (oldExtension.Compare(newExtension) == 0)   
        m_editedImage->Save(path);
    else                                            
        m_editedImage->SaveDiffExtension(path);

    return true;
}

CImage* CImageViewer::GetImage()
{
    return m_image;
}

void CImageViewer::ShowPalette()
{
    m_palette->ShowWindow(SW_SHOWNORMAL);
    m_palette->UpdateWindow();

    Invalidate();
}

void CImageViewer::HidePalette()
{
    m_palette->ShowWindow(SW_HIDE);
    m_palette->UpdateWindow();

    Invalidate();
}

// 줌 스케일을 적용한 이미지의 영역을 구한다.
RECT CImageViewer::GetImageRect(const CImage& image)
{
    RECT rc;
    GetClientRect(&rc);

    LONG clientCenterX = (rc.right - rc.left) / 2;
    LONG clientCenterY = (rc.bottom - rc.top) / 2;

    LONG imageWidth = (LONG)(m_image->GetWidth() * m_zoomScale);
    LONG imageHeight = (LONG)(m_image->GetHeight() * m_zoomScale);

    LONG x = clientCenterX - (imageWidth / 2);
    LONG y = clientCenterY - (imageHeight / 2);

    RECT rcImage{ x, y, x + imageWidth, y + imageHeight };
    return rcImage;
}

void CImageViewer::CaptureImage()
{
    if (!m_image)   return;

    // 이미지 영역
    RECT rcImage = GetImageRect(*m_image);
    int imageWidth = (rcImage.right - rcImage.left);
    int imageHeight = (rcImage.bottom - rcImage.top);

    // 이미지 뷰어 영역
    RECT rcClient;
    GetClientRect(&rcClient);
    int clientCenterX = (rcClient.right - rcClient.left) / 2;
    int clientCenterY = (rcClient.bottom - rcClient.top) / 2;

    // 이미지의 시작 Position
    int copyStartX = clientCenterX - (imageWidth / 2);
    int copyStartY = clientCenterY - (imageHeight / 2);

    // 현재 이미지 뷰어의 영역을 memHdc에 그려준다.
    HDC hdcScreen = GetDC();
    HDC memHdc = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, imageWidth, imageHeight);
    SelectObject(memHdc, hBitmap);

    BitBlt(memHdc, 0, 0, imageWidth, imageHeight, hdcScreen, copyStartX, copyStartY, SRCCOPY);

    // BMP
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_image->GetWidth();
    bmi.bmiHeader.biHeight = m_image->GetHeight();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    // 이미지 캡처 데이터
    vector<BYTE> bmData(m_image->GetBitmapData().size());
    GetDIBits(memHdc, hBitmap, 0, imageHeight, bmData.data(), &bmi, DIB_RGB_COLORS);
    
    // 편집 이미지
    m_editedImage->SetBitmapData(bmData);

    // Clean Up
    DeleteObject(hBitmap);
    DeleteDC(memHdc);
    ReleaseDC(hdcScreen);
}

bool CImageViewer::IsPaletteVisible() const
{
    return m_palette->IsWindowVisible();
}

eImageFormat CImageViewer::GetImageFormat() const
{
    return m_image->GetImageFormat();
}

void CImageViewer::ZoomIn()
{
    m_zoomScale += 0.1f;
    Invalidate();
}

void CImageViewer::ZoomOut()
{
    if (m_zoomScale <= 0.15f)
        return;

    m_zoomScale -= 0.1f;
    Invalidate();
}

bool CImageViewer::IsPressedCtrl() const
{
    return m_pressedCtrl;
}

void CImageViewer::DrawImage(HDC hdc, RECT* rect, float zoomScale)
{
    UINT imgViewerWidth = rect->right - rect->left;
    UINT imgViewerHeight = rect->bottom - rect->top;

    UINT clientCenterX = imgViewerWidth / 2;
    UINT clientCenterY = imgViewerHeight / 2;

    UINT imageWidth = (UINT)(m_image->GetWidth() * m_zoomScale);
    UINT imageHeight = (UINT)(m_image->GetHeight() * m_zoomScale);

    UINT x = clientCenterX - (imageWidth / 2);
    UINT y = clientCenterY - (imageHeight / 2);

    // 그리기
    m_editedImage->Draw(hdc, x, y, m_zoomScale);
}

void CImageViewer::DrawBackground(HDC hdc, RECT* rect)
{
    SetBkColor(hdc, RGB(0, 0, 0));
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, rect, brush);
    DeleteObject(brush);
}

void CImageViewer::DrawPixelInfo(HDC hdc)
{
    SetTextColor(hdc, RGB(255, 255, 255));

    CString posText;
    posText.Format(L"X : %d , Y : %d", m_curPosColorInfo.x, m_curPosColorInfo.y);

    COLORREF colorRef = m_curPosColorInfo.colorRef;
    CString r;
    r.Format(L"R : %d", GetRValue(colorRef));

    CString g;
    g.Format(L"G : %d", GetGValue(colorRef));

    CString b;
    b.Format(L"B : %d", GetBValue(colorRef));

    // TEXT DRAW
    TextOut(hdc, m_rgbInfoRect.left, m_rgbInfoRect.top, posText, posText.GetLength());
    TextOut(hdc, m_rgbInfoRect.left, m_rgbInfoRect.top + 15, r, r.GetLength());
    TextOut(hdc, m_rgbInfoRect.left, m_rgbInfoRect.top + 30, g, g.GetLength());
    TextOut(hdc, m_rgbInfoRect.left, m_rgbInfoRect.top + 45, b, b.GetLength());
}

void CImageViewer::DrawOutline(HDC hdc, RECT* rect)
{
    if (m_palette->IsWindowVisible())
    {
        HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN hPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 0));
        HGDIOBJ hOldBrush = SelectObject(hdc, hNullBrush);
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);

        // 외곽선
        Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

LRESULT CImageViewer::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}
