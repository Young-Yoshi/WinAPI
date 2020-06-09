#include"function.h"

HFONT g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
COLORREF g_rgbText = RGB(0, 0, 0);

BOOL LoadTextFileToEdit(HWND hEdit, LPCTSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != UINT_MAX)
		{
			lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			ZeroMemory(lpszFileText, dwFileSize + 1);
			if (lpszFileText != NULL)
			{
				DWORD dwRead;
				if (ReadFile(hFile, lpszFileText, dwFileSize, &dwRead, NULL))
				{
					if (SetWindowText(hEdit, lpszFileText))
					{
						bSuccess = TRUE;
						SetFileNameToStatusBar(hEdit);
					}
				}
				//GlobalFree(lpszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL SaveTextFileFromEdit(HWND hEdit, LPCTSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(
		lpszFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextlength = GetWindowTextLength(hEdit);
		if (dwTextlength)
		{
			lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextlength + 1);
			if (lpszFileText)
			{
				if (GetWindowText(hEdit, lpszFileText, dwTextlength + 1))
				{
					DWORD dwWrite;
					if (WriteFile(hFile, lpszFileText, dwTextlength, &dwWrite, NULL))
						bSuccess = TRUE;
				}
				GlobalFree(lpszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL __stdcall DoFileNew(HWND hwnd)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);

	SetWindowText(hEdit, "");
	ZeroMemory(szFileName, sizeof(szFileName));
	
	if (lpszFileText) 
	{
		GlobalFree(lpszFileText);
		lpszFileText = NULL;
	}
	SetFileNameToStatusBar(GetDlgItem(hwnd, IDC_EDIT));
		return TRUE;
}

BOOL __stdcall DoFileOpen(HWND hwnd)
{
	//----------------OPEN TXT FILE----------------// 
	OPENFILENAME ofn;
	//CHAR szFileName[MAX_PATH]{};

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text files: (*.txt)\0*.txt\0All files (*.*)\0*.*";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";

	if (GetOpenFileName(&ofn))
	{
		LoadTextFileToEdit(GetDlgItem(hwnd, IDC_EDIT), szFileName);
		//return TRUE;
	}
	//----------------OPEN TXT FILE----------------//

	return FALSE;
}

VOID DoFileSaveAS(HWND hwnd)
{
	//----------------SAVEAS----------------//
	OPENFILENAME ofn;
	//CHAR szFileName[MAX_PATH]{};

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text files: (*.text)\0*.txt\0All file: (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "txt";

	if (GetSaveFileName(&ofn))
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
		SaveTextFileFromEdit(hEdit, szFileName);
	}
	//----------------SAVEAS----------------//
}

BOOL FileChanged(HWND hEdit)
{
	BOOL bFileWasChanged = FALSE;
	DWORD dwCurrentTextLength = GetWindowTextLength(hEdit);
	DWORD dwFileTextLength = lpszFileText ? strlen(lpszFileText) : 0;

	if (dwCurrentTextLength || dwFileTextLength)return TRUE;
	else
	{
		LPSTR lpszCurrentText = (LPSTR)GlobalAlloc(GPTR, dwCurrentTextLength + 1);
		if (lpszFileText && strcmp(lpszFileText, lpszCurrentText)) bFileWasChanged = TRUE;
		GlobalFree(lpszCurrentText);
	}
	return bFileWasChanged;
}

VOID DoSelectFont(HWND hwnd)
{
	CHOOSEFONT cf = { sizeof(CHOOSEFONT) };
	LOGFONT lf;

	GetObject(g_hFont, sizeof(LOGFONT), &lf);
	
	cf.Flags = CF_APPLY | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = g_rgbText;

	if (ChooseFont(&cf))
	{
		HFONT hf = CreateFontIndirect(&lf);
		if (hf)
		{
			g_hFont = hf;
		}
		else
		{
			MessageBox(hwnd, "Font creation failed!", "Error", MB_OK | MB_ICONERROR);
		}
		g_rgbText = cf.rgbColors;
	}
	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
	SendMessage(hEdit, WM_SETFONT, (WPARAM)g_hFont, MAKELPARAM(TRUE, 0));
	SendMessage(GetDlgItem(hwnd, IDC_EDIT), WM_CTLCOLOREDIT, g_rgbText, IDC_EDIT);
}


VOID WatchChanges(HWND hwnd, BOOL(__stdcall*Action)(HWND))
{
	if (FileChanged(GetDlgItem(hwnd, IDC_EDIT)))
	{
		switch (MessageBox(hwnd, "Save changed in file?", "Confirmation", MB_YESNOCANCEL | MB_ICONQUESTION))
		{
		case IDYES: SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
		case IDNO: Action(hwnd);
		case IDCANCEL: break;
		}
	}
	else
	{
		Action(hwnd);
	}
}

