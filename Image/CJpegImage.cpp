#include "CJpegImage.h"
#include "MyImageApi.h"

#include "../turbojpeg.h"
#include "../Utils/EzDllLoader.h"

#include <assert.h>

using namespace MyImageApi;
using namespace std;

// turbo-jpeg DLL 함수 정의
typedef tjhandle (*func_tjInitDecompress)(void);

typedef int (*func_tjDecompressHeader2)(tjhandle handle, unsigned char* jpegBuf,
                                        unsigned long jpegSize, int* width,
                                        int* height, int* jpegSubsamp);

typedef int (*func_tjDecompress2)(tjhandle handle, const unsigned char* jpegBuf,
                                  unsigned long jpegSize, unsigned char* dstBuf,
                                  int width, int pitch, int height, int pixelFormat,
                                  int flags);

typedef int (*func_tjDestroy)(tjhandle handle);

typedef tjhandle (*func_tjInitCompress)(void);

typedef int (*func_tjCompress2)(tjhandle handle, const unsigned char* srcBuf,
                                int width, int pitch, int height, int pixelFormat,
                                unsigned char** jpegBuf, unsigned long* jpegSize,
                                int jpegSubsamp, int jpegQual, int flags);

typedef void (*func_tjFree)(unsigned char* buffer);

CJpegImage::CJpegImage()
{
    m_format = eImageFormat::IMAGE_FORMAT_JPG;
}

CJpegImage::~CJpegImage()
{
}

CJpegImage* CJpegImage::Copy()
{
    CJpegImage* newJpegImage   = new CJpegImage();
    newJpegImage->m_path       = m_path;
    newJpegImage->m_width      = m_width;
    newJpegImage->m_height     = m_height;
    newJpegImage->m_bitmapData = m_bitmapData;
    newJpegImage->m_format     = m_format;
    newJpegImage->m_bpp        = m_bpp;
    newJpegImage->m_stride     = m_stride;
    
    return newJpegImage;
}

BOOL CJpegImage::Load(const CString& path)
{
    if (path.IsEmpty())     return false;

    CString currentDir = GetDLLPath();

    EzDllLoader ezDll(currentDir);
    if (!ezDll.IsLoaded()) { assert(0); return false; }

    // 파일 오픈
    HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // 파일 사이즈 구하기
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return false;
    }

    // JPEG 이미지 메모리 할당
    vector<BYTE> jpegData((UINT)size.QuadPart);
    if (!jpegData.data())
    {
        // 메모리 할당 실패
        CloseHandle(hFile);
        return false;
    }

    // 파일 읽기
    DWORD readBytes;
    if (!ReadFile(hFile, jpegData.data(), (DWORD)size.QuadPart, &readBytes, NULL)
        || readBytes != size.QuadPart)
    {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    
    // DLL 함수 가져오기
    func_tjInitDecompress tjInitDecompress = (func_tjInitDecompress)::GetProcAddress(ezDll.Handle(), "tjInitDecompress");
    func_tjDecompressHeader2 tjDecompressHeader2 = (func_tjDecompressHeader2)::GetProcAddress(ezDll.Handle(), "tjDecompressHeader2");
    func_tjDecompress2 tjDecompress2 = (func_tjDecompress2)::GetProcAddress(ezDll.Handle(), "tjDecompress2");
    func_tjDestroy tjDestroy = (func_tjDestroy) ::GetProcAddress(ezDll.Handle(), "tjDestroy");
    
    // 이미지 압축 풀기
    tjhandle tj = tjInitDecompress();
    int width, height, subsamp;
    tjDecompressHeader2(tj, jpegData.data(), (DWORD)size.QuadPart, &width, &height, &subsamp);
    vector<BYTE> imageData(width * height * 3);
    if (tjDecompress2(tj, jpegData.data(), (DWORD)size.QuadPart, imageData.data(), width, 0, height, TJPF_RGB, 0) != 0)
    {
        tjDestroy(tj);
        return false;
    }
    tjDestroy(tj);

    m_path = path;

    const int srcStride = width * 3;
    Create(width, height, 24);
    GetImageJpgDecodedData(imageData.data(), height, srcStride);

    return true;
}

BOOL CJpegImage::Load()
{
    return Load(m_path);
}

BOOL CJpegImage::Save(const CString& path)
{
    CString currentDir = GetDLLPath();
    EzDllLoader ezDll(currentDir);
    if (!ezDll.IsLoaded())  return false;

    // 파일 생성
    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // DLL 함수 가져오기
    func_tjInitCompress tjInitCompress = (func_tjInitCompress) ::GetProcAddress(ezDll.Handle(), "tjInitCompress");
    func_tjCompress2 tjCompress2 = (func_tjCompress2)::GetProcAddress(ezDll.Handle(), "tjCompress2");
    func_tjFree tjFree = (func_tjFree)::GetProcAddress(ezDll.Handle(), "tjFree");
    func_tjDestroy tjDestroy = (func_tjDestroy) ::GetProcAddress(ezDll.Handle(), "tjDestroy");

    tjhandle tjInstance = tjInitCompress();
    BYTE* jpegBuffer = NULL;
    DWORD jpegSize = 0;
    int stride = GetBmpStride(24);

    // BMP -> JPG
    // 이차원 PNG 데이터로 변환
    vector<vector<BYTE>> vImageData(m_height, vector<BYTE>(m_width * 3));
    GetTwoDimenJpgData(m_bitmapData, &vImageData);

    // 위 아래 뒤집기
    FlipUpAndDown(&vImageData);
    
    // 1차원 배열로 변환
    vector<BYTE> bitmapData(m_width * m_height * 3);
    GetOneDimenData(vImageData, &bitmapData);

    // JPG 포맷 압축
    int result = tjCompress2(tjInstance, bitmapData.data(), m_width, m_width * 3, m_height, TJPF_BGR, &jpegBuffer, &jpegSize, TJSAMP_444, 90, TJFLAG_FASTDCT);
    if (result < 0)
        return false;

    // 파일 쓰기
    DWORD bytesWritten;
    BOOL success = WriteFile(hFile, jpegBuffer, jpegSize, &bytesWritten, NULL);
    if (!success || bytesWritten != jpegSize)
    {
        return false;
    }

    // Clean Up
    CloseHandle(hFile);
    tjFree(jpegBuffer);
    tjDestroy(tjInstance);

    return true;
}

void CJpegImage::GetImageJpgDecodedData(BYTE* src, int height, int srcStride)
{
    // data 복사
    //memcpy(m_bitmapData.data(), src, srcStride * GetHeight());
    GetRawDataFrom(src, GetWidth(), height, srcStride);

    // RGB -> BGR 변환
    Rgb24ToBgr(m_bitmapData.data(), GetWidth(), GetHeight(), m_stride);

    // 위 아래 뒤집기
    FlipUpsideDown(m_bitmapData.data(), GetHeight(), m_stride);
}

CString CJpegImage::GetDLLPath()
{
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);

    CString currentDir = buffer;
    
#ifdef _WIN64
    currentDir.Append(L"\\turbojpeg-x64.dll");
#else
    currentDir.Append(L"\\turbojpeg-x86.dll");
#endif

    return currentDir;
}
