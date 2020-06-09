#pragma once

#include "resource.h"


//frequently used headers.
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include "LatticeView.h"

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


//Declare methods for initializing the class, creating and discarding resources, handling the message loop, rendering content, and the windows procedure.
class Enharmonic
{
public:
	Enharmonic();
	~Enharmonic();

	// Register the window class and call methods for instantiating drawing resources
	HRESULT Initialize();

	// Process and dispatch messages
	void RunMessageLoop();

//private:

	WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

	// The windows procedure.
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
		);

	LatticeView latticeView;
	HWND hTemperamentDlg;
};