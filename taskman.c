/*!
 * @file taskman.c
 * @brief Windows Task Manager
 *
 * Copyright 2023 by Yuri Prokushev
 *
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
  Heavely based on Undocumented Windows
*/

#include <stdio.h>	// sscanf

#include "taskman.h"

/*! @brief Initial dialog position parsed from the command line. */
static WORD nLeft=0, nTop=0;

/*!
 * @brief EnumWindows callback: add visible, titled windows to the task list.
 * @param[in] hWnd    Enumerated top-level window.
 * @param[in] lParam  Handle to the task list box.
 * @return TRUE to continue enumeration.
 * @retval TRUE Continue enumeration.
 */
BOOL CALLBACK _export EnumWindowsProc(HWND   hWnd, LPARAM lParam)
{
	char szBuf[80];

	if (IsWindowVisible(hWnd)) 
	{
		GetWindowText(hWnd, szBuf, sizeof(szBuf));
		if (lstrlen(szBuf))
		{
			SendMessage((HWND)lParam, LB_SETITEMDATA, SendMessage((HWND)lParam, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)szBuf), (LPARAM)hWnd);
		}
	}
  
	return TRUE;
}

/*!
 * @brief Dialog proc for the frame
 * @param[in] hWndDlg  Dialog window handle
 * @param[in] msg      WM_xxx message
 * @param[in] wParam   Message 16-bit parameter
 * @param[in] lParam   Message 32-bit parameter
 * @return Non-zero - Message processed; Zero - DefDlgProc() must process the message
 * @retval TRUE  Message processed
 * @retval FALSE DefDlgProc() must process the message
 */
BOOL CALLBACK _export TaskManDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			RECT rect;

			GetWindowRect(hWndDlg, &rect);
		        SetWindowPos(hWndDlg,
				HWND_NOTOPMOST,
				(GetSystemMetrics(SM_CXSCREEN)-(rect.right-rect.left))/2,
				(GetSystemMetrics(SM_CYSCREEN)-(rect.bottom-rect.top))/2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
			);
			return FALSE;
		}
	case WM_ACTIVATEAPP:
		{
			if (!wParam) // on deactivate
			{
				SendMessage(hWndDlg, WM_CLOSE, 0, 0);
			        return FALSE;
			}
			break;
		}
	case WM_ACTIVATE:
		{
			if (!wParam) // on deactivate
			{
				SendMessage(hWndDlg, WM_CLOSE, 0, 0);
				return FALSE;
			}
			break;
		}
	case WM_SHOWWINDOW:
		{
			if (wParam) // on show
			{
				HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);
				HINSTANCE hInst=(HINSTANCE)GetWindowWord(hWndDlg, GWW_HINSTANCE);
				FARPROC fpEnum;

				while (SendMessage(hListBox, LB_DELETESTRING, 0, 0)!=LB_ERR) {};

				SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

				fpEnum=MakeProcInstance((FARPROC)EnumWindowsProc, hInst);
				if (fpEnum)
				{
					EnumWindows((WNDENUMPROC)fpEnum, (LPARAM)hListBox);
					FreeProcInstance(fpEnum);
				}

				SendMessage(hListBox, LB_SETCURSEL, 0, 0);
				SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
				InvalidateRect(hListBox, NULL, TRUE);
				return FALSE;
			}
			break;
		}

	case WM_CLOSE:
		EndDialog(hWndDlg, IDCANCEL);
		return FALSE;
		break;

	case WM_COMMAND:
		switch (wParam) {
			case IDD_TASKLIST:
			{
				if(HIWORD(lParam) != LBN_DBLCLK)
			        {
					break;
				}
				// passthru
			}
			case IDD_SWITCHTO:
			{
				HWND hwnd, hwndPopup;
				HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);
	
				ShowWindow(hWndDlg, SW_HIDE);
	
				if (IsWindow(hwnd=SendMessage(hListBox, LB_GETITEMDATA, SendMessage(hListBox, LB_GETCURSEL, 0, 0), 0)))
				{
					if (IsWindow(hwndPopup=GetLastActivePopup(hwnd)))
					{
						if (!(GetWindowLong(hwndPopup, GWL_STYLE) & WS_DISABLED))
						{
							SwitchToThisWindow(hwndPopup, TRUE);
						}
					}
				}
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_ENDTASK:
			{
				HWND hwnd, hwndPopup;
				HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);

				ShowWindow(hWndDlg, SW_HIDE);

				if (IsWindow(hwnd=SendMessage(hListBox, LB_GETITEMDATA, SendMessage(hListBox, LB_GETCURSEL, 0, 0), 0)))
				{
					if (IsWindow(hwndPopup=GetLastActivePopup(hwnd)))
					{
						if (!(GetWindowLong(hwndPopup, GWL_STYLE) & WS_DISABLED))
						{
							SwitchToThisWindow(hwndPopup, TRUE);
							PostMessage(hwndPopup, WM_CLOSE, 0, 0);
						}
					}
				}
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDCANCEL:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_CASCADE:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				CascadeChildWindows(GetDesktopWindow(), 0);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_TILE:
			{
				HWND hWndDesktop;
				ShowWindow(hWndDlg, SW_HIDE);
				hWndDesktop=GetDesktopWindow();
				if (GetKeyState(VK_SHIFT) == 0x8000)
					TileChildWindows(hWndDesktop, MDITILE_HORIZONTAL);
				else
					TileChildWindows(hWndDesktop, MDITILE_VERTICAL);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_ARRANGEICONS:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				ArrangeIconicWindows(GetDesktopWindow());
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
		        break;
		}

	    default:
	        return FALSE;

	}

	return TRUE;
}

/*!
 * @brief Application entry point.
 * @param[in] inst     This instance
 * @param[in] prev     Previous instance
 * @param[in] cmdline  Command line
 * @param[in] show     Show state
 * @return Always 0
 * @retval 0 Always
 */
int PASCAL WinMain (HINSTANCE inst, HINSTANCE prev, LPSTR cmdline, int show)
{
	VOID (FAR PASCAL * RegisterPenApp)(UINT, BOOL);
	FARPROC fpDlgProc;
	char CmdLine[260];

	if (prev) 
	{
		return 0;
	}

	if (*cmdline)
	{
		lstrcpy(CmdLine, cmdline);
		sscanf(CmdLine, "%u %u", &nLeft, &nTop);
	}

	RegisterPenApp=(VOID (FAR PASCAL *)(UINT, BOOL))
		GetProcAddress((HMODULE)GetSystemMetrics(SM_PENWINDOWS), "RegisterPenApp");

	if (RegisterPenApp)
		(*RegisterPenApp)(RPA_DEFAULT, TRUE);

	if (fpDlgProc=MakeProcInstance((FARPROC)TaskManDlgProc, inst))
	{
		DialogBox(inst, MAKEINTRESOURCE(10), 0, (DLGPROC)fpDlgProc);
		FreeProcInstance(fpDlgProc);
	}

	if (RegisterPenApp)
	{
		(*RegisterPenApp)(RPA_DEFAULT, FALSE);
	}

	return 0;

}
