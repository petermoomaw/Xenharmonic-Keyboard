#include "stdafx.h"
#include "Enharmonic.h"
#include "Dialog.h"
#include <windows.h>    // included for Windows Touch
#include <windowsx.h>   // included for point conversion
#include <Commdlg.h>
#include <strsafe.h>
#include "csound.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
//ATOM				MyRegisterClass(HINSTANCE hInstance);
//BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	TemperamentDialogProc(HWND, UINT, WPARAM, LPARAM);

Enharmonic::Enharmonic()
{
	latticeView.enharmonic = this;
}

Enharmonic::~Enharmonic()
{

}

void Enharmonic::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hTemperamentDlg, &msg))
		{
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
	    }
	}
}

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

HRESULT Enharmonic::Initialize()
{
	HRESULT hr;

	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = latticeView.CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Enharmonic::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"D2DEnharmonic";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		latticeView.m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


		// Create the window. https://msdn.microsoft.com/en-us/library/windows/desktop/ms632679(v=vs.85).aspx
		latticeView.m_hwnd = CreateWindow(
			L"D2DEnharmonic",
			L"Direct2D Demo App",
			WS_OVERLAPPEDWINDOW,  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(1000.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(600.f * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = latticeView.m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			// register the window for touch instead of gestures
			RegisterTouchWindow(latticeView.m_hwnd, 0);
			//RegisterTouchWindow(latticeView.m_hwnd, TWF_FINETOUCH);
			//RegisterTouchWindow(latticeView.m_hwnd, TWF_FINETOUCH && TWF_WANTPALM);

			BOOL fEnabled = FALSE;
			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_TOUCH_CONTACTVISUALIZATION,
				0, sizeof(fEnabled), &fEnabled);
			
			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_TOUCH_TAP,
				0, sizeof(fEnabled), &fEnabled);

			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_TOUCH_DOUBLETAP,
				0, sizeof(fEnabled), &fEnabled);

			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_TOUCH_PRESSANDHOLD,
				0, sizeof(fEnabled), &fEnabled);

			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_GESTURE_PRESSANDTAP,
				0, sizeof(fEnabled), &fEnabled);

			SetWindowFeedbackSetting(latticeView.m_hwnd,
				FEEDBACK_TOUCH_RIGHTTAP,
				0, sizeof(fEnabled), &fEnabled);

			hTemperamentDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOGMAIN), latticeView.m_hwnd, Main_DlgProc);
			DLGHDR *pHdr = (DLGHDR *)GetWindowLong(hTemperamentDlg, GWL_USERDATA); //Save a pointer to enharmonic objejt in dialog
			pHdr->enharmonic = this;

			ShowWindow(latticeView.m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(latticeView.m_hwnd);
		}
	}

	return hr;
}

int WINAPI WinMain(
	HINSTANCE hInstIN /* hInstance */,
	HINSTANCE /* hPrevInstance */,
	LPSTR /* lpCmdLine */,
	int nCmdShow
	)
{
	hInst = hInstIN;

	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			Enharmonic app;

			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}




LRESULT CALLBACK Enharmonic::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		Enharmonic *pEnharmonic = (Enharmonic *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pEnharmonic)
			);

		result = 1;
	}
	else
	{
		Enharmonic *pEnharmonic = reinterpret_cast<Enharmonic *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
				)));

		bool wasHandled = false;

		if (pEnharmonic)
		{
			switch (message)
			{
			case WM_TOUCH:
				pEnharmonic->latticeView.ProcessTouch(hwnd, wParam, lParam);
				break;
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pEnharmonic->latticeView.OnResize(width, height);
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			result = 0;
			wasHandled = true;
			break;

		    case WM_PAINT:
			{
				pEnharmonic->latticeView.OnRender();
				ValidateRect(hwnd, NULL);
			}
			result = 0;
			wasHandled = true;

			break;
			
			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			result = 1;
			wasHandled = true;
			break;

			case  WM_CHAR:  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx
				switch (wParam)
				{
				case 0x08:  // backspace 
				case 0x0A:  // linefeed 
				case 0x09:  // tab 
				case 0x0D:  // carriage return 
					break;

				case 0x1B:  // escape 
				{
					DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
					if (!(dwStyle & WS_OVERLAPPEDWINDOW))
					{
						// Make window not full screen
						SetWindowLong(hwnd, GWL_STYLE,
							dwStyle | WS_OVERLAPPEDWINDOW);
						SetWindowPlacement(hwnd, &pEnharmonic->g_wpPrev);
						SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
							SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
							SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
						InvalidateRect(hwnd, NULL, FALSE);
					}
				}
					break;

				default:    // displayable character 

					TCHAR ch = (TCHAR)wParam;
					if ('w' == ch)
					{
						//////////////////////////////////////
						// Toggle between full screen
						DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
						if (dwStyle & WS_OVERLAPPEDWINDOW) {
							MONITORINFO mi = { sizeof(mi) };
							if (GetWindowPlacement(hwnd, &pEnharmonic->g_wpPrev) &&
								GetMonitorInfo(MonitorFromWindow(hwnd,
									MONITOR_DEFAULTTOPRIMARY), &mi)) {
								SetWindowLong(hwnd, GWL_STYLE,
									dwStyle & ~WS_OVERLAPPEDWINDOW);
								SetWindowPos(hwnd, HWND_TOP,
									mi.rcMonitor.left, mi.rcMonitor.top,
									mi.rcMonitor.right - mi.rcMonitor.left,
									mi.rcMonitor.bottom - mi.rcMonitor.top,
									SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
								InvalidateRect(hwnd, NULL, FALSE);
							}
						}
						else {
							SetWindowLong(hwnd, GWL_STYLE,
								dwStyle | WS_OVERLAPPEDWINDOW);
							SetWindowPlacement(hwnd, &pEnharmonic->g_wpPrev);
							SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
								SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
								SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
							InvalidateRect(hwnd, NULL, FALSE);
						}
					}
					else if ('l' == ch)
					{
						DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
						MONITORINFO mi = { sizeof(mi) };
						if (GetWindowPlacement(hwnd, &pEnharmonic->g_wpPrev) &&
							GetMonitorInfo(MonitorFromWindow(hwnd,
								MONITOR_DEFAULTTOPRIMARY), &mi)) {
							SetWindowLong(hwnd, GWL_STYLE,
								dwStyle & ~WS_OVERLAPPEDWINDOW);
							SetWindowPos(hwnd, HWND_TOP,
								mi.rcMonitor.left, mi.rcMonitor.top,
								(mi.rcMonitor.right - mi.rcMonitor.left)/2,
								mi.rcMonitor.bottom - mi.rcMonitor.top,
								SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
							InvalidateRect(hwnd, NULL, FALSE);
						}
					}
					else if ('r' == ch)
					{
						DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
						MONITORINFO mi = { sizeof(mi) };
						if (GetWindowPlacement(hwnd, &pEnharmonic->g_wpPrev) &&
							GetMonitorInfo(MonitorFromWindow(hwnd,
								MONITOR_DEFAULTTOPRIMARY), &mi)) {
							SetWindowLong(hwnd, GWL_STYLE,
								dwStyle & ~WS_OVERLAPPEDWINDOW);
							SetWindowPos(hwnd, HWND_TOP,
								(mi.rcMonitor.right - mi.rcMonitor.left) / 2, mi.rcMonitor.top,
								(mi.rcMonitor.right - mi.rcMonitor.left) / 2,
								mi.rcMonitor.bottom - mi.rcMonitor.top,
								SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
							InvalidateRect(hwnd, NULL, FALSE);
						}
					}
					else if ('f' == ch)
					{
						pEnharmonic->latticeView.temperament.setFactorRatios(! pEnharmonic->latticeView.temperament.getFactorRatios());
						InvalidateRect(hwnd, NULL, FALSE);
					}
					else if ('t' == ch)
					{
						ShowWindow(pEnharmonic->hTemperamentDlg, SW_SHOW);
					}
					else if ('e' == ch)
					{
						if (pEnharmonic->latticeView.mode == TouchMode::EDIT_ROTATE_SCALE)
						{
							pEnharmonic->latticeView.mode = TouchMode::PLAY_MUSIC;
						}
						else
						{
							pEnharmonic->latticeView.mode = TouchMode::EDIT_ROTATE_SCALE;
						}
					}
					break;
				}


			
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}


WNDPROC g_pfnPeriod_Old = NULL;


LRESULT CALLBACK MyEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TemperamentDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		//HMENU hmenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_TEMPERAMENT_MENU));
		//SetMenu(hDlg, hmenu);

		GetOpenFileName(NULL);

//		g_pfnPeriod_Old = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD2), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD3), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD5), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD7), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD11), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD13), GWLP_WNDPROC, (LONG_PTR)MyEditProc);
		return TRUE;
	}

	case WM_DESTROY:
	case WM_CLOSE:
	{
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD2), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD3), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD5), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD7), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD11), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);
//		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PERIOD13), GWLP_WNDPROC, (LONG_PTR)g_pfnPeriod_Old);

		EndDialog(hDlg, 0);
		return TRUE;
	}

	}

	return FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
