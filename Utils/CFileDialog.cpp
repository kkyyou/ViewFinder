#include "CFileDialog.h"

#include <atlbase.h>

// 파일다이얼로그 띄우기
CString CFileDialog::ShowFileDialog(const CLSID fileDlgType, eImageFormat imageFormat)
{
	// 경로
	LPWSTR pszFilePath = NULL;

	// 이미지 파일 선택
	IFileDialog* pFileDialog = NULL;
	HRESULT hr = CoCreateInstance(fileDlgType, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
	if (SUCCEEDED(hr))
	{
		// Set file dialog options
		DWORD dwFlags;
		hr = pFileDialog->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			hr = pFileDialog->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
		}

		// 파일 타입
		COMDLG_FILTERSPEC fileTypes[] = { 
			{ L"Bitmap File", L"*.bmp" }, 
			{ L"Jpeg File", L"*.jpg" },
			{ L"Png File", L"*.png" }
		};
		hr = pFileDialog->SetFileTypes(_countof(fileTypes), fileTypes);

		// 파일다이얼로그 파일 형식 선택
		if (imageFormat != eImageFormat::INVALID)
		{
			pFileDialog->SetFileTypeIndex((int)imageFormat);
		}

		// Show file dialog
		hr = pFileDialog->Show(NULL);

		// Get selected file path
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem = NULL;
			hr = pFileDialog->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				CoTaskMemFree(pszFilePath);
			}
			pItem->Release();
		}
	}

	// 저장 시 확장자 붙여주기
	CString path;
	if (fileDlgType == CLSID_FileSaveDialog)
	{
		UINT fTypeIdx = 0;
		pFileDialog->GetFileTypeIndex(&fTypeIdx);
		path = pszFilePath;

		// 저장하기 전에 확장자가 없는 파일만 확장자 추가
		CString extension = PathFindExtension(path);
		if (extension.IsEmpty())
		{
			if		(fTypeIdx == 1)	path.Append(L".bmp");
			else if (fTypeIdx == 2)	path.Append(L".jpg");
			else if (fTypeIdx == 3)	path.Append(L".png");
		}

	}

	pFileDialog->Release();

	return pszFilePath != NULL ? CString(path) : CString();
}

// 파일 열기
CString CFileDialog::Open()
{
	return ShowFileDialog(CLSID_FileOpenDialog);
}

// 파일 저장
CString CFileDialog::Save(eImageFormat imageFormat)
{
	return ShowFileDialog(CLSID_FileSaveDialog, imageFormat);
}
